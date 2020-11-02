//
// Created by swkhack on 2020/1/14.
//

#include <AndroidDef.h>
#include "DexRestore.h"
DexRestore *pDexRestore= nullptr;


DexRestore::DexRestore(const u1 *pDexMemory, int dexLen, const u1 *pDexCodeInfo, int codeInfoLen) {
    parseMethodInfoData(pDexCodeInfo,codeInfoLen);
    pDexFile = dexFileParse(pDexMemory,dexLen,0);
    pDexFile->pClassLookup = dexCreateClassLookup(pDexFile);

}

bool DexRestore::parseMethodInfoData(const u1 *pDexCodeInfo, int codeInfoLen) {
    auto p=  pDexCodeInfo;
    auto pEnd = p+codeInfoLen;
    while(p!=pEnd){
        auto itemLen = *(int*)p;

        CMethodInfo* info = (CMethodInfo*)(p+4);
        mMethodMap[info->methodIndex]=info;

        p=p+itemLen;
    }
    return false;
}

bool DexRestore::tryFixClass(const char *className) {
    auto pClassDef = dexFindClass(pDexFile,className);
    if(pClassDef== nullptr){
        return false;
    }
    //parse classDef
    auto pData = dexGetClassData(pDexFile,pClassDef);
    if(pData== nullptr){
        return false;
    }
    DexClassDataHeader header;
    dexReadClassDataHeader(&pData,&header);
    //skip field
    for(auto i=0;i<header.staticFieldsSize+header.instanceFieldsSize;i++){
        u4 index;
        DexField field;
        dexReadClassDataField(&pData,&field,&index);

    }
    auto methodFixer = [&](const u1** pData,u4* lastIndex){
        //record data ptr
        auto pSave = (u1*)*pData;
        DexMethod method;
        dexReadClassDataMethod(pData,&method,lastIndex);
        auto mCodeInfo = mMethodMap.find(method.methodIdx);
        if(mCodeInfo==mMethodMap.end()){
            return false;
        }
        //find and fix it
        //skip index
        readUnsignedLeb128((const u1 **)&pSave);
        //1. fix flag
        pSave = writeUnsignedLeb128Ext(pSave,method.accessFlags & ~ACC_NATIVE,5);
        //2. fix code offset
        pSave = writeUnsignedLeb128Ext(pSave,(u1*)&mCodeInfo->second->code_item-pDexFile->baseAddr,5);

        return true;
    };
    u4 index=0;
    for(auto i=0;i<header.directMethodsSize;i++){
        methodFixer(&pData,&index);
    }
    index=0;
    for(auto i=0;i<header.virtualMethodsSize;i++){
        methodFixer(&pData,&index);
    }

    return false;
}
