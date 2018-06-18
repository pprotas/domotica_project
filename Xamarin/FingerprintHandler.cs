using System;
using Android;
using Android.Content;
using Android.Hardware.Fingerprints;
using Android.OS;
using Android.Support.V4.App;
using Android.Widget;

namespace HomeSafe9001
{
    internal class FingerprintHandler : FingerprintManager.AuthenticationCallback
    {
        private Context mainActivity;
        public FingerprintHandler(Context mainActivity)
        {
            this.mainActivity = mainActivity;
        }
        internal void StartAuthentication(FingerprintManager fingerprintManager, FingerprintManager.CryptoObject cryptoObject)
        {
            CancellationSignal cancellationSignal = new CancellationSignal();
            if (ActivityCompat.CheckSelfPermission(mainActivity, Manifest.Permission.UseFingerprint)
                != (int)Android.Content.PM.Permission.Granted)
                return;
            fingerprintManager.Authenticate(cryptoObject, cancellationSignal, 0, this, null);
        }
        public override void OnAuthenticationFailed()
        {
            Toast.MakeText(mainActivity, "Fingerprint Authentication failed!", ToastLength.Long).Show();
        }
        public override void OnAuthenticationSucceeded(FingerprintManager.AuthenticationResult result)
        {
            mainActivity.StartActivity(new Intent(mainActivity, typeof(HomeActivity)));
        }
    }
}
