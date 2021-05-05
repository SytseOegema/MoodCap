using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Logging;
using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.Primitives;
using moodCap;

namespace moodCap.Controllers
{
    [ApiController]
    [Route("[controller]")]
    public class CapImageController : ControllerBase
    {
        private readonly ILogger<CapImageController> _logger;

        public CapImageController(ILogger<CapImageController> logger)
        {
            _logger = logger;
        }

        [HttpGet]
        public IEnumerable<CapImage> GetImage()
        {
            var rng = new Random();
            return Enumerable.Range(1, 5).Select(index => new CapImage
            {
                CameraName = "camera"
            })
            .ToArray();
        }

        [HttpPost]
        // public async Task<ActionResult<CapImage>> PostImage([FromForm] FormCollection collection)
        public async Task<ActionResult<CapImage>> PostImage([FromForm] IFormCollection collection)
        {
            // Check that the right parameters are send in the request.
            if(!collection.ContainsKey("CameraName")) {
                return BadRequest("The request does not contain a parameter 'CameraName'");
            }
            if(collection.Files.GetFile("Content") == null) {
                return BadRequest("The request does not contain a file named 'Content'");
            }

            IFormFile cameraImage = collection.Files.GetFile("Content");
            // Check the file is an image: JPG, JPEG or PNG.
            if(!isImage(cameraImage)) {
                return BadRequest("The file 'Content' that was send is not a JPG, JPEG or PNG");
            }

            StringValues cameraNameValues = new StringValues();
            collection.TryGetValue("CameraName", out cameraNameValues);
            string cameraName = cameraNameValues.ToString();

            CapImage image = new CapImage();
            image.CameraName = cameraName;
            image.Content = cameraImage;

            bool success = await StoreImage(image);

            return CreatedAtAction(nameof(GetImage),
                new { id = image.CameraName }
                , image);
        }

        /**
         * Checks if the provided file is an image file; JPG, JPEG or PNG
         * @param file the file that is checked.
         * @returns bool that indicates if the file is an image.
         */
        private bool isImage(IFormFile file) {
            var extension = file.FileName
                .Split('.')[file.FileName.Split('.').Length - 1];

            extension = extension.ToLower();

            switch(extension)
            {
                case "jpg":
                    return true;
                case "jpeg":
                    return true;
                case "png":
                    return true;
                default:
                    return false;
            }
        }

        /**
         * Stores an image in the image folder. The image is stored with the
         * name: [CameraName]-[TimeOfStorage].[OriginalExtension].
         * Where the CameraName is obtained from the image pareter.
         * @param image the image that is stored
         * @returns bool that indicates the success of the file storage.
         */
        private async Task<bool> StoreImage(CapImage image) {
            bool isSaveSuccess = false;
            string fileName;
            try
            {
                var extension = "." + image.Content.FileName
                    .Split('.')[image.Content.FileName.Split('.').Length - 1];
                // Create a new Name for the file to keep track of them.
                fileName = image.CameraName
                    + '-'
                    + DateTime.Now.Ticks
                    + extension;

                var pathBuilt = Path.Combine(Directory.GetCurrentDirectory(),
                    "images");
                // Check if image folder exists. Otherwise create.
                if (!Directory.Exists(pathBuilt))
                {
                    Directory.CreateDirectory(pathBuilt);
                }

                var path = Path.Combine(Directory.GetCurrentDirectory(), "images",
                fileName);

                using (var stream = new FileStream(path, FileMode.Create))
                {
                    await image.Content.CopyToAsync(stream);
                }
                isSaveSuccess = true;
            }
            catch (Exception e)
            {
               _logger.LogError(DateTime.Now
                  + ": An exception occured in StoreImage()");
                Exception error = e;
                while(error != null) {
                   _logger.LogError(error.ToString());
                   error = error.InnerException;
                }
            }
            return isSaveSuccess;
        }
    }
}
