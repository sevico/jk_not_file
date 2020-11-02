//===------------------------------------------------------------*- C++ -*-===//
//
//                     Created by F8LEFT on 2017/10/8.
//                   Copyright (c) 2017. All rights reserved.
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//
#include <sys/mman.h>
#include <sys/stat.h>
#include "JAsset.h"
#include "JniInfo.h"


namespace JAsset {
    bool releaseEncryptFileIntoMem(AAssetManager *mgr, const char *fileName,
                                   const u1 *&pMem, u4 &len) {
        AAsset *pFile = AAssetManager_open(mgr, fileName, AASSET_MODE_STREAMING);
        if (pFile != NULL) {
            len = AAsset_getLength(pFile);

            void* m = mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
            if(m != MAP_FAILED) {
                pMem = (u1*) m;
                char tmp[1024];
                int iRead;
                u1* wPoint = (u1*)pMem;
                auto restLen = len;
                while ((iRead = AAsset_read(pFile, tmp, len > 1024 ? 1024: restLen)) > 0) {
                    memcpy(wPoint, tmp, iRead);
                    wPoint += iRead;
                    restLen -= iRead;
                }
            }

            AAsset_close(pFile);
            return m != MAP_FAILED;
        }
        return false;
    }

// read data from asset, and decrypt, and release into cache dir
    bool releaseEncryptFileIntoCache(AAssetManager *mgr, const char *fileName,
                                     mode_t mode)
    {
        // TODO add hash check to avoid multi release
        AAsset *pFile = AAssetManager_open(mgr, fileName, AASSET_MODE_STREAMING);
        if (pFile != NULL) {
            std::string relPath = JniInfo::cachePath + "/" + fileName;
            auto fp = fopen(relPath.c_str(), "w+");
            if(fp != 0) {
                int fd = fileno(fp);
                fchmod(fd, mode);

                auto fileLen = AAsset_getLength(pFile);
                char buf[1024];
                int iRead;
                while ((iRead = AAsset_read(pFile, buf, 1024)) > 0) {
                    fwrite(buf, 1, iRead, fp);
                }
                fclose(fp);
            } else {
                FLOGE(Unable to write file %s, relPath.c_str());
            }
            AAsset_close(pFile);
            return fp != 0;
        }
        return false;
    }

    AAssetManager *getAssetManagerFromCtx(JNIEnv *env, jobject context) {
        auto assetManager_obj = JniInfo::CallObjectMethod(env, context,
                                                          "getAssets", "()Landroid/content/res/AssetManager;");
        auto assetManager = AAssetManager_fromJava(env, assetManager_obj);
        env->DeleteLocalRef(assetManager_obj);
        return assetManager;
    }
}
