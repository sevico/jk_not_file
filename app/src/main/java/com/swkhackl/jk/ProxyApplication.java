package com.swkhackl.jk;

import android.app.Application;
import android.app.Instrumentation;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.util.ArrayMap;

import java.io.File;
import java.lang.ref.WeakReference;
import java.util.ArrayList;

import dalvik.system.DexClassLoader;

public class ProxyApplication extends Application {
    static {
        System.loadLibrary("FShell");
    }
//    protected  native int fromNative();
    String mOldAppName=null;
    @Override
    public void onCreate() {
        super.onCreate();
//        int a = fromNative();
        mOldAppName = ShellInfo.getApplicationName();

        if(mOldAppName==null){
            return;
        }

        Object currentActivityThread = JavaRef.invokeStaticMethod("android.app.ActivityThread","currentActivityThread",new Class[]{},new Class[]{});
        Object mBoundApplication = JavaRef.getFieldObject("android.app.ActivityThread", "mBoundApplication", currentActivityThread);
        Object loadedApkInfo = JavaRef.getFieldObject("android.app.ActivityThread$AppBindData", "info",mBoundApplication);
        JavaRef.setFieldObject("android.app.LoadedApk","mApplication",loadedApkInfo,null);
        String className = mOldAppName;
        ApplicationInfo appInfoLoadedApke = (ApplicationInfo) JavaRef.getFieldObject("android.app.LoadedApk", "mApplicationInfo",loadedApkInfo);
        appInfoLoadedApke.className = className;


        ApplicationInfo appinfoInAppBindData = (ApplicationInfo) JavaRef.getFieldObject("android.app.ActivityThread$AppBindData", "appInfo",mBoundApplication);
        appinfoInAppBindData.className = className;

        Application oldApplication = (Application) JavaRef.getFieldObject("android.app.ActivityThread", "mInitialApplication",currentActivityThread);
        ArrayList<Application> mAllApplications = (ArrayList<Application>) JavaRef.getFieldObject("android.app.ActivityThread", "mAllApplications",currentActivityThread);
        mAllApplications.remove(oldApplication);

        Application orgApp = (Application) JavaRef.invokeMethod("android.app.LoadedApk", "makeApplication",new Class[]{boolean.class,Instrumentation.class}, loadedApkInfo,new Object[]{false,null});
        orgApp.onCreate();                      //makeApplication内部不会调用onCreate，这里自己调用下
        JavaRef.setFieldObject("android.app.ActivityThread","mInitialApplication", currentActivityThread,orgApp);
    }

    @Override
    protected native void attachBaseContext(Context base);
//    protected void attachBaseContext(Context base) {
//        super.attachBaseContext(base);
//        File cache = getDir("shell",MODE_PRIVATE);
//        String srcDex = cache + "/encrypt.dex";
//        File dexFile = FileManager.releaseAssetsFile(this,"encrypt.dex",srcDex,null);
//        DexClassLoader dl = new DexClassLoader(srcDex,getDir("swkhack",MODE_PRIVATE).getAbsolutePath(),
//                            getApplicationInfo().nativeLibraryDir,getClassLoader());
//
//
//        Object currentActivityThread = JavaRef.invokeStaticMethod("android.app.ActivityThread","currentActivityThread",new Class[]{},new Class[]{});
//        ArrayMap mPakages = (ArrayMap) JavaRef.getFieldObject("android.app.ActivityThread","mPackages",currentActivityThread);
//        WeakReference wr = (WeakReference)mPakages.get(getPackageName());
//        JavaRef.setFieldObject("android.app.LoadedApk","mClassLoader",wr.get(),dl);
//
//
//        return;
//    }
}
