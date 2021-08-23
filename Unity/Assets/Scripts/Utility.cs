#define USE_ANDROID
using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;


public static class Utility
{
    public static void Invoke(this MonoBehaviour mb, Action f, float delay)
    {
        mb.StartCoroutine(InvokeRoutine(f, delay));
    }

    private static IEnumerator InvokeRoutine(System.Action f, float delay)
    {
        yield return new WaitForSeconds(delay);
        f();
    }

    public static float RemapClamped(float aValue, float aIn1, float aIn2, float aOut1, float aOut2)
    {
        float t = (aValue - aIn1) / (aIn2 - aIn1);
        if (t > 1f)
            return aOut2;
        if (t < 0f)
            return aOut1;
        return aOut1 + (aOut2 - aOut1) * t;
    }

    public static void SetupAndroidTheme(int primaryARGB, int darkARGB, string label = null)
    {
        //#if UNITY_ANDROID && !UNITY_EDITOR
        label = label ?? Application.productName;
        Screen.fullScreen = true;
        AndroidJavaObject activity = new AndroidJavaClass("com.unity3d.player.UnityPlayer").GetStatic<AndroidJavaObject>("currentActivity");
        activity.Call("runOnUiThread", new AndroidJavaRunnable(() =>
        {
            AndroidJavaClass layoutParamsClass = new AndroidJavaClass("android.view.WindowManager$LayoutParams");
            int flagFullscreen = layoutParamsClass.GetStatic<int>("FLAG_FULLSCREEN");
            int flagNotFullscreen = layoutParamsClass.GetStatic<int>("FLAG_FORCE_NOT_FULLSCREEN");
            int flagDrawsSystemBarBackgrounds = layoutParamsClass.GetStatic<int>("FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS");
            AndroidJavaObject windowObject = activity.Call<AndroidJavaObject>("getWindow");
            windowObject.Call("clearFlags", flagFullscreen);
            windowObject.Call("addFlags", flagNotFullscreen);
            windowObject.Call("addFlags", flagDrawsSystemBarBackgrounds);
            int sdkInt = new AndroidJavaClass("android.os.Build$VERSION").GetStatic<int>("SDK_INT");
            int lollipop = 21;
            if (sdkInt > lollipop)
            {
                windowObject.Call("setStatusBarColor", darkARGB);
                string myName = activity.Call<string>("getPackageName");
                AndroidJavaObject packageManager = activity.Call<AndroidJavaObject>("getPackageManager");
                AndroidJavaObject drawable = packageManager.Call<AndroidJavaObject>("getApplicationIcon", myName);
                AndroidJavaObject taskDescription = new AndroidJavaObject("android.app.ActivityManager$TaskDescription", label, drawable.Call<AndroidJavaObject>("getBitmap"), primaryARGB);
                activity.Call("setTaskDescription", taskDescription);
            }
        }));
        //#endif
    }

    static public int ToARGB(Color color)
    {
        Color32 c = (Color32)color;
        byte[] b = new byte[] { c.b, c.g, c.r, c.a };
        return System.BitConverter.ToInt32(b, 0);
    }
}


