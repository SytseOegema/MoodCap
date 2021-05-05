using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.HttpsPolicy;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Hosting;
using Microsoft.Extensions.Logging;
using Microsoft.OpenApi.Models;


namespace moodCap
{
    public class Startup
    {
        public Startup(IConfiguration configuration)
        {
            Configuration = configuration;
        }

        public IConfiguration Configuration { get; }

        // This method gets called by the runtime. Use this method to add services to the container.
        public void ConfigureServices(IServiceCollection services)
        {
            services.AddCors(options =>
            {
                options.AddPolicy("CorsPolicy", builder =>
                    builder.AllowAnyOrigin()
                    .AllowAnyMethod()
                    .AllowAnyHeader());
            });
            services.AddControllers()
                .ConfigureApiBehaviorOptions(options =>
                {
                    options.InvalidModelStateResponseFactory = context =>
                    {
                        // Get an instance of ILogger (see below) and log accordingly.

                        Console.WriteLine("check dit dan gek");

                        return new BadRequestObjectResult(context.ModelState);
                    };
                });
                // .ConfigureApiBehaviorOptions(options =>
                // {
                //     options.SuppressConsumesConstraintForFormFileParameters = true;
                //     options.SuppressInferBindingSourcesForParameters = true;
                //     options.SuppressModelStateInvalidFilter = true;
                //     options.SuppressMapClientErrors = true;
                // });

            services.AddSwaggerGen(c =>
            {
                c.SwaggerDoc("v1", new OpenApiInfo { Title = "moodCap", Version = "v1" });
            });

            // services.PostConfigure<ApiBehaviorOptions>(options =>
            // {
            //     var builtInFactory = options.InvalidModelStateResponseFactory;
            //
            //     options.InvalidModelStateResponseFactory = context =>
            //     {
            //         // Get an instance of ILogger (see below) and log accordingly.
            //         var loggerFactory = context.HttpContext.RequestServices
            //             .GetRequiredService<ILoggerFactory>();
            //         var logger = loggerFactory.CreateLogger("YourCategory");
            //         logger.LogInformation("bad Request");
            //         Console.WriteLine("hoihoihoi");
            //         return builtInFactory(context);
            //     };
            // });
        }

        // This method gets called by the runtime. Use this method to configure the HTTP request pipeline.
        public void Configure(IApplicationBuilder app, IWebHostEnvironment env)
        {
            if (env.IsDevelopment())
            {
                // app.UseDeveloperExceptionPage();
                app.UseSwagger();
                app.UseSwaggerUI(c =>
                {
                    c.SwaggerEndpoint("/swagger/v1/swagger.json", "moodCap v1");
                    c.RoutePrefix = string.Empty;
                });
            }

            // app.UseHttpsRedirection();
            app.UseMiddleware<LogMiddleware>();

            app.UseRouting();

            // app.UseAuthorization();

            app.UseEndpoints(endpoints =>
            {
                endpoints.MapControllers();
            });
        }
    }
}
