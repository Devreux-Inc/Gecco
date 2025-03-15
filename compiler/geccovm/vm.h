//
// Created by wylan on 12/19/24.
//

#ifndef vm_h
#define vm_h

/* A Virtual Machine vm-h < Calls and Functions vm-include-object
#include "chunk.h"
*/
#include "../object.h"
#include "../table.h"
#include "../value.h"

//< stack-max
/* A Virtual Machine stack-max < Calls and Functions frame-max
#define STACK_MAX 256
*/

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
  ObjClosure* closure;
  uint8_t* ip;
  Value* slots;
} CallFrame;

// Module system structures
typedef struct {
  ObjString* name;
  Table exports;  // Exported symbols from this module
} Module;

typedef struct {
  int count;
  int capacity;
  Module* modules;
  Table moduleNames;  // Maps module names to indices
} ModuleRegistry;

typedef struct {
  CallFrame frames[FRAMES_MAX];
  int frameCount;
  Value stack[STACK_MAX];
  Value* stackTop;
  Table globals;
  Table strings;
  ObjString* initString;
  ObjUpvalue* openUpvalues;
  size_t bytesAllocated;
  size_t nextGC;
  Obj* objects;
  int grayCount;
  int grayCapacity;
  Obj** grayStack;
  
  // Module system
  ModuleRegistry moduleRegistry;
  bool isExporting;  // Flag to track if we're exporting a symbol
  bool isImporting;  // Flag to indicate we're importing a module (collecting exports)
  ObjString* currentModule;  // Current module being processed
} VM;

typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

// These functions need to be externally visible
extern void initVM();
extern void freeVM();
extern InterpretResult interpret(const char* source);
extern InterpretResult interpretInclude(const char* path);
extern void push(Value value);
extern Value pop();

// Module system exports
extern Module* findModule(ObjString* name);
extern Module* createModule(ObjString* name);
extern bool findExportedSymbol(ObjString* name, Value* value);

#endif //vm_h
