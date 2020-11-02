//
// Created by swkhack on 2019/12/31.
//

#include "ProxyApplication.h"
#include "JniInfo.h"
#include "JAsset.h"
#include <string>
#include <dlfcn.h>
#include <vm/native.h>
#include <sys/mman.h>

#include "DexLoader/DexRestore.h"
#include "DexLoader/DvmLoader.h"

bool ::ProxyApplication::registerNativeMethod(JNIEnv *env) {

    auto clazz  = env->FindClass("com/swkhackl/jk/ProxyApplication");
    JNINativeMethod methods[]={
            {"attachBaseContext","(Landroid/content/Context;)V",(void*)attachBaseContext}
    };
    env->RegisterNatives(clazz,methods,1);
    return true;
}

bool makeDexElements(JNIEnv *env,jobject classLoader,const std::vector<jobject> &dexFile_objs){
    if(dexFile_objs.empty()){
        return false;
    }
    env->PushLocalFrame(0x10);
    jobject pathList_obj = JniInfo::GetObjectField(env,classLoader,"pathList","Ldalvik/system/DexPathList;");
    jobjectArray dexElements_obj = static_cast<jobjectArray>(JniInfo::GetObjectField(env,
                                                                                     pathList_obj,
                                                                                     "dexElements",
                                                                                     "[Ldalvik/system/DexPathList$Element;"));

    jint dexElementCount = env->GetArrayLength(dexElements_obj);
    jint dexElementsNewCount = dexElementCount+dexFile_objs.size();


    jclass elements_cla = env->FindClass("dalvik/system/DexPathList$Element");
    jobjectArray new_dexElements_obj = env->NewObjectArray(dexElementsNewCount,elements_cla, nullptr);
    for(auto i=0;i<dexElementCount;i++){
        env->SetObjectArrayElement(new_dexElements_obj,i,env->GetObjectArrayElement(dexElements_obj,i));

    }
    jmethodID element_init_mid = env->GetMethodID(elements_cla,"<init>","(Ljava/io/File;ZLjava/io/File;Ldalvik/system/DexFile;)V");
    for(auto i=0;i<dexFile_objs.size();i++){
        jobject new_dexElement = env->NewObject(elements_cla,element_init_mid,nullptr, false,
                                                nullptr,dexFile_objs[i]);
        env->SetObjectArrayElement(new_dexElements_obj,i+dexElementCount,new_dexElement);
    }
    JniInfo::SetObjectField(env,pathList_obj,"dexElements","[Ldalvik/system/DexPathList$Element;",new_dexElements_obj);
    env->PopLocalFrame(nullptr);
    return true;
}


void ::ProxyApplication::attachBaseContext(JNIEnv *env, jobject obj, jobject context) {
    JniInfo::CallNonvirtualVoidMethod(env,obj,"android/content/ContextWrapper","attachBaseContext","(Landroid/content/Context;)V",context);
    if(!JniInfo::init(env,context)){
        return;
    }
    auto assetManager = JAsset::getAssetManagerFromCtx(env,context);

//    JAsset::releaseEncryptFileIntoCache(assetManager,"encrypt.dex",0444);
//    std::string cacheFilePath = JniInfo::cachePath+"/encrypt.dex";
//    std::string cacheFileOpt = JniInfo::cachePath+"/encrypt.odex";
//    jstring cacheFilePath_jstr=env->NewStringUTF(cacheFilePath.c_str());
//    jstring cacheFileOpt_jstr = env->NewStringUTF(cacheFileOpt.c_str());

//    auto dexFile_obj = JniInfo::CallStaticObjectMethod(env,"dalvik/system/DexFile","loadDex","(Ljava/lang/String;Ljava/lang/String;I)Ldalvik/system/DexFile;",
//                                cacheFilePath_jstr,cacheFileOpt_jstr, false);
//    auto dvm = dlopen("libdvm.so",RTLD_NOW);
//    JNINativeMethod *dvm_dalvik_system_DexFile = (JNINativeMethod *) dlsym(dvm,
//                                                                           "dvm_dalvik_system_DexFile");
//
//    void (*fnOpenDexFileNative)(const u4* args,JValue* pResult) = nullptr;
//    auto dvmCreateStringFromCstr = (void* (*)(const char* utf8Str))dlsym(dvm,"_Z23dvmCreateStringFromCstrPKc");
//
//    for(auto p = dvm_dalvik_system_DexFile;p->fnPtr!= nullptr;p++){
//        if(strcmp(p->name,"openDexFileNative")==0 && strcmp(p->signature,"(Ljava/lang/String;Ljava/lang/String;I)I")==0){
//            fnOpenDexFileNative = (void (*)(const u4 *, JValue *))(p->fnPtr);
//            break;
//        }
//    }
//    DexOrJar *pDexOrJar = nullptr;
//    if(fnOpenDexFileNative!= nullptr){
//        u4 args[2];
//        args[0] = static_cast<u4>(reinterpret_cast<uintptr_t >((dvmCreateStringFromCstr(cacheFilePath.c_str()))));
//        args[1] = static_cast<u4>(reinterpret_cast<uintptr_t >((dvmCreateStringFromCstr(cacheFileOpt.c_str()))));
//        JValue result;
//        fnOpenDexFileNative(args,&result);
//        pDexOrJar = reinterpret_cast<DexOrJar *>(result.l);
//    }

    DvmLoader::initEnv();
    u1* pDex;
    u4 pDexLen;
    JAsset::releaseEncryptFileIntoMem(assetManager, "encrypt.dex",
            (const u1 *&)(pDex), pDexLen);

    DvmLoader dvmLoader;
    auto pDexOrJar = dvmLoader.loadFromMemory(pDex,pDexLen);

    auto dexFile_clazz=env->FindClass("dalvik/system/DexFile");
    auto dexFile_obj = env->AllocObject(dexFile_clazz);
    JniInfo::SetIntField(env,dexFile_obj,"mCookie","I",static_cast<u4>(reinterpret_cast<uintptr_t >(pDexOrJar)));

    auto classLoader_obj = JniInfo::CallObjectMethod(env,context,"getClassLoader","()Ljava/lang/ClassLoader;");
    std::vector<jobject> dexFile_objs{dexFile_obj};
    makeDexElements(env,classLoader_obj,dexFile_objs);

    //load data

    const u1* pDexMem = pDex;
    u4 iDexLen = pDexLen;

    mprotect((void*)PAGE_START((u4)(long)pDexMem),iDexLen*2,PROT_WRITE|PROT_READ);



    const u1* pCryptMem;
    u4 iCryptLen;
    const char* encrypt = (char*)"crypt.dat";
    JAsset::releaseEncryptFileIntoMem(assetManager,encrypt,pCryptMem,iCryptLen);
    pDexRestore = new DexRestore(pDexMem,iDexLen,pCryptMem,iCryptLen);
//    DexRestore dex(pDexMem,iDexLen,pCryptMem,iCryptLen);
    return;

}