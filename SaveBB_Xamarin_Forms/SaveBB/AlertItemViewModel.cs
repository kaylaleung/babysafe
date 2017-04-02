using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;

namespace SaveBB
{
    class AlertItemViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;
        AlertItem model;
        public AlertItemViewModel() { model = new AlertItem(); }
        public string AlertValue
        {
            set
            {
                if (!value.Equals(model.AlertValue, StringComparison.Ordinal))
                {
                    model.AlertValue = value;
                    OnPropertyChanged("AlertValue");
                }
            }
            get
            {
                return model.AlertValue;
            }
        }
        void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            var handler = PropertyChanged;
            if (handler != null)
            {
                handler(this, new PropertyChangedEventArgs(propertyName));
            }
        }
    }
}
