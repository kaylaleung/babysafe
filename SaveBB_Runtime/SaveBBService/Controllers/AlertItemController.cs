using System.Linq;
using System.Threading.Tasks;
using System.Web.Http;
using System.Web.Http.Controllers;
using System.Web.Http.OData;
using Microsoft.Azure.Mobile.Server;
using SaveBBService.DataObjects;
using SaveBBService.Models;
using System;
using Twilio;

namespace SaveBBService.Controllers
{
    public class AlertItemController : TableController<AlertItem>
    {
        protected override void Initialize(HttpControllerContext controllerContext)
        {
            base.Initialize(controllerContext);
            SaveBBContext context = new SaveBBContext();
            DomainManager = new EntityDomainManager<AlertItem>(context, Request);
        }

        // GET tables/AlertItem
        public IQueryable<AlertItem> GetAllAlertItems()
        {
            return Query();
        }

        // GET tables/AlertItem/48D68C86-6EA6-4C25-AA33-223FC9A27959
        public SingleResult<AlertItem> GetAlertItem(string id)
        {
            return Lookup(id);

        }

        // PATCH tables/AlertItem/48D68C86-6EA6-4C25-AA33-223FC9A27959
        public Task<AlertItem> PatchAlertItem(string id, Delta<AlertItem> patch)
        {
            return UpdateAsync(id, patch);
        }

        // POST tables/AlertItem
        public async Task<IHttpActionResult> PostAlertItem(AlertItem item)
        {

            item.Id = Guid.NewGuid().ToString();
            item.AlertTime = DateTimeOffset.Now;

            AlertItem current = await InsertAsync(item);
            
           if(!string.IsNullOrEmpty(item.PhoneNum) && !string.IsNullOrEmpty(item.AlertType))
            {
                string AccountSid = Environment.GetEnvironmentVariable("TWILIO_AccountSid");
                string AuthToken = Environment.GetEnvironmentVariable("TWILIO_AuthToken");
                string TwilioAssignedPhonenum = Environment.GetEnvironmentVariable("TWILIO_PhoneNum");
                string TwilioTargetPhoneNumOverride = Environment.GetEnvironmentVariable("TWILIO_TargetPhoneNumOverride");
                var twilio = new TwilioRestClient(AccountSid, AuthToken);
                TimeZoneInfo easternTimeZone = TimeZoneInfo.FindSystemTimeZoneById("Eastern Standard Time");
                DateTime easternDateTime = TimeZoneInfo.ConvertTimeFromUtc(DateTime.UtcNow, easternTimeZone);
                var message = twilio.SendMessage(TwilioAssignedPhonenum, TwilioTargetPhoneNumOverride, "Alert from SaveBB: Check on your baby! " + easternDateTime.ToString("hh:mm:ss tt"));

                if (message.RestException != null)
                {
                    return NotFound();
                }
            }

            return CreatedAtRoute("Tables", new { id = current.Id }, current);
        }

        // DELETE tables/AlertItem/48D68C86-6EA6-4C25-AA33-223FC9A27959
        public Task DeleteAlertItem(string id)
        {
            return DeleteAsync(id);
        }
    }
}