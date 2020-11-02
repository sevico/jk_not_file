package com.swkhackl.jk;

import android.util.Log;

/**
 * Created by F8LEFT on 2016/3/25.
 */
public class Global {


    public final static String LOGTAG = "sss";

    public static void LogD(String prefix, String info) {
        Log.d(LOGTAG, "[" + prefix + "]" + info);
    }

    public static void LogI(String prefix, String info) {
        Log.i(LOGTAG, "[" + prefix + "]" + info);
    }

    public static void LogE(String prefix, String info) {
        Log.e(LOGTAG, "[" + prefix + "]" + info);
    }
}
