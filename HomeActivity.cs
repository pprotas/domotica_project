using Android.App;
using Android.OS;
using Android.Support.V7.App;
namespace HomeSafe9001
{
    [Activity(Label = "HomeActivity", Theme = "@style/AppTheme")]
    public class HomeActivity : AppCompatActivity
    {
        protected override void OnCreate(Bundle savedInstanceState)
        {
            base.OnCreate(savedInstanceState);
            // Create your application here  
            SetContentView(Resource.Layout.Home);
        }
    }
}
