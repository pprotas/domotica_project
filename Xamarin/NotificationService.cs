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
using System.Net;
using System.Net.Sockets;

namespace HomeSafe9001
{
    [Service]
    public class NotificationService : Service
    {
        int notificationId = 9001;
        HomeActivity _homeActivity;

        public Socket socket = null;                       // Socket   
        public Timer timerSockets;   // = new System.Timers.Timer() { Interval = 1000, Enabled = false};

        Timer notificationTimer = new Timer() { Interval = 1000 };
        NotificationCompat.Builder builder;
        NotificationManager notificationManager;

        
        public override IBinder OnBind(Intent intent)
        {
            throw new NotImplementedException();
        }

        [return: GeneratedEnum]
        public override StartCommandResult OnStartCommand(Intent intent, [GeneratedEnum] StartCommandFlags flags, int startId)
        {
            _homeActivity = new HomeActivity();

            Intent notificationIntent = new Intent(this, typeof(HomeActivity));

            TaskStackBuilder stackBuilder = TaskStackBuilder.Create(this);
            stackBuilder.AddParentStack(Java.Lang.Class.FromType(typeof(HomeActivity)));
            stackBuilder.AddNextIntent(notificationIntent);

            PendingIntent pendingNotificationIntent = stackBuilder.GetPendingIntent(0, (int)PendingIntentFlags.UpdateCurrent);

            builder = new NotificationCompat.Builder(this)
                .SetAutoCancel(true)
                .SetContentIntent(pendingNotificationIntent)
                .SetContentTitle("Movement detected");

            notificationManager =
                (NotificationManager)GetSystemService(NotificationService);

            //notificationTimer.Start();
            //notificationTimer.Elapsed += ElapsedNotificationTimer;

            //timerSockets = new System.Timers.Timer() { Interval = _homeActivity.timerDelay, Enabled = false }; // Interval >= 750
            //timerSockets.Elapsed += (obj, args) =>
            //{
            //    //RunOnUiThread(() =>
            //    //{
            //    if (socket != null) // only if socket exists
            //    {
            //        // Send a command to the Arduino server on every tick (loop though list)
            //        _homeActivity.UpdateGUI(_homeActivity.executeCommand(_homeActivity.commandList[_homeActivity.listIndex].Item1),
            //            _homeActivity.commandList[_homeActivity.listIndex].Item2);  //e.g. UpdateGUI(executeCommand("s"), textViewChangePinStateValue);
            //        if (++_homeActivity.listIndex >= _homeActivity.commandList.Count) _homeActivity.listIndex = 0;
            //    }
            //    else timerSockets.Enabled = false;  // If socket broken -> disable timer
            //    //});
            //};

            return StartCommandResult.NotSticky;
        }

        //private void ElapsedNotificationTimer(object sender, EventArgs e)
        //{
        //    if (executeCommand("b") == "TRU")
        //    {
        //        notificationManager.Notify(notificationId, builder.Build());
        //    }
        //}

        //public void ElapsedTimer(object sender, EventArgs e)
        //{
        //    {
        //        //RunOnUiThread(() =>
        //        //{
        //        if (socket != null) // only if socket exists
        //        {
        //            // Send a command to the Arduino server on every tick (loop though list)
        //            _homeActivity.UpdateGUI(executeCommand(_homeActivity.commandList[_homeActivity.listIndex].Item1), _homeActivity.commandList[_homeActivity.listIndex].Item2);  //e.g. UpdateGUI(executeCommand("s"), textViewChangePinStateValue);
        //            if (++_homeActivity.listIndex >= _homeActivity.commandList.Count) _homeActivity.listIndex = 0;
        //        }
        //        else timerSockets.Enabled = false;  // If socket broken -> disable timer
        //        //});
        //    };
        //} 

        //public string executeCommand(string cmd)
        //{
        //    byte[] buffer = new byte[4]; // response is always 4 bytes
        //    int bytesRead = 0;
        //    string result = "---";

        //    if (socket != null)
        //    {
        //        //Send command to server
        //        socket.Send(Encoding.ASCII.GetBytes(cmd));

        //        try //Get response from server
        //        {
        //            //Store received bytes (always 4 bytes, ends with \n)
        //            bytesRead = socket.Receive(buffer);  // If no data is available for reading, the Receive method will block until data is available,
        //            //Read available bytes.              // socket.Available gets the amount of data that has been received from the network and is available to be read
        //            while (socket.Available > 0) bytesRead = socket.Receive(buffer);
        //            if (bytesRead == 4)
        //                result = Encoding.ASCII.GetString(buffer, 0, bytesRead - 1); // skip \n
        //            else result = "err";
        //        }
        //        catch (Exception exception)
        //        {
        //            result = exception.ToString();
        //            if (socket != null)
        //            {
        //                socket.Close();
        //                socket = null;
        //            }
        //            _homeActivity.UpdateConnectionState(3, result);
        //        }
        //    }
        //    return result;
        //}

        ////Connect to socket ip/prt(simple sockets)
        //public void ConnectSocket(string ip, string prt)
        //{
        //    _homeActivity = new HomeActivity();

        //    _homeActivity.RunOnUiThread(() =>
        //    {
        //        if (socket == null)                                       // create new socket
        //        {
        //            _homeActivity.UpdateConnectionState(1, "Connecting...");
        //            try  // to connect to the server (Arduino).
        //            {
        //                socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        //                socket.Connect(new IPEndPoint(IPAddress.Parse(ip), Convert.ToInt32(prt)));
        //                if (socket.Connected)
        //                {
        //                    _homeActivity.UpdateConnectionState(2, "Connected");
        //                    timerSockets.Enabled = true;                //Activate timer for communication with Arduino
        //                }
        //            }
        //            catch (Exception exception)
        //            {
        //                timerSockets.Enabled = false;
        //                if (socket != null)
        //                {
        //                    socket.Close();
        //                    socket = null;
        //                }
        //                _homeActivity.UpdateConnectionState(4, exception.Message);
        //            }
        //        }
        //        else // disconnect socket
        //        {
        //            socket.Close(); socket = null;
        //            timerSockets.Enabled = false;
        //            _homeActivity.UpdateConnectionState(4, "Disconnected");
        //        }
        //    });
        //}
    }
}