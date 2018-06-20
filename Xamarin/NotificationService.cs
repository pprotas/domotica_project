using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using Android.App;
using Android.Content;
using Android.OS;
using Android.Runtime;
using Android.Views;
using Android.Widget;

using Android.Support.V4.App;
using TaskStackBuilder = Android.Support.V4.App.TaskStackBuilder;
using System.Timers;

namespace HomeSafe9001
{
    [Service]
    class NotificationService : Service
    {
        int notificationId = 9001;
        Timer serviceTimer = new Timer { Interval = 1000 };
        HomeActivity homeActivity;
        NotificationCompat.Builder builder;
        NotificationManager notificationManager;

        public override IBinder OnBind(Intent intent)
        {
            throw new NotImplementedException();
        }

        [return: GeneratedEnum]
        public override StartCommandResult OnStartCommand(Intent intent, [GeneratedEnum] StartCommandFlags flags, int startId)
        {
            Intent notificationIntent = new Intent(this, typeof(HomeActivity));

            TaskStackBuilder stackBuilder = TaskStackBuilder.Create(this);
            stackBuilder.AddParentStack(Java.Lang.Class.FromType(typeof(MainActivity)));
            stackBuilder.AddNextIntent(notificationIntent);

            PendingIntent pendingNotificationIntent = stackBuilder.GetPendingIntent(0, (int)PendingIntentFlags.UpdateCurrent);

            builder = new NotificationCompat.Builder(this)
                .SetAutoCancel(true)
                .SetContentIntent(pendingNotificationIntent)
                .SetContentTitle("Movement detected");

            notificationManager =
                (NotificationManager)GetSystemService(NotificationService);

            serviceTimer.Start();
            serviceTimer.Elapsed += ElapsedServiceTimer;


            return StartCommandResult.NotSticky;
        }

        public void ElapsedServiceTimer(object sender, EventArgs e)
        {
            if(executeCommand("b") == "TRU")
            {
                notificationManager.Notify(notificationId, builder.Build());
            }
        }

        //Send command to server and wait for response (blocking)
        //Method should only be called when socket existst
        public string executeCommand(string cmd)
        {
            byte[] buffer = new byte[4]; // response is always 4 bytes
            int bytesRead = 0;
            string result = "---";

            if (homeActivity.socket != null)
            {
                //Send command to server
                homeActivity.socket.Send(Encoding.ASCII.GetBytes(cmd));

                try //Get response from server
                {
                    //Store received bytes (always 4 bytes, ends with \n)
                    bytesRead = homeActivity.socket.Receive(buffer);  // If no data is available for reading, the Receive method will block until data is available,
                    //Read available bytes.              // socket.Available gets the amount of data that has been received from the network and is available to be read
                    while (homeActivity.socket.Available > 0) bytesRead = homeActivity.socket.Receive(buffer);
                    if (bytesRead == 4)
                        result = Encoding.ASCII.GetString(buffer, 0, bytesRead - 1); // skip \n
                    else result = "err";
                }
                catch (Exception exception)
                {
                    result = exception.ToString();
                    if (homeActivity.socket != null)
                    {
                        homeActivity.socket.Close();
                        homeActivity.socket = null;
                    }
                    try
                    {
                        homeActivity.UpdateConnectionState(3, result);
                    }
                    catch(Exception e)
                    {

                    }
                }
            }
            return result;
        }
    }
}