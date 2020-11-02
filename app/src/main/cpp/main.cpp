

#include "jni.h"
#include "ProxyApplication.h"


int fromNative(JNIEnv *env,jobject obj){

    return 1;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm,void* reser){
    JNIEnv *env = nullptr;
    if(vm->GetEnv((void**)&env,JNI_VERSION_1_6)!=JNI_OK){
        return -1;
    }
    ProxyApplication::registerNativeMethod(env);

//    auto clazz = env->FindClass("com/swkhackl/jk/ProxyApplication");
//    JNINativeMethod methods[]={
//            "fromNative","()I",(void*)fromNative
//    };
//    env->RegisterNatives(clazz,methods,1);

    return JNI_VERSION_1_6;
}