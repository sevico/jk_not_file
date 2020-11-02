//
// Created by F8LEFT on 2017/2/21.
//

#ifndef VMSHELL_BASEDEF_H
#define VMSHELL_BASEDEF_H

#include "MinAndroidDef.h"

bool safe_mul(unsigned *, unsigned a, unsigned b);
bool safe_add(unsigned *, unsigned a, unsigned b);

#include <string>

void StringAppendV(std::string* dst, const char* format, va_list ap);

std::string StringPrintf(const char* fmt, ...);

void StringAppendF(std::string* dst, const char* format, ...);

#endif //VMSHELL_BASEDEF_H
