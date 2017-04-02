using System;
using System.Threading.Tasks;
using Xamarin.Forms;

namespace SaveBB
{
    public partial class AlertList : ContentPage
    {
        AlertItemManager manager;

        
        AlertItemViewModel vm;
        int count = 0;
        bool _keepPolling = true;


        public AlertList()
        {
            InitializeComponent();

            
            vm = new AlertItemViewModel { AlertValue = "" };
            BindingContext = vm;
            refreshButton.Clicked += async (s, e) => {
                await RefreshItem(true);
            };
            babyIcon.Source = ImageSource.FromFile("babyicon.png");
            babyImage.Source = ImageSource.FromFile("babyup.jpg");
            water.Source = ImageSource.FromFile("water.jpg");
            sun.Source = ImageSource.FromFile("sun.jpg");
            heart.Source = ImageSource.FromFile("heart.jpg");
            temp.Source = ImageSource.FromFile("temp.jpg");

            manager = AlertItemManager.DefaultManager;

            // OnPlatform<T> doesn't currently support the "Windows" target platform, so we have this check here.
            if (manager.IsOfflineEnabled &&
                (Device.OS == TargetPlatform.Windows || Device.OS == TargetPlatform.WinPhone))
            {
                var syncButton = new Button
                {
                    Text = "Sync items",
                    HeightRequest = 30
                };
                syncButton.Clicked += OnSyncItems;

                //buttonsPanel.Children.Add(syncButton);
            }
        }

        protected override async void OnAppearing()
        {
            base.OnAppearing();

            ContinuousWebRequest();

            // Set syncItems to true in order to synchronize the data on startup when running in offline mode
            //await RefreshItems(true, syncItems: false);
        }

        private async void ContinuousWebRequest()
        {
            while (_keepPolling)
            {
                await RefreshItem(true);
                // Update the UI (because of async/await magic, this is still in the UI thread!)
                if (_keepPolling)
                {
                    await Task.Delay(TimeSpan.FromSeconds(20));
                }
            }
        }

        private async Task RefreshItem(bool showActivityIndicator)
        {
            //using (var scope = new ActivityIndicatorScope(syncIndicator, showActivityIndicator))
            {
                count++;
                AlertItem item = await manager.GetLatestAlertItemAsync();
                if (item != null)
                {
                    vm.AlertValue = "#" +  count + " " + DateTime.Now.ToLocalTime() + " : " + item.AlertValue;
                    decimal av;
                    if (decimal.TryParse(item.AlertValue, out av))
                    {
                        if (av < 0)
                            babyImage.Source = ImageSource.FromFile("babydown.jpg");
                        else
                            babyImage.Source = ImageSource.FromFile("babyup.jpg");

                        labelHumidity.Text = "Humidity: " + item.Humidity + "%";
                        lblHRate.Text = "Rate: " + item.HeartRate + " BPM";
                        lblRoomTemp.Text = "Room Temp: " + item.Temp + "F";
                        lblBodyTemp.Text = "Body Temp: 96F";
                    }
                }

            }
        }



        // Data methods
        async Task AddItem(AlertItem item)
        {
            await manager.SaveTaskAsync(item);
            //alertList.ItemsSource = await manager.GetAlertItemsAsync();
        }

        async Task CompleteItem(AlertItem item)
        {
            //item.Done = true;
            await manager.SaveTaskAsync(item);
            //alertList.ItemsSource = await manager.GetAlertItemsAsync();
        }

        public async void OnAdd(object sender, EventArgs e)
        {
            //var alert = new AlertItem { AlertValue = newItemName.Text };
            //await AddItem(alert);

            //newItemName.Text = string.Empty;
            //newItemName.Unfocus();
        }

        // Event handlers
        public async void OnSelected(object sender, SelectedItemChangedEventArgs e)
        {
            var alert = e.SelectedItem as AlertItem;
            if (Device.OS != TargetPlatform.iOS && alert != null)
            {
                // Not iOS - the swipe-to-delete is discoverable there
                if (Device.OS == TargetPlatform.Android)
                {
                    await DisplayAlert(alert.AlertValue, "Press-and-hold to complete task " + alert.AlertValue, "Got it!");
                }
                else
                {
                    // Windows, not all platforms support the Context Actions yet
                    if (await DisplayAlert("Mark completed?", "Do you wish to complete " + alert.AlertValue + "?", "Complete", "Cancel"))
                    {
                        await CompleteItem(alert);
                    }
                }
            }

            // prevents background getting highlighted
            //alertList.SelectedItem = null;
        }

        // http://developer.xamarin.com/guides/cross-platform/xamarin-forms/working-with/listview/#context
        public async void OnComplete(object sender, EventArgs e)
        {
            var mi = ((MenuItem)sender);
            var alert = mi.CommandParameter as AlertItem;
            await CompleteItem(alert);
        }

        // http://developer.xamarin.com/guides/cross-platform/xamarin-forms/working-with/listview/#pulltorefresh
        public async void OnRefresh(object sender, EventArgs e)
        {
            var list = (ListView)sender;
            Exception error = null;
            try
            {
                await RefreshItems(false, true);
            }
            catch (Exception ex)
            {
                error = ex;
            }
            finally
            {
                list.EndRefresh();
            }

            if (error != null)
            {
                await DisplayAlert("Refresh Error", "Couldn't refresh data (" + error.Message + ")", "OK");
            }
        }

        public async void OnSyncItems(object sender, EventArgs e)
        {
            await RefreshItems(true, true);
        }

        private async Task RefreshItems(bool showActivityIndicator, bool syncItems)
        {
            using (var scope = new ActivityIndicatorScope(syncIndicator, showActivityIndicator))
            {
                //alertList.ItemsSource = await manager.GetAlertItemsAsync(syncItems);
            }
        }

        private class ActivityIndicatorScope : IDisposable
        {
            private bool showIndicator;
            private ActivityIndicator indicator;
            private Task indicatorDelay;

            public ActivityIndicatorScope(ActivityIndicator indicator, bool showIndicator)
            {
                this.indicator = indicator;
                this.showIndicator = showIndicator;

                if (showIndicator)
                {
                    indicatorDelay = Task.Delay(2000);
                    SetIndicatorActivity(true);
                }
                else
                {
                    indicatorDelay = Task.FromResult(0);
                }
            }

            private void SetIndicatorActivity(bool isActive)
            {
                this.indicator.IsVisible = isActive;
                this.indicator.IsRunning = isActive;
            }

            public void Dispose()
            {
                if (showIndicator)
                {
                    indicatorDelay.ContinueWith(t => SetIndicatorActivity(false), TaskScheduler.FromCurrentSynchronizationContext());
                }
            }
        }
    }
}

