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
    public class ListImages : IHttpFunction
    {
        private readonly ILogger _logger;
        private StorageClient _storage;

        public ListImages(ILogger<ListImages> logger) {
            _logger = logger;
            _storage = StorageClient.Create();
        }

        public async Task HandleAsync(HttpContext context)
        {
            HttpResponse response = context.Response;
            HttpRequest request = context.Request;

            if (request.Method != "GET")
            {
                response.StatusCode = (int) HttpStatusCode.MethodNotAllowed;
                return;
            }
            var storageObjects = _storage.ListObjects("moodcap");

            string output = "";
            foreach(var storageObject in storageObjects) {
                output += storageObject.Name;
            }

            response.StatusCode = 200;
            await response.WriteAsync(output);

            return;
        }
    }
}
