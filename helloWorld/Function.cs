using Google.Cloud.Functions.Framework;
using Microsoft.AspNetCore.Http;
using System.Threading.Tasks;

namespace HelloWorld
{
    public class Function : IHttpFunction
    {
        public async Task HandleAsync(HttpContext context)
        {
            await context.Response.WriteAsync("Hello World!");
        }
    }
}
