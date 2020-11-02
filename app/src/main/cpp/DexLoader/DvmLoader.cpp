//
// Created by swkhack on 2020/1/15.
//

#include "DvmLoader.h"
#include "../AndroidDef/vm/DvmDex.h"
#include "DexRestore.h"
#include "../FAInHook/FAInHook.h"
#include <vm/DvmDex.h>
#include <dlfcn.h>
#include <vm/native.h>
#include "../FAInHook/FAInHook.h"
void* (*oldDvmDefineClass)(DvmDex*,const char*,Object*) = nullptr;

void* myDvmDefineClass(DvmDex *pDvmDex,const char* descriptor,Object *classLoader){
    pDexRestore->tryFixClass(descriptor);

    return oldDvmDefineClass(pDvmDex,descriptor,classLoader);

}
static int hashcmpDexOrJar(const void* tableVal, const void* newVal)
{
    return (long) newVal - (long) tableVal;
}
void* gDvm_userDexFiles = nullptr;
static void addToDexFileTable(DexOrJar* pDexOrJar) {
    /*
     * Later on, we will receive this pointer as an argument and need
     * to find it in the hash table without knowing if it's valid or
     * not, which means we can't compute a hash value from anything
     * inside DexOrJar. We don't share DexOrJar structs when the same
     * file is opened multiple times, so we can just use the low 32
     * bits of the pointer as the hash.
     */
    u4 hash = (u4) (long)pDexOrJar;
    void* result;

    dvmHashTableLock(static_cast<HashTable *>(gDvm_userDexFiles));
    result = dvmHashTableLookup(static_cast<HashTable *>(gDvm_userDexFiles), hash, pDexOrJar,
                                hashcmpDexOrJar, true);
    dvmHashTableUnlock(static_cast<HashTable *>(gDvm_userDexFiles));

    if (result != pDexOrJar) {
        ALOGE("Pointer has already been added?");
//        dvmAbort();
    }

    pDexOrJar->okayToFree = true;
}



void f_hashTableFree(HashTable* pHashTable){
    gDvm_userDexFiles = pHashTable;
    return;
}


bool DvmLoader::initEnv() {
    auto libDvm = dlopen("libdvm.so",RTLD_NOW);
    auto pDefineClass = dlsym(libDvm,"_Z14dvmDefineClassP6DvmDexPKcP6Object");

    auto inHook = FAInHook::instance();
    inHook->registerHook((Elf_Addr)pDefineClass,(Elf_Addr)myDvmDefineClass,(Elf_Addr*)&oldDvmDefineClass);

    auto p_hashTableFree = dlsym(libDvm,"_Z16dvmHashTableFreeP9HashTable");

    inHook->registerHook(reinterpret_cast<Elf_Addr>(p_hashTableFree), reinterpret_cast<Elf_Addr>(f_hashTableFree), nullptr);
    inHook->hookAll();

    auto dvmInternalNativeShutdown = (void(*)())dlsym(libDvm,"_Z25dvmInternalNativeShutdownv");
    dvmInternalNativeShutdown();
    inHook->unhook(reinterpret_cast<Elf_Addr>(p_hashTableFree));
    return false;
}

void *DvmLoader::loadFromMemory(const u1 *pDex, u4 len) {
    auto libDvm = dlopen("libdvm.so",RTLD_NOW);
    auto dvmDexFileOpenPartial = (int (*)(const void* addr, int len, DvmDex** ppDvmDex))dlsym(libDvm,"_Z21dvmDexFileOpenPartialPKviPP6DvmDex");
    auto dexCreateClassLookup = (DexClassLookup* (*)(DexFile* pDexFile))dlsym(libDvm,"_Z20dexCreateClassLookupP7DexFile");

    DvmDex* pDvmDex = nullptr;
    if(dvmDexFileOpenPartial(pDex,len,&pDvmDex)!=0){
        return nullptr;
    }

    auto pClassLookup = dexCreateClassLookup(pDvmDex->pDexFile);
    pDvmDex->pDexFile->pClassLookup = pClassLookup;

    auto pRawDexFile = (RawDexFile*)malloc(sizeof(RawDexFile));
    pRawDexFile->cacheFileName= nullptr;
    pRawDexFile->pDvmDex=pDvmDex;
    DexOrJar* pDexOrJar = NULL;

    pDexOrJar = (DexOrJar*) malloc(sizeof(DexOrJar));
    pDexOrJar->isDex = true;
    pDexOrJar->pRawDexFile = pRawDexFile;
    pDexOrJar->pDexMemory = const_cast<u1 *>(pDex);
    pDexOrJar->fileName = strdup("<memory>");
    addToDexFileTable(pDexOrJar);

    return pDexOrJar;
}
