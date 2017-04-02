using System;
using Microsoft.WindowsAzure.MobileServices;
using Newtonsoft.Json;

namespace SaveBB
{
	public class AlertItem
	{
		string id;
		string name;
		bool done;

		[JsonProperty(PropertyName = "id")]
		public string Id
		{
			get { return id; }
			set { id = value;}
		}

        public string AlertValue { get; set; }

        public DateTimeOffset AlertTime { get; set; }

        public decimal? Humidity { get; set; }

        public decimal? Temp { get; set; }

        public decimal? HeartRate { get; set; }


        //[JsonProperty(PropertyName = "text")]
        //public string Name
        //{
        //	get { return name; }
        //	set { name = value;}
        //}

        //[JsonProperty(PropertyName = "complete")]
        //public bool Done
        //{
        //	get { return done; }
        //	set { done = value;}
        //}

        [Version]
        public string Version { get; set; }
	}
}

