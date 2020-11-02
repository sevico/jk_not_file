//===------------------------------------------------------------*- C++ -*-===//
//
//                     Created by F8LEFT on 2017/4/18.
//                   Copyright (c) 2017. All rights reserved.
//===--------------------------------------------------------------------------
//
//===----------------------------------------------------------------------===//

#include "JniInfo.h"
#include <sys/system_properties.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>
#include <asm/mman.h>
#include <sys/mman.h>
#include <stdlib.h>
#include "AndroidDef.h"


bool isNumber(char a) {
    if(a >= '0' && a <='9') {
        return true;
    }
    return false;
}

bool isNumber(char a[]) {
    for (int i = 0; a[i]; i++) {
        if (!isNumber(a[i])) {
            return false;
        }
    }
    return true;
}

namespace JniInfo {
    // variable define in namespace JniInfo
    std::string packageName;
    std::string apkPath;
    std::string libPath;
    std::string filePath;
    std::string cachePath;
    int sdk_int = -1;
    JavaVM* vm = nullptr;
    bool isDalvik = false;
    bool isArt = false;
}

bool JniInfo::attach(JavaVM *vm, JNIEnv *env) {
    JniInfo::vm = vm;
    sdk_int = getSdkVersion();
    isArt = isArtMode();
    isDalvik = !isArt;
    return true;
}

bool JniInfo::init(JNIEnv* env, jobject context_obj) {
    // get packageName
    jstring pkgName_jstr = (jstring)CallObjectMethod(
            env, context_obj, "getPackageName", "()Ljava/lang/String;");
    packageName = jstrToCstr(env, pkgName_jstr);
    env->DeleteLocalRef(pkgName_jstr);

    // get apkPath
    jstring apkPath_jstr = (jstring)CallObjectMethod(
            env, context_obj, "getPackageResourcePath", "()Ljava/lang/String;");
    apkPath = jstrToCstr(env, apkPath_jstr);
    env->DeleteLocalRef(apkPath_jstr);

    // get native library path
    jobject appInfo_obj = CallObjectMethod(
            env, context_obj, "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
    jstring nativeLibraryDir_jstr = (jstring) GetObjectField(
            env, appInfo_obj, "nativeLibraryDir", "Ljava/lang/String;");
    libPath = jstrToCstr(env, nativeLibraryDir_jstr);
    env->DeleteLocalRef(appInfo_obj);
    env->DeleteLocalRef(nativeLibraryDir_jstr);

    // get fileData path
    jobject filesDir_obj = CallObjectMethod(
            env, context_obj, "getFilesDir", "()Ljava/io/File;");
    jstring filesDir_jstr = (jstring)CallObjectMethod(
            env, filesDir_obj, "getAbsolutePath", "()Ljava/lang/String;");
    filePath = jstrToCstr(env, filesDir_jstr);
    env->DeleteLocalRef(filesDir_obj);
    env->DeleteLocalRef(filesDir_jstr);

    // setup cachePath (and mkdir if not exit)
    cachePath = filePath.substr(0, filePath.rfind('/')) + "/.cache";
    mkdir(cachePath.c_str(), 0700);

    FLOGD(==============Global Jni Information==================);
    FLOGD(packagename %s, packageName.c_str());
    FLOGD(apkPath %s, apkPath.c_str());
    FLOGD(libPath %s, libPath.c_str());
    FLOGD(filePath %s, filePath.c_str());
    FLOGD(cachePath %s, cachePath.c_str());
    FLOGD(sdk is %d, sdk_int);
    if(isArt) {
        FLOGD(device is running in art mode);
    } else {
        FLOGD(device is running in dalvik mode);
    }
    if(is64Bit()) {
        FLOGD(process is running in 64 bit);
    } else {
        FLOGD(process is running in 32 bit);
    }

    FLOGD(============Global Jni Information End================);
    return true;
}

bool JniInfo::isArtMode() {
    char value1[PROP_VALUE_MAX];
    char value2[PROP_VALUE_MAX];
    if(__system_property_get("persist.sys.dalvik.vm.lib", value1) > 0 &&
            strncmp(value1, "libart", 6u) == 0) {
        return true;
    }
    if(__system_property_get("persist.sys.dalvik.vm.lib.2", value2) > 0&&
            strncmp(value2, "libart", 6u) == 0) {
        return true;
    }
    if(strncmp(value1, "libdvm", 6) == 0 ||
            strncmp(value2, "libdvm", 6) == 0) {
        return false;
    }
    if(sdk_int <= 19) {
        return false;
    }
    FLOGE(Unable to check runtim version);
    return true;
}

int JniInfo::getSdkVersion() {
    char value[PROP_VALUE_MAX];
    if(__system_property_get("ro.build.version.sdk", value) <= 0) {
        return -1;
    }
    int rel = 0;
    sscanf(value, "%d", &rel);
    return rel;
}

std::string JniInfo::jstrToCstr(JNIEnv *env, jstring jstr) {
    if(jstr == nullptr) {
        return std::move(std::string());
    }
    jboolean isCopy;
    const char* str = env->GetStringUTFChars(jstr, &isCopy);
    std::string rel = str;
    if(isCopy == JNI_TRUE) {
        env->ReleaseStringUTFChars(jstr, str);
    }
    return std::move(rel);
}

int JniInfo::getTracePid() {
    char buf[0x400] = {0};
    FILE *fd = fopen("/proc/self/status", "r");
    if (fd == NULL) {
        return 0;
    }

    bool findTPid = false;
    std::string tPidStr = "TracerPid:";
    while (!findTPid && fgets(buf, 0x400, fd)) {
        findTPid = !memcmp(buf, tPidStr.c_str(), 0xA);
    }
    fclose(fd);
    if (!findTPid) {
        return false;
    }
//    LOGE("TracerPid is :%s", buf);
    int pNum;
    for(pNum = 0; !isNumber(buf[pNum]); pNum++);
    int tid = atoi(&buf[pNum]);
    if (tid != 0) {
        FLOGD(A TracerPid has been found: %d, tid);
    }
    return tid;
}


JniInfo::RuntimeVersion JniInfo::getRuntimeVersion() {

#if defined(__arm__)
    return ARM;
#elif defined(__aarch64__)
    return ARM64;
#elif defined(__i386__)
    return X86;
#elif defined(__x86_64__)
    return X64;
#elif defined(__mips64__)  /* mips64el-* toolchain defines __mips__ too */
    return MIPS64;
#elif defined(__mips__)
    return MIPS;
#endif
    return UNKNOWN;
}

bool JniInfo::is64Bit() {
    switch (getRuntimeVersion()) {
        case X64:
        case ARM64:
        case MIPS64:
            return true;
        default:
            return false;
    }
}

//=========------------------------------------------------------
// expended jni bridge
#define CALL_VIRTUAL(_ctype, _jname, _retfail)                              \
     _ctype JniInfo::Call##_jname##Method(JNIEnv* env, jobject obj,         \
        const char *name, const char *sig, ...)                             \
    {                                                                       \
         va_list  args;                                                     \
         va_start(args, sig);                                               \
         VarArgs vargs(args);                                               \
         va_end(args);                                                      \
         return Call##_jname##MethodV(env, obj, name, sig, vargs.vaargs()); \
    }                                                                       \
     _ctype JniInfo::Call##_jname##MethodV(JNIEnv* env, jobject obj,        \
        const char *name, const char *sig, va_list args)                    \
    {                                                                       \
        assert(name != nullptr && sig != nullptr);                          \
        if(obj == nullptr) {                                                \
            return _retfail;                                                \
        }                                                                   \
        jclass clazz = env->GetObjectClass(obj);                            \
        AutoJniRefRelease _autoRel = AutoJniRefRelease(env, clazz);         \
        jmethodID methodId = env->GetMethodID(clazz, name, sig);            \
        if(methodId == nullptr) {                                           \
            FLOGE(unable to call method %s %s, name, sig);                  \
            env->ExceptionClear();                                          \
            return _retfail;                                                \
        }                                                                   \
        return env->Call##_jname##MethodV(obj, methodId, args);             \
    }                                                                       \
     _ctype JniInfo::Call##_jname##MethodA(JNIEnv* env, jobject obj,        \
        const char *name, const char *sig, jvalue* args)                    \
    {                                                                       \
        assert(name != nullptr && sig != nullptr);                          \
        if(obj == nullptr) {                                                \
            return _retfail;                                                \
        }                                                                   \
        jclass clazz = env->GetObjectClass(obj);                            \
        jmethodID methodId = env->GetMethodID(clazz, name, sig);            \
        AutoJniRefRelease _autoRel = AutoJniRefRelease(env, clazz);         \
        if(methodId == nullptr) {                                           \
            FLOGE(unable to call method %s %s, name, sig);                  \
            env->ExceptionClear();                                          \
            return _retfail;                                                \
        }                                                                   \
        return env->Call##_jname##MethodA(obj, methodId, args);             \
    }

CALL_VIRTUAL(jobject, Object, nullptr)
CALL_VIRTUAL(jboolean, Boolean, 0)
CALL_VIRTUAL(jbyte, Byte, 0)
CALL_VIRTUAL(jchar, Char, 0)
CALL_VIRTUAL(jshort, Short, 0)
CALL_VIRTUAL(jint, Int, 0)
CALL_VIRTUAL(jlong, Long, 0)
CALL_VIRTUAL(jfloat, Float, 0.0f)
CALL_VIRTUAL(jdouble, Double, 0.0)
CALL_VIRTUAL(void, Void, )

#define CALL_NONVIRTUAL(_ctype, _jname, _retfail)                              \
     _ctype JniInfo::CallNonvirtual##_jname##Method(JNIEnv* env, jobject obj,  \
        const char* classSig, const char *name, const char *sig, ...)       \
    {                                                                       \
         va_list  args;                                                     \
         va_start(args, sig);                                               \
         VarArgs vargs(args);                                               \
         va_end(args);                                                      \
         return CallNonvirtual##_jname##MethodV(env, obj, classSig, name, sig, vargs.vaargs()); \
    }                                                                       \
     _ctype JniInfo::CallNonvirtual##_jname##MethodV(JNIEnv* env, jobject obj, \
        const char* classSig, const char *name, const char *sig, va_list args) \
    {                                                                       \
        assert(classSig != nullptr && name != nullptr && sig != nullptr);   \
        if(obj == nullptr) {                                                \
            return _retfail;                                                \
        }                                                                   \
        jclass clazz = env->FindClass(classSig);                            \
        if(clazz == nullptr) {                                              \
            FLOGE(unable to find clazz %s, classSig);                       \
            return _retfail;                                                \
        }                                                                   \
        AutoJniRefRelease _autoRel = AutoJniRefRelease(env, clazz);         \
        jmethodID methodId = env->GetMethodID(clazz, name, sig);            \
        if(methodId == nullptr) {                                           \
            FLOGE(unable to call method %s %s, name, sig);                  \
            env->ExceptionClear();                                          \
            return _retfail;                                                \
        }                                                                   \
        return env->CallNonvirtual##_jname##MethodV(obj, clazz, methodId, args);  \
    }                                                                       \
     _ctype JniInfo::CallNonvirtual##_jname##MethodA(JNIEnv* env, jobject obj,  \
        const char* classSig, const char *name, const char *sig, jvalue* args)  \
    {                                                                       \
        assert(classSig != nullptr && name != nullptr && sig != nullptr);   \
        if(obj == nullptr) {                                                \
            return _retfail;                                                \
        }                                                                   \
        jclass clazz = env->FindClass(classSig);                            \
        if(clazz == nullptr) {                                              \
            FLOGE(unable to find clazz %s, classSig);                       \
            return _retfail;                                                \
        }                                                                   \
        jmethodID methodId = env->GetMethodID(clazz, name, sig);            \
        AutoJniRefRelease _autoRel = AutoJniRefRelease(env, clazz);         \
        if(methodId == nullptr) {                                           \
            FLOGE(unable to call method %s %s, name, sig);                  \
            env->ExceptionClear();                                          \
            return _retfail;                                                \
        }                                                                   \
        return env->CallNonvirtual##_jname##MethodA(obj, clazz, methodId, args); \
    }

CALL_NONVIRTUAL(jobject, Object, nullptr)
CALL_NONVIRTUAL(jboolean, Boolean, 0)
CALL_NONVIRTUAL(jbyte, Byte, 0)
CALL_NONVIRTUAL(jchar, Char, 0)
CALL_NONVIRTUAL(jshort, Short, 0)
CALL_NONVIRTUAL(jint, Int, 0)
CALL_NONVIRTUAL(jlong, Long, 0)
CALL_NONVIRTUAL(jfloat, Float, 0.0f)
CALL_NONVIRTUAL(jdouble, Double, 0.0)
CALL_NONVIRTUAL(void, Void, )

#define CALL_STATIC(_ctype, _jname, _retfail)                               \
     _ctype JniInfo::CallStatic##_jname##Method(JNIEnv* env,                \
        const char* classSig, const char *name, const char *sig, ...)       \
    {                                                                       \
         va_list  args;                                                     \
         va_start(args, sig);                                               \
         VarArgs vargs(args);                                               \
         va_end(args);                                                      \
         return CallStatic##_jname##MethodV(env, classSig, name, sig, vargs.vaargs()); \
    }                                                                       \
     _ctype JniInfo::CallStatic##_jname##MethodV(JNIEnv* env,               \
        const char* classSig, const char *name, const char *sig, va_list args) \
    {                                                                       \
        assert(classSig != nullptr && name != nullptr && sig != nullptr);   \
        jclass clazz = env->FindClass(classSig);                            \
        if(clazz == nullptr) {                                              \
            FLOGE(unable to find clazz %s, classSig);                       \
            return _retfail;                                                \
        }                                                                   \
        AutoJniRefRelease _autoRel = AutoJniRefRelease(env, clazz);         \
        jmethodID methodId = env->GetStaticMethodID(clazz, name, sig);      \
        if(methodId == nullptr) {                                           \
            FLOGE(unable to call method %s %s, name, sig);                  \
            env->ExceptionClear();                                          \
            return _retfail;                                                \
        }                                                                   \
        return env->CallStatic##_jname##MethodV(clazz, methodId, args);     \
    }                                                                       \
     _ctype JniInfo::CallStatic##_jname##MethodA(JNIEnv* env,               \
        const char* classSig, const char *name, const char *sig, jvalue* args)  \
    {                                                                       \
        assert(classSig != nullptr && name != nullptr && sig != nullptr);   \
        jclass clazz = env->FindClass(classSig);                            \
        if(clazz == nullptr) {                                              \
            FLOGE(unable to find clazz %s, classSig);                       \
            return _retfail;                                                \
        }                                                                   \
        jmethodID methodId = env->GetStaticMethodID(clazz, name, sig);      \
        AutoJniRefRelease _autoRel = AutoJniRefRelease(env, clazz);         \
        if(methodId == nullptr) {                                           \
            FLOGE(unable to call method %s %s, name, sig);                  \
            env->ExceptionClear();                                          \
            return _retfail;                                                \
        }                                                                   \
        return env->CallStatic##_jname##MethodA(clazz, methodId, args);     \
    }

CALL_STATIC(jobject, Object, nullptr)
CALL_STATIC(jboolean, Boolean, 0)
CALL_STATIC(jbyte, Byte, 0)
CALL_STATIC(jchar, Char, 0)
CALL_STATIC(jshort, Short, 0)
CALL_STATIC(jint, Int, 0)
CALL_STATIC(jlong, Long, 0)
CALL_STATIC(jfloat, Float, 0.0f)
CALL_STATIC(jdouble, Double, 0.0)
CALL_STATIC(void, Void, )

#define GET_FIELD(_ctype, _jname, _retfail)                                   \
    _ctype JniInfo::Get##_jname##Field(JNIEnv* env, jobject obj,              \
        const char* name, const char* sig)                                    \
        {                                                                     \
            assert(name != nullptr && sig != nullptr);                        \
            if(obj == nullptr) {                                              \
                return _retfail;                                              \
            }                                                                 \
            jclass clazz = env->GetObjectClass(obj);                          \
            AutoJniRefRelease _autoRel1 = AutoJniRefRelease(env, clazz);      \
            jfieldID fieldId = env->GetFieldID(clazz, name, sig);             \
            if(fieldId == nullptr) {                                          \
                FLOGE(unable to get field %s %s, name, sig);                  \
                env->ExceptionClear();                                        \
                return _retfail;                                              \
            }                                                                 \
            return env->Get##_jname##Field(obj, fieldId);                     \
        }                                                                     \
    _ctype JniInfo::GetStatic##_jname##Field(JNIEnv*env, const char* classSig,\
        const char* name, const char* sig)                                    \
        {                                                                     \
            assert(classSig != nullptr && name != nullptr && sig != nullptr); \
            jclass clazz = env->FindClass(classSig);                          \
            if(clazz == nullptr) {                                            \
                FLOGE(Unable to find class %s, classSig);                     \
                return _retfail;                                              \
            }                                                                 \
            AutoJniRefRelease _autoRel1 = AutoJniRefRelease(env, clazz);      \
            jfieldID fieldId = env->GetStaticFieldID(clazz, name, sig);       \
            if(fieldId == nullptr) {                                          \
                FLOGE(unable to get field %s %s, name, sig);                  \
                env->ExceptionClear();                                        \
                return _retfail;                                              \
            }                                                                 \
            return env->GetStatic##_jname##Field(clazz, fieldId);             \
        }

GET_FIELD(jobject, Object, nullptr)
GET_FIELD(jboolean, Boolean, 0)
GET_FIELD(jbyte, Byte, 0)
GET_FIELD(jchar, Char, 0)
GET_FIELD(jshort, Short, 0)
GET_FIELD(jint, Int, 0)
GET_FIELD(jlong, Long, 0)
GET_FIELD(jfloat, Float, 0.0f)
GET_FIELD(jdouble, Double, 0.0)

#define SET_FIELD(_ctype, _jname)                                             \
    bool JniInfo::Set##_jname##Field(JNIEnv* env, jobject obj,                \
        const char* name, const char* sig, _ctype val)                        \
        {                                                                     \
            assert(name != nullptr && sig != nullptr);                        \
            if(obj == nullptr) {                                              \
                return false;                                                 \
            }                                                                 \
            jclass clazz = env->GetObjectClass(obj);                          \
            AutoJniRefRelease _autoRel1 = AutoJniRefRelease(env, clazz);      \
            jfieldID fieldId = env->GetFieldID(clazz, name, sig);             \
            if(fieldId == nullptr) {                                          \
                FLOGE(unable to set field %s %s, name, sig);                  \
                env->ExceptionClear();                                        \
                return false;                                                 \
            }                                                                 \
            env->Set##_jname##Field(obj, fieldId, val);                       \
            return true;                                                      \
        }                                                                     \
    bool JniInfo::SetStatic##_jname##Field(JNIEnv*env, const char* classSig,  \
        const char* name, const char* sig, _ctype val)                        \
        {                                                                     \
            assert(classSig != nullptr && name != nullptr && sig != nullptr); \
            jclass clazz = env->FindClass(classSig);                          \
            if(clazz == nullptr) {                                            \
                FLOGE(Unable to find class %s, classSig);                     \
                return false;                                                 \
            }                                                                 \
            AutoJniRefRelease _autoRel1 = AutoJniRefRelease(env, clazz);      \
            jfieldID fieldId = env->GetStaticFieldID(clazz, name, sig);       \
            if(fieldId == nullptr) {                                          \
                FLOGE(unable to set field %s %s, name, sig);                  \
                env->ExceptionClear();                                        \
                return false;                                                 \
            }                                                                 \
            env->SetStatic##_jname##Field(clazz, fieldId, val);               \
            return true;                                                      \
        }

SET_FIELD(jobject, Object)
SET_FIELD(jboolean, Boolean)
SET_FIELD(jbyte, Byte)
SET_FIELD(jchar, Char)
SET_FIELD(jshort, Short)
SET_FIELD(jint, Int)
SET_FIELD(jlong, Long)
SET_FIELD(jfloat, Float)
SET_FIELD(jdouble, Double)


