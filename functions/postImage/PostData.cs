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
    public class PostData : IHttpFunction
    {
        private readonly ILogger _logger;

        public PostData(ILogger<PostData> logger) {
            _logger = logger;
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
            if(!request.Form.ContainsKey("TestData")) {
                response.StatusCode = (int) HttpStatusCode.BadRequest;
                await response.WriteAsync("The request does not contain a TestData");
                return;
            }

            // obtain CameraName
            StringValues cameraNameValues = new StringValues();
            request.Form.TryGetValue("CameraName", out cameraNameValues);
            string cameraName = cameraNameValues.ToString();

            // obtain TestData
            StringValues testDataValues = new StringValues();
            request.Form.TryGetValue("TestData", out testDataValues);
            string testData = testDataValues.ToString();

            response.StatusCode = 200;
            await response.WriteAsync("Got TestData: " + testData + " and cameraName: " + cameraName);
            return;
        }

    }
}
