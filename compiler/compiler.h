//
// Created by wylan on 12/19/24.
//

#ifndef compiler_h
#define compiler_h

//> Strings compiler-include-object
#include "object.h"
//< Strings compiler-include-object
//> Compiling Expressions compile-h
#include "vm.h"

//< Compiling Expressions compile-h
/* Scanning on Demand compiler-h < Compiling Expressions compile-h
void compile(const char* source);
*/
/* Compiling Expressions compile-h < Calls and Functions compile-h
bool compile(const char* source, Chunk* chunk);
*/
//> Calls and Functions compile-h
ObjFunction* compile(const char* source);
//< Calls and Functions compile-h
//> Garbage Collection mark-compiler-roots-h
void markCompilerRoots();
//< Garbage Collection mark-compiler-roots-h


#endif //compiler_h
