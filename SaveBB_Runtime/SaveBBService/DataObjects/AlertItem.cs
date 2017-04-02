using Microsoft.Azure.Mobile.Server;
using System;
using System.ComponentModel;

namespace SaveBBService.DataObjects
{
    public class AlertItem : EntityData
    {
        //public string Text { get; set; }

        //public bool Complete { get; set; }

        public DateTimeOffset AlertTime { get; set; }
        
        public string AlertValue { get; set; }
        public string AlertType { get; set; }

        [DisplayName("Text/SMS #")]
        public string PhoneNum { get; set; }

        public decimal? Humidity { get; set; }

        [DisplayName("Temp.")]
        public decimal? Temp { get; set; }

        [DisplayName("Heart")]
        public decimal? HeartRate { get; set; }



    }
}