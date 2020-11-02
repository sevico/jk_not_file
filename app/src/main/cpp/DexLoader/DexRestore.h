//
// Created by swkhack on 2020/1/14.
//

#ifndef JK_DEXRESTORE_H
#define JK_DEXRESTORE_H

#include <vm/Common.h>
#include <vm/Dalvik.h>
#include <map>
struct CMethodInfo{
    u4 methodIndex;
    DexCode code_item;
};



class DexRestore {
public:
    DexRestore(const u1 *pDexMemory, int dexLen, const u1 *pDexCodeInfo, int codeInfoLen);

    bool tryFixClass(const char *className);

    bool parseMethodInfoData(const u1* pDexCodeInfo,int codeInfoLen);

private:

    u1* writeUnsignedLeb128Ext(u1* ptr,u4 data,int wLen){
        int wted=0;
        while (data>0x7f){
            *ptr++=(data&0x7f)|0x80;
            data>>=7;
            wted++;
        }
        if(wted>=wLen-1){
            *ptr++=(data&0x7f)|0x80;
            data>>=7;
        }else{
            while(wted<wLen-1){
                *ptr++=(data&0x7f)|0x80;
                data>>=7;
                wted++;
            }
            *ptr++=0;
        }
        return ptr;
    }

    std::map<u4,CMethodInfo*> mMethodMap;
    DexFile* pDexFile;
};
extern  DexRestore *pDexRestore;

#endif //JK_DEXRESTORE_H
