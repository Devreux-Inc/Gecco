//
// Created by wylan on 12/19/24.
//

#ifndef compiler_h
#define compiler_h

#include "../object.h"

ObjFunction* compile(const char* source, ObjString* moduleName);
void markCompilerRoots();

#endif //compiler_h
