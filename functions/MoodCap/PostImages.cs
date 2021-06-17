using Google.Cloud.Functions.Framework;
using Google.Cloud.Storage.V1;
using Microsoft.AspNetCore.Http;
using Microsoft.Extensions.Logging;
using Microsoft.Extensions.Primitives;
using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Threading.Tasks;

namespace MoodCap
{
    public class PostImages : IHttpFunction
    {
        private readonly ILogger _logger;
        private StorageClient _storage;

        public PostImages(ILogger<PostImages> logger) {
            _logger = logger;
            _storage = StorageClient.Create();
        }

        public async Task HandleAsync(HttpContext context)
        {
            HttpResponse response = context.Response;
            HttpRequest request = context.Request;

            if (request.Method != "POST")
            {
                response.StatusCode = (int) HttpStatusCode.MethodNotAllowed;
                return;
            }
            if(!request.Form.ContainsKey("CameraName")) {
                response.StatusCode = (int) HttpStatusCode.BadRequest;
                await response.WriteAsync("The request does not contain a CameraName");
                return;
            }
            if(request.Form.Files.GetFile("Content0") == null) {
                response.StatusCode = (int) HttpStatusCode.BadRequest;
                await response.WriteAsync("The request does not contain an file named Content");
                return;
            }

            // obtain CameraName
            StringValues cameraNameValues = new StringValues();
            request.Form.TryGetValue("CameraName", out cameraNameValues);
            string cameraName = cameraNameValues.ToString();

            // obtain files Content[x] with a max of 10 files.
            for (var idx = 0; idx < 10; ++idx) {
                // break if there are no more files.
                if(request.Form.Files.GetFile("Content" + idx) == null) {
                    break;
                }

                IFormFile cameraImage = request.Form.Files.GetFile("Content" + idx);
                // Check the file is an image: JPG, JPEG or PNG.
                if(!IsImage(cameraImage)) {
                    response.StatusCode = (int) HttpStatusCode.BadRequest;
                    await response.WriteAsync("The uploaded file is not a JPG, JPEG or PNG");
                    return;
                }

                if(! await StoreImage(cameraImage, cameraName)) {
                    // internal server error
                    response.StatusCode = (int) HttpStatusCode.InternalServerError;
                    await response.WriteAsync("Something went wrong while storing the file");
                    return;
                }
            }

            response.StatusCode = 200;
            return;
        }

        /**
         * Checks if the provided file is an image file; JPG, JPEG or PNG
         * @param file the file that is checked.
         * @returns bool that indicates if the file is an image.
         */
        private bool IsImage(IFormFile file) {
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
         * @param image the image that is stored
         * @param cameraName the name of the camera that the image was send
         * from.
         * @returns bool that indicates the success of the file storage.
         */
        private async Task<bool> StoreImage(IFormFile image, string cameraName) {
            bool isSaveSuccess = false;
            string fileName;
            try
            {
                var extension = "." + image.FileName
                    .Split('.')[image.FileName.Split('.').Length - 1];
                // Create a new Name for the file to keep track of them.
                fileName = cameraName
                    + '-'
                    + DateTime.Now.Ticks
                    + extension;

                using (var stream = image.OpenReadStream())
                {
                    // writes the file to the cloud storage bucket moodcap
                    await _storage.UploadObjectAsync("moodcap", fileName, null, stream);
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
