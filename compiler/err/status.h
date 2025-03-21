//
// Created by wylan on 12/21/24.
//

#ifndef EXIT_H
#define EXIT_H

#define EXIT_SUCCESS        0
#define EXIT_FAILURE        1
#define EXIT_FAILURE_MAJOR  2
#define EXIT_FAILURE_MINOR  3

#define COMPILER_ERROR      65
#define COMPILER_WARNING    66
#define COMPILER_INFO       67

#define RUNTIME_ERROR       75

#define FILE_NOT_FOUND      84
#define FILE_NOT_READABLE   87
#define FILE_NOT_WRITABLE   88

#define INTERPRETER_ERROR   90

#define OUT_OF_MEMORY       100

extern int exit_status(int status);
extern int exit_without_status(int status);

#endif //EXIT_H
