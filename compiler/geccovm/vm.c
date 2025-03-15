//
// Created by wylan on 12/19/24.
//

//> A Virtual Machine vm-c
//> Types of Values include-stdarg

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>  // For malloc() and free()
#include <string.h>
#include <time.h>
#include <unistd.h>  // For getcwd()
#include "../common.h"
#include "../compiler/compiler.h"
#include "../debug/debug.h"
#include "../object.h"
#include "../memory/memory.h"
#include "vm.h"

VM vm; // [one]

static Value clockNative(int argCount, Value *args) {
    return NUMBER_VAL((double) clock() / CLOCKS_PER_SEC);
}

static void resetStack() {
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
    vm.openUpvalues = nullptr;
}

static void runtimeError(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    // Print stack trace
    for (int i = vm.frameCount - 1; i >= 0; i--) {
        CallFrame *frame = &vm.frames[i];
        ObjFunction *function = frame->closure->function;
        size_t instruction = frame->ip - function->chunk.code - 1;
        fprintf(stderr, "[line %d] in ", // [minus]
                function->chunk.lines[instruction]);

        if (function->name == NULL) {
            fprintf(stderr, "script\n");
        } else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    // If we're in import mode, log the error but don't stop execution
    if (vm.isImporting) {
        fprintf(stderr, "WARNING: Error during module import, but continuing execution\n");
        // Don't reset the stack when importing - we need to recover
        return;
    }

    // If we have a runtime error and want to show info, we could add it here
    
    resetStack();
}

static void defineNative(const char *name, NativeFn function) {
    push(OBJ_VAL(copyString(name, (int) strlen(name))));
    push(OBJ_VAL(newNative(function)));
    tableSet(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

static void initModuleRegistry() {
    vm.moduleRegistry.count = 0;
    vm.moduleRegistry.capacity = 0;
    vm.moduleRegistry.modules = nullptr;
    initTable(&vm.moduleRegistry.moduleNames);
}

void initVM() {
    resetStack();
    vm.objects = nullptr;
    vm.bytesAllocated = 0;
    vm.nextGC = 1024 * 1024;

    vm.grayCount = 0;
    vm.grayCapacity = 0;
    vm.grayStack = nullptr;
    
    // Initialize standard tables
    initTable(&vm.globals);
    initTable(&vm.strings);
    
    // Initialize module system
    initModuleRegistry();
    
    // Reset flags
    vm.isExporting = false;  // Reset with each declaration (set to true for 'exp' prefix)
    vm.isImporting = false;  // Not currently processing an import
    vm.currentModule = NULL; // Not in any module context
    
    // Set up the initialize string ("init")
    vm.initString = nullptr;
    vm.initString = copyString("init", 4);
    
    // Register native functions
    defineNative("clock", clockNative);
}

static void freeModuleRegistry() {
    // Free all module exports tables
    for (int i = 0; i < vm.moduleRegistry.count; i++) {
        freeTable(&vm.moduleRegistry.modules[i].exports);
    }
    
    // Free module array
    FREE_ARRAY(Module, vm.moduleRegistry.modules, vm.moduleRegistry.capacity);
    
    // Free module names table
    freeTable(&vm.moduleRegistry.moduleNames);
}

void freeVM() {
    freeTable(&vm.globals);
    freeTable(&vm.strings);
    
    // Free module registry
    freeModuleRegistry();
    
    vm.initString = nullptr;
    freeObjects();
}

void push(Value value) {
    *vm.stackTop = value;
    vm.stackTop++;
}

Value pop() {
    vm.stackTop--;
    return *vm.stackTop;
}

static Value peek(int distance) {
    return vm.stackTop[-1 - distance];
}

static bool call(ObjClosure *closure, int argCount) {
    if (argCount != closure->function->arity) {
        runtimeError("Expected %d arguments but got %d.", closure->function->arity, argCount);
        return false;
    }

    if (vm.frameCount == FRAMES_MAX) {
        runtimeError("Stack overflow.");
        return false;
    }

    CallFrame *frame = &vm.frames[vm.frameCount++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;
    frame->slots = vm.stackTop - argCount - 1;
    return true;
}

static bool callValue(Value callee, int argCount) {
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
            case OBJ_BOUND_METHOD: {
                ObjBoundMethod *bound = AS_BOUND_METHOD(callee);
                vm.stackTop[-argCount - 1] = bound->receiver;
                return call(bound->method, argCount);
            }

            case OBJ_CLASS: {
                ObjClass *klass = AS_CLASS(callee);
                vm.stackTop[-argCount - 1] = OBJ_VAL(newInstance(klass));
                Value initializer;
                if (tableGet(&klass->methods, vm.initString, &initializer)) {
                    return call(AS_CLOSURE(initializer), argCount);
                } else if (argCount != 0) {
                    runtimeError("Expected 0 arguments but got %d.", argCount);
                    return false;
                }

                return true;
            }

            case OBJ_CLOSURE:
                return call(AS_CLOSURE(callee), argCount);
            case OBJ_NATIVE: {
                NativeFn native = AS_NATIVE(callee);
                Value result = native(argCount, vm.stackTop - argCount);
                vm.stackTop -= argCount + 1;
                push(result);
                return true;
            }

            default:
                break; // Non-callable object type.
        }
    }
    runtimeError("Can only call functions and classes.");
    return false;
}

static bool invokeFromClass(ObjClass *klass, ObjString *name, int argCount) {
    Value method;
    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError("Undefined property '%s'.", name->chars);
        return false;
    }
    return call(AS_CLOSURE(method), argCount);
}

static bool invoke(ObjString *name, int argCount) {
    Value receiver = peek(argCount);

    if (!IS_INSTANCE(receiver)) {
        runtimeError("Only instances have methods.");
        return false;
    }

    ObjInstance *instance = AS_INSTANCE(receiver);

    Value value;
    if (tableGet(&instance->fields, name, &value)) {
        vm.stackTop[-argCount - 1] = value;
        return callValue(value, argCount);
    }

    return invokeFromClass(instance->klass, name, argCount);
}

static bool bindMethod(ObjClass *klass, ObjString *name) {
    Value method;
    if (!tableGet(&klass->methods, name, &method)) {
        runtimeError("Undefined property '%s'.", name->chars);
        return false;
    }

    ObjBoundMethod *bound = newBoundMethod(peek(0), AS_CLOSURE(method));
    pop();
    push(OBJ_VAL(bound));
    return true;
}

static ObjUpvalue *captureUpvalue(Value *local) {
    ObjUpvalue *prevUpvalue = nullptr;
    ObjUpvalue *upvalue = vm.openUpvalues;
    while (upvalue != NULL && upvalue->location > local) {
        prevUpvalue = upvalue;
        upvalue = upvalue->next;
    }

    if (upvalue != NULL && upvalue->location == local) {
        return upvalue;
    }

    ObjUpvalue *createdUpvalue = newUpvalue(local);
    createdUpvalue->next = upvalue;

    if (prevUpvalue == NULL) {
        vm.openUpvalues = createdUpvalue;
    } else {
        prevUpvalue->next = createdUpvalue;
    }

    return createdUpvalue;
}

static void closeUpvalues(Value *last) {
    while (vm.openUpvalues != NULL && vm.openUpvalues->location >= last) {
        ObjUpvalue *upvalue = vm.openUpvalues;
        upvalue->closed = *upvalue->location;
        upvalue->location = &upvalue->closed;
        vm.openUpvalues = upvalue->next;
    }
}

static void defineMethod(ObjString *name) {
    Value method = peek(0);
    ObjClass *klass = AS_CLASS(peek(1));
    tableSet(&klass->methods, name, method);
    pop();
}

static bool isFalsey(Value value) {
    return IS_NULL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
    ObjString *b = AS_STRING(peek(0));
    ObjString *a = AS_STRING(peek(1));

    int length = a->length + b->length;
    char *chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString *result = takeString(chars, length);
    pop();
    pop();
    push(OBJ_VAL(result));
}

static InterpretResult run() {
    CallFrame *frame = &vm.frames[vm.frameCount - 1];
#define READ_BYTE() (*frame->ip++)
#define READ_SHORT() (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define READ_CONSTANT() (frame->closure->function->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define BINARY_OP(valueType, op) \
    do { \
      if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
        runtimeError("Operands must be numbers."); \
        return INTERPRET_RUNTIME_ERROR; \
      } \
      double b = AS_NUMBER(pop()); \
      double a = AS_NUMBER(pop()); \
      push(valueType(a op b)); \
    } while (false)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("          ");
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
      printf("[ ");
      printValue(*slot);
      printf(" ]");
    }
    printf("\n");
    disassembleInstruction(&frame->closure->function->chunk,
        (int)(frame->ip - frame->closure->function->chunk.code));
#endif

        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }

            case OP_NULL: push(NULL_VAL);
                break;
            case OP_TRUE: push(BOOL_VAL(true));
                break;
            case OP_FALSE: push(BOOL_VAL(false));
                break;
            case OP_POP: pop();
                break;
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(frame->slots[slot]);
                break;
            }

            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peek(0);
                break;
            }

            case OP_GET_GLOBAL: {
                ObjString *name = READ_STRING();
                Value value;
                
                // First check in globals
                if (tableGet(&vm.globals, name, &value)) {
                    push(value);
                    break;
                }
                
                // If not found in globals, check in all module exports using our helper function
                if (findExportedSymbol(name, &value)) {
                    push(value);
                    break;
                }
                
                // Not found anywhere
                runtimeError("Undefined variable '%s'.", name->chars);
                return INTERPRET_RUNTIME_ERROR;
            }

            case OP_DEFINE_GLOBAL: {
                ObjString *name = READ_STRING();
                Value value = peek(0);
                tableSet(&vm.globals, name, value);
                
                // When processing a module import, if we encounter a variable marked
                // with 'exp', add it to the module's exports table
                if (vm.isImporting && vm.isExporting && vm.currentModule != NULL) {
                    Module* module = findModule(vm.currentModule);
                    if (module != NULL) {
                        // Register this symbol as an export from the current module
                        tableSet(&module->exports, name, value);
                    }
                }
                
                pop();
                break;
            }

            case OP_SET_GLOBAL: {
                ObjString *name = READ_STRING();
                if (tableSet(&vm.globals, name, peek(0))) {
                    tableDelete(&vm.globals, name); // [delete]
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_GET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                push(*frame->closure->upvalues[slot]->location);
                break;
            }

            case OP_SET_UPVALUE: {
                uint8_t slot = READ_BYTE();
                *frame->closure->upvalues[slot]->location = peek(0);
                break;
            }

            case OP_GET_PROPERTY: {
                //> get-not-instance
                if (!IS_INSTANCE(peek(0))) {
                    runtimeError("Only instances have properties.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjInstance *instance = AS_INSTANCE(peek(0));
                ObjString *name = READ_STRING();

                Value value;
                if (tableGet(&instance->fields, name, &value)) {
                    pop(); // Instance.
                    push(value);
                    break;
                }

                if (!bindMethod(instance->klass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_SET_PROPERTY: {
                if (!IS_INSTANCE(peek(1))) {
                    runtimeError("Only instances have fields.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjInstance *instance = AS_INSTANCE(peek(1));
                tableSet(&instance->fields, READ_STRING(), peek(0));
                Value value = pop();
                pop();
                push(value);
                break;
            }

            case OP_GET_SUPER: {
                ObjString *name = READ_STRING();
                ObjClass *superclass = AS_CLASS(pop());

                if (!bindMethod(superclass, name)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }

            case OP_GREATER: BINARY_OP(BOOL_VAL, >);
                break;
            case OP_LESS: BINARY_OP(BOOL_VAL, <);
                break;

            case OP_ADD: {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate();
                } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                } else {
                    runtimeError("Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }

            case OP_SUBTRACT: BINARY_OP(NUMBER_VAL, -);
                break;
            case OP_MULTIPLY: BINARY_OP(NUMBER_VAL, *);
                break;
            case OP_DIVIDE: BINARY_OP(NUMBER_VAL, /);
                break;
            case OP_MOD:
                if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(modulo(a, b)));
                } else {
                    runtimeError("Operands must be two numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            case OP_POW:
                if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    int b = AS_NUMBER(pop());
                    float a = AS_NUMBER(pop());
                    push(NUMBER_VAL(power(a, b)));
                } else {
                    runtimeError("Operands must be two numbers.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;

            case OP_NOT:
                push(BOOL_VAL(isFalsey(pop())));
                break;

            case OP_NEGATE:
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;

            case OP_PRINT: {
                // Print the value at the top of the stack
                printValue(pop());
                printf("\n");
                break;
            }

            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                frame->ip += offset;
                break;
            }

            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();

                if (isFalsey(peek(0))) frame->ip += offset;
                break;
            }

            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                frame->ip -= offset;
                break;
            }

            case OP_CALL: {
                int argCount = READ_BYTE();
                if (!callValue(peek(argCount), argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }

                frame = &vm.frames[vm.frameCount - 1];
                break;
            }

            case OP_INVOKE: {
                ObjString *method = READ_STRING();
                int argCount = READ_BYTE();
                if (!invoke(method, argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }

            case OP_SUPER_INVOKE: {
                ObjString *method = READ_STRING();
                int argCount = READ_BYTE();
                ObjClass *superclass = AS_CLASS(pop());
                if (!invokeFromClass(superclass, method, argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }

            case OP_CLOSURE: {
                ObjFunction *function = AS_FUNCTION(READ_CONSTANT());
                ObjClosure *closure = newClosure(function);
                push(OBJ_VAL(closure));
                for (int i = 0; i < closure->upvalueCount; i++) {
                    uint8_t isLocal = READ_BYTE();
                    uint8_t index = READ_BYTE();
                    if (isLocal) {
                        closure->upvalues[i] = captureUpvalue(frame->slots + index);
                    } else {
                        closure->upvalues[i] = frame->closure->upvalues[index];
                    }
                }
                break;
            }

            case OP_CLOSE_UPVALUE:
                closeUpvalues(vm.stackTop - 1);
                pop();
                break;

            case OP_RETURN: {
                Value result = pop();
                closeUpvalues(frame->slots);
                vm.frameCount--;
                if (vm.frameCount == 0) {
                    pop();
                    return INTERPRET_OK;
                }

                vm.stackTop = frame->slots;
                push(result);
                frame = &vm.frames[vm.frameCount - 1];
                break;
            }

            case OP_TYPE: {
                break;
            }

            case OP_COLON: {
                break;
            }

            case OP_CLASS:
                push(OBJ_VAL(newClass(READ_STRING())));
                break;
            case OP_INHERIT: {
                Value superclass = peek(1);

                if (!IS_CLASS(superclass)) {
                    runtimeError("Superclass must be a class.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                ObjClass *subclass = AS_CLASS(peek(0));
                tableAddAll(&AS_CLASS(superclass)->methods, &subclass->methods);
                pop(); // Subclass.
                break;
            }

            case OP_METHOD:
                defineMethod(READ_STRING());
                break;
        }
    }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

void hack(bool b) {
    run();
    if (b) hack(false);
}

// Module system functions
Module* createModule(ObjString* name) {
    // Check if we need to grow the modules array
    if (vm.moduleRegistry.count + 1 > vm.moduleRegistry.capacity) {
        int oldCapacity = vm.moduleRegistry.capacity;
        vm.moduleRegistry.capacity = GROW_CAPACITY(oldCapacity);
        vm.moduleRegistry.modules = GROW_ARRAY(Module, vm.moduleRegistry.modules,
                                         oldCapacity, vm.moduleRegistry.capacity);
    }
    
    // Initialize the new module
    Module* module = &vm.moduleRegistry.modules[vm.moduleRegistry.count];
    module->name = name;
    initTable(&module->exports);
    
    // Add to the registry
    Value indexValue = NUMBER_VAL(vm.moduleRegistry.count);
    tableSet(&vm.moduleRegistry.moduleNames, name, indexValue);
    
    // Increment the count
    vm.moduleRegistry.count++;
    
    return module;
}

Module* findModule(ObjString* name) {
    Value indexValue;
    if (!tableGet(&vm.moduleRegistry.moduleNames, name, &indexValue)) {
        return NULL;
    }
    
    int index = (int)AS_NUMBER(indexValue);
    if (index < 0 || index >= vm.moduleRegistry.count) {
        return NULL;
    }
    
    return &vm.moduleRegistry.modules[index];
}

/**
 * Searches all loaded modules for an exported symbol.
 * If found, returns true and sets value to the exported value.
 * 
 * @param name The symbol name to look for
 * @param value Pointer to store the value if found
 * @return true if found, false otherwise
 */
bool findExportedSymbol(ObjString* name, Value* value) {
    // Look for the symbol in all registered modules' export tables
    for (int i = 0; i < vm.moduleRegistry.count; i++) {
        Module* module = &vm.moduleRegistry.modules[i];
        
        // Check if this module exports the symbol
        if (tableGet(&module->exports, name, value)) {
            return true;
        }
    }
    
    // Symbol not found in any module
    return false;
}

static char* readEntireFile(const char* path) {
    // Try to find the file in different locations
    FILE* file = NULL;
    char* resolvedPath = NULL;
    size_t resolvedPathLen = 0;
    
    // Get the current working directory for relative paths
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        // Try paths relative to bin directory since that's where the executable is
        
        // First try the exact path
        file = fopen(path, "rb");
        
        // If that failed, try looking in the current directory
        if (file == NULL) {
            resolvedPathLen = strlen(cwd) + strlen(path) + 2; // +2 for "/" and '\0'
            resolvedPath = (char*)malloc(resolvedPathLen);
            if (resolvedPath != NULL) {
                sprintf(resolvedPath, "%s/%s", cwd, path);
                file = fopen(resolvedPath, "rb");
                if (file == NULL) {
                    free(resolvedPath);
                    resolvedPath = NULL;
                }
            }
        }
        
        // If that failed, try looking in the bin directory
        if (file == NULL) {
            resolvedPathLen = strlen(cwd) + strlen("/bin/") + strlen(path) + 1; // +1 for '\0'
            resolvedPath = (char*)malloc(resolvedPathLen);
            if (resolvedPath != NULL) {
                sprintf(resolvedPath, "%s/bin/%s", cwd, path);
                file = fopen(resolvedPath, "rb");
                if (file == NULL) {
                    free(resolvedPath);
                    resolvedPath = NULL;
                }
            }
        }
        
        // If that failed, try adding .gec extension if not already present
        if (file == NULL) {
            // Check if the path already ends with .gec
            size_t pathLen = strlen(path);
            if (pathLen < 4 || strcmp(path + pathLen - 4, ".gec") != 0) {
                // Create a new path with .gec extension
                resolvedPathLen = strlen(cwd) + strlen("/bin/") + strlen(path) + 5; // +5 for ".gec" and '\0'
                resolvedPath = (char*)malloc(resolvedPathLen);
                if (resolvedPath != NULL) {
                    sprintf(resolvedPath, "%s/bin/%s.gec", cwd, path);
                    file = fopen(resolvedPath, "rb");
                    if (file == NULL) {
                        free(resolvedPath);
                        resolvedPath = NULL;
                    }
                }
            }
        }
    }
    
    // If still NULL, try looking in a standard directory (not implemented yet)
    // This would be a good place to search in a standard library path
    
    if (file == NULL) {
        fprintf(stderr, "Could not find file: '%s'\n", path);
        return NULL;
    }
    
    // Determine file size
    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);
    
    // printf("File found, size: %zu bytes\n", fileSize);
    
    // Allocate buffer to hold the entire file
    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }
    
    // Read the file into the buffer
    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Error reading file: Read only %zu of %zu bytes\n", bytesRead, fileSize);
        free(buffer);
        fclose(file);
        return NULL;
    }
    
    // Null terminate the buffer
    buffer[bytesRead] = '\0';
    
    fclose(file);
    return buffer;
}

static InterpretResult interpretModule(const char* source, ObjString* moduleName, bool isInclude) {
    // Create a new module entry or find existing one
    Module* module = findModule(moduleName);
    if (module == NULL) {
        module = createModule(moduleName);
    }
    
    // Compile the module, passing the module name
    ObjFunction* function = compile(source, moduleName);
    if (function == NULL) {
        return INTERPRET_COMPILE_ERROR;
    }
    
    // Always execute the module code to process declarations
    push(OBJ_VAL(function));
    ObjClosure* closure = newClosure(function);
    pop();
    push(OBJ_VAL(closure));
    
    if (!call(closure, 0)) {
        return INTERPRET_RUNTIME_ERROR;
    }
    
    return run();
}

InterpretResult interpret(const char *source) {
    // Use main module for direct execution (non-include)
    ObjString* mainModuleName = copyString("main", 4);
    
    // Ensure we're not in importing mode for the main script
    bool wasImporting = vm.isImporting;
    vm.isImporting = false;
    
    // Execute the main script
    InterpretResult result = interpretModule(source, mainModuleName, false);
    
    // Restore previous importing flag (should rarely be needed)
    vm.isImporting = wasImporting;
    
    return result;  
}

/**
 * Processes an include statement by compiling and executing a module file,
 * and making its exported symbols (marked with 'exp') available to the calling code.
 *
 * @param path The file path to include
 * @return InterpretResult indicating success or failure
 */
InterpretResult interpretInclude(const char* path) {
    // Convert the path to a module name 
    ObjString* moduleName = copyString(path, (int)strlen(path));
    
    // Check if this module has already been loaded - if so, we can reuse it
    Module* existingModule = findModule(moduleName);
    if (existingModule != NULL) {
        return INTERPRET_OK;
    }
    
    // Read the module source code file
    char* source = readEntireFile(path);
    if (source == NULL) {
        return INTERPRET_RUNTIME_ERROR;
    }
    
    // Create a new module entry in the registry
    Module* module = createModule(moduleName);
    
    // Save the current VM context before switching to the module context
    ObjString* prevModule = vm.currentModule;
    
    // Track which module we're compiling - this helps with export registration
    vm.currentModule = moduleName;
    
    // Compile the module code with the module name for context
    ObjFunction* function = compile(source, moduleName);
    if (function == NULL) {
        // Compilation error - restore VM state and return
        vm.currentModule = prevModule;
        free(source);
        return INTERPRET_COMPILE_ERROR;
    }
    
    // Execute the module to process all declarations
    // This will register any exported symbols (marked with 'exp')
    push(OBJ_VAL(function));
    ObjClosure* closure = newClosure(function);
    pop();
    push(OBJ_VAL(closure));
    
    if (!call(closure, 0)) {
        // Runtime error during module execution
        vm.currentModule = prevModule;
        free(source);
        return INTERPRET_RUNTIME_ERROR;
    }
    
    // Run the module code
    InterpretResult result = run();
    
    // After module execution, scan globals for symbols marked with 'exp'
    // and add them to the module's exports table
    for (int i = 0; i < vm.globals.capacity; i++) {
        Entry* entry = &vm.globals.entries[i];
        if (entry->key != NULL && entry->key->chars != NULL) {
            // Skip builtin functions like 'clock' - only add user-defined exports
            if (strcmp(entry->key->chars, "clock") != 0) {
                // Add to module exports table
                tableSet(&module->exports, entry->key, entry->value);
            }
        }
    }
    
    // Restore the previous module context
    vm.currentModule = prevModule;
    
    // Free the source buffer
    free(source);
    
    return result;
}

//< interpret
