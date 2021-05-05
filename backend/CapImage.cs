using System;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Http;

namespace moodCap
{
    public class CapImage
    {
        public IFormFile Content { get; set; }

        public string CameraName { get; set; }
    }
}
