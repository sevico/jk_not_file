//
// Created by swkhack on 2020/1/15.
//

#ifndef JK_DVMLOADER_H
#define JK_DVMLOADER_H
#include <vm/Common.h>
//#include "Common.h"

class DvmLoader {
public:
    static bool  initEnv();
    void *loadFromMemory(const u1* pDex,u4 len);

};


#endif //JK_DVMLOADER_H
