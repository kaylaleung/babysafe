using Microsoft.Owin;
using Owin;

[assembly: OwinStartup(typeof(SaveBBService.Startup))]

namespace SaveBBService
{
    public partial class Startup
    {
        public void Configuration(IAppBuilder app)
        {
            ConfigureMobileApp(app);
        }
    }
}