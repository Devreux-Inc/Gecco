//
// Created by wylan on 12/19/24.
//

#ifndef object_h
#define object_h

#include "common.h"
#include "chunk.h"
#include "table.h"
#include "value.h"

#define OBJ_TYPE(value)        (AS_OBJ(value)->type)
#define IS_BOUND_METHOD(value) isObjType(value, OBJ_BOUND_METHOD)
#define IS_CLASS(value)        isObjType(value, OBJ_CLASS)
#define IS_CLOSURE(value)      isObjType(value, OBJ_CLOSURE)
#define IS_FUNCTION(value)     isObjType(value, OBJ_FUNCTION)
#define IS_INSTANCE(value)     isObjType(value, OBJ_INSTANCE)
#define IS_NATIVE(value)       isObjType(value, OBJ_NATIVE)
#define IS_STRING(value)       isObjType(value, OBJ_STRING)

#define AS_BOUND_METHOD(value) ((ObjBoundMethod*)AS_OBJ(value))
#define AS_CLASS(value)        ((ObjClass*)AS_OBJ(value))
#define AS_CLOSURE(value)      ((ObjClosure*)AS_OBJ(value))
#define AS_FUNCTION(value)     ((ObjFunction*)AS_OBJ(value))
#define AS_INSTANCE(value)     ((ObjInstance*)AS_OBJ(value))
#define AS_NATIVE(value) (((ObjNative*)AS_OBJ(value))->function)
#define AS_STRING(value)       ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)      (((ObjString*)AS_OBJ(value))->chars)

typedef enum {
    //> Methods and Initializers obj-type-bound-method
    OBJ_BOUND_METHOD,
    //< Methods and Initializers obj-type-bound-method
    //> Classes and Instances obj-type-class
    OBJ_CLASS,
    //< Classes and Instances obj-type-class
    //> Closures obj-type-closure
    OBJ_CLOSURE,
    //< Closures obj-type-closure
    //> Calls and Functions obj-type-function
    OBJ_FUNCTION,
    //< Calls and Functions obj-type-function
    //> Classes and Instances obj-type-instance
    OBJ_INSTANCE,
    //< Classes and Instances obj-type-instance
    //> Calls and Functions obj-type-native
    OBJ_NATIVE,
    //< Calls and Functions obj-type-native
    OBJ_STRING,
    //> Closures obj-type-upvalue
    OBJ_UPVALUE
    //< Closures obj-type-upvalue
} ObjType;

//< obj-type

struct Obj {
    ObjType type;
    //> Garbage Collection is-marked-field
    bool isMarked;
    //< Garbage Collection is-marked-field
    //> next-field
    struct Obj *next;
    //< next-field
};

//> Calls and Functions obj-function

typedef struct {
    Obj obj;
    int arity;
    //> Closures upvalue-count
    int upvalueCount;
    //< Closures upvalue-count
    Chunk chunk;
    ObjString *name;
} ObjFunction;

//< Calls and Functions obj-function
//> Calls and Functions obj-native

typedef Value (*NativeFn)(int argCount, Value *args);

typedef struct {
    Obj obj;
    NativeFn function;
} ObjNative;

//< Calls and Functions obj-native
//> obj-string

struct ObjString {
    Obj obj;
    int length;
    char *chars;
    //> Hash Tables obj-string-hash
    uint32_t hash;
    //< Hash Tables obj-string-hash
};

//< obj-string
//> Closures obj-upvalue
typedef struct ObjUpvalue {
    Obj obj;
    Value *location;
    //> closed-field
    Value closed;
    //< closed-field
    //> next-field
    struct ObjUpvalue *next;
    //< next-field
} ObjUpvalue;

//< Closures obj-upvalue
//> Closures obj-closure
typedef struct {
    Obj obj;
    ObjFunction *function;
    //> upvalue-fields
    ObjUpvalue **upvalues;
    int upvalueCount;
    //< upvalue-fields
} ObjClosure;

//< Closures obj-closure
//> Classes and Instances obj-class

typedef struct {
    Obj obj;
    ObjString *name;
    //> Methods and Initializers class-methods
    Table methods;
    //< Methods and Initializers class-methods
} ObjClass;

//< Classes and Instances obj-class
//> Classes and Instances obj-instance

typedef struct {
    Obj obj;
    ObjClass *klass;
    Table fields; // [fields]
} ObjInstance;

//< Classes and Instances obj-instance

//> Methods and Initializers obj-bound-method
typedef struct {
    Obj obj;
    Value receiver;
    ObjClosure *method;
} ObjBoundMethod;

//< Methods and Initializers obj-bound-method
//> Methods and Initializers new-bound-method-h
ObjBoundMethod *newBoundMethod(Value receiver,
                               ObjClosure *method);

//< Methods and Initializers new-bound-method-h
//> Classes and Instances new-class-h
ObjClass *newClass(ObjString *name);

//< Classes and Instances new-class-h
//> Closures new-closure-h
ObjClosure *newClosure(ObjFunction *function);

//< Closures new-closure-h
//> Calls and Functions new-function-h
ObjFunction *newFunction();

//< Calls and Functions new-function-h
//> Classes and Instances new-instance-h
ObjInstance *newInstance(ObjClass *klass);

//< Classes and Instances new-instance-h
//> Calls and Functions new-native-h
ObjNative *newNative(NativeFn function);

//< Calls and Functions new-native-h
//> take-string-h
ObjString *takeString(char *chars, int length);

//< take-string-h
//> copy-string-h
ObjString *copyString(const char *chars, int length);

//> Closures new-upvalue-h
ObjUpvalue *newUpvalue(Value * slot);
//< Closures new-upvalue-h
//> print-object-h
void printObject(Value value);

//< print-object-h

//< copy-string-h
//> is-obj-type
static inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

//< is-obj-type

#endif //object_h
