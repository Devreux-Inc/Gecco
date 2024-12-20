//
// Created by wylan on 12/19/24.
//

#ifndef compiler_h
#define compiler_h

#include "object.h"
//< Strings compiler-include-object
//> Compiling Expressions compile-h
#include "vm.h"

ObjFunction* compile(const char* source);
void markCompilerRoots();


#endif //compiler_h
