//
// Created by swkhack on 2019/12/31.
//

#ifndef JK_PROXYAPPLICATION_H
#define JK_PROXYAPPLICATION_H


#include "jni.h"


class ProxyApplication {
public:
    static void attachBaseContext(JNIEnv *env,jobject obj,jobject context);
    static bool registerNativeMethod(JNIEnv *env);

};


#endif //JK_PROXYAPPLICATION_H
