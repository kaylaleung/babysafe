using System;
using System.Collections.Generic;
using System.Data;
using System.Data.Entity;
using System.Linq;
using System.Net;
using System.Web;
using System.Web.Mvc;
using SaveBBService.DataObjects;
using SaveBBService.Models;
using Twilio;

namespace SaveBBService.Controllers
{
    public class AlertItemsMVCController : Controller
    {
        private SaveBBContext db = new SaveBBContext();

        // GET: AlertItemsMVC
        public ActionResult Index()
        {
            return View(db.AlertItems.OrderByDescending(i => i.AlertTime).Take(10).ToList());
        }

        // GET: AlertItemsMVC/Details/5
        public ActionResult Details(string id)
        {
            if (id == null)
            {
                return new HttpStatusCodeResult(HttpStatusCode.BadRequest);
            }
            AlertItem alertItem = db.AlertItems.Find(id);
            if (alertItem == null)
            {
                return HttpNotFound();
            }
            return View(alertItem);
        }

        // GET: AlertItemsMVC/Create
        public ActionResult Create()
        {
            return View();
        }

        // POST: AlertItemsMVC/Create
        // To protect from overposting attacks, please enable the specific properties you want to bind to, for 
        // more details see http://go.microsoft.com/fwlink/?LinkId=317598.
        [HttpPost]
        [ValidateAntiForgeryToken]
        public ActionResult Create([Bind(Include = "AlertValue,PhoneNum,Humidity,Temp,HeartRate,CreatedAt,UpdatedAt")] AlertItem alertItem)
        {
            long phonenumResult;
            if (string.IsNullOrEmpty(alertItem.PhoneNum) || alertItem.PhoneNum.Length != 11 || !long.TryParse(alertItem.PhoneNum, out phonenumResult))
            {
                ModelState.AddModelError("PhoneNum", "PhoneNum has to be a number of 11 digits");
            }

            if (!string.IsNullOrEmpty(alertItem.PhoneNum))
            {
                // ONLY A FEW PRE-REGISTERED Number will allow to use Text in the development phase!
                List<string> registeredPhoneNums = new List<string>();
                registeredPhoneNums.Add("17329936799");
                registeredPhoneNums.Add("17324858626");

                if (registeredPhoneNums.Contains(alertItem.PhoneNum))
                {
                    string AccountSid = Environment.GetEnvironmentVariable("TWILIO_AccountSid");
                    string AuthToken = Environment.GetEnvironmentVariable("TWILIO_AuthToken");
                    string TwilioAssignedPhonenum = Environment.GetEnvironmentVariable("TWILIO_PhoneNum");
                    //string TwilioTargetPhoneNumOverride = Environment.GetEnvironmentVariable("TWILIO_TargetPhoneNumOverride");
                    string EnableTexting = Environment.GetEnvironmentVariable("TWILIO_EnableTexting");
                    if (!string.IsNullOrEmpty(EnableTexting) && EnableTexting == "1")
                    {
                        var twilio = new TwilioRestClient(AccountSid, AuthToken);
                        TimeZoneInfo easternTimeZone = TimeZoneInfo.FindSystemTimeZoneById("Eastern Standard Time");
                        DateTime easternDateTime = TimeZoneInfo.ConvertTimeFromUtc(DateTime.UtcNow, easternTimeZone);
                        var message = twilio.SendMessage(TwilioAssignedPhonenum, "+1" + alertItem.PhoneNum, "Alert from Stanford OHS Girls Can Code Baby Monitor: Check on your baby! " + easternDateTime.ToString("hh:mm:ss tt"));

                        if (message.RestException != null)
                        {
                            ModelState.AddModelError("", "Twilio SMS Failed! " + message.RestException.Message);
                        }
                    }
                }
            }

            alertItem.Id = Guid.NewGuid().ToString();
            alertItem.AlertTime = DateTimeOffset.Now;
            alertItem.AlertType = "sms";

            if (ModelState.IsValid)
            {
                db.AlertItems.Add(alertItem);
                db.SaveChanges();


                return RedirectToAction("Index");
            }

            return View(alertItem);
        }

        //// GET: AlertItemsMVC/Edit/5
        //public ActionResult Edit(string id)
        //{
        //    if (id == null)
        //    {
        //        return new HttpStatusCodeResult(HttpStatusCode.BadRequest);
        //    }
        //    AlertItem alertItem = db.AlertItems.Find(id);
        //    if (alertItem == null)
        //    {
        //        return HttpNotFound();
        //    }
        //    return View(alertItem);
        //}

        //// POST: AlertItemsMVC/Edit/5
        //// To protect from overposting attacks, please enable the specific properties you want to bind to, for 
        //// more details see http://go.microsoft.com/fwlink/?LinkId=317598.
        //[HttpPost]
        //[ValidateAntiForgeryToken]
        //public ActionResult Edit([Bind(Include = "Id,AlertTime,AlertValue,AlertType,PhoneNum,Humidity,Temp,HeartRate,Version,CreatedAt,UpdatedAt,Deleted")] AlertItem alertItem)
        //{
        //    if (ModelState.IsValid)
        //    {
        //        db.Entry(alertItem).State = EntityState.Modified;
        //        db.SaveChanges();
        //        return RedirectToAction("Index");
        //    }
        //    return View(alertItem);
        //}

        //// GET: AlertItemsMVC/Delete/5
        //public ActionResult Delete(string id)
        //{
        //    if (id == null)
        //    {
        //        return new HttpStatusCodeResult(HttpStatusCode.BadRequest);
        //    }
        //    AlertItem alertItem = db.AlertItems.Find(id);
        //    if (alertItem == null)
        //    {
        //        return HttpNotFound();
        //    }
        //    return View(alertItem);
        //}

        //// POST: AlertItemsMVC/Delete/5
        //[HttpPost, ActionName("Delete")]
        //[ValidateAntiForgeryToken]
        //public ActionResult DeleteConfirmed(string id)
        //{
        //    AlertItem alertItem = db.AlertItems.Find(id);
        //    db.AlertItems.Remove(alertItem);
        //    db.SaveChanges();
        //    return RedirectToAction("Index");
        //}

        public ActionResult API()
        {
            var port = Request.Url.Port;
            ViewData["host"] = Request.Url.Host + (port == 0 ? "" : ":" + port);
            return View();
        }

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                db.Dispose();
            }
            base.Dispose(disposing);
        }
    }
}
