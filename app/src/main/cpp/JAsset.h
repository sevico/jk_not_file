//===------------------------------------------------------------*- C++ -*-===//
//
//                     Created by F8LEFT on 2017/10/8.
//                   Copyright (c) 2017. All rights reserved.
//===----------------------------------------------------------------------===//
//
//===----------------------------------------------------------------------===//


#ifndef SHELL2_JASSET_H
#define SHELL2_JASSET_H

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <vm/Common.h>

namespace JAsset {
    AAssetManager* getAssetManagerFromCtx(JNIEnv* env, jobject context);

    bool releaseEncryptFileIntoMem(AAssetManager *mgr,  const char *fileName,
                                   const u1 *&pMem, u4 &len);
// read data from asset, and decrypt, and release into cache dir
    bool releaseEncryptFileIntoCache(AAssetManager *mgr, const char *fileName,
                                     mode_t mode);
}
#endif //SHELL2_JASSET_H
