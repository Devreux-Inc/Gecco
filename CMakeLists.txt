cmake_minimum_required(VERSION 3.30)
project(Gecco C)

set(CMAKE_C_STANDARD 23)

add_executable(Gecco
        compiler/chunk.c
        compiler/chunk.h
        compiler/common.h
        compiler/compiler.c
        compiler/compiler.h
        compiler/debug.c
        compiler/debug.h
        compiler/main.c
        compiler/memory.c
        compiler/memory.h
        compiler/object.c
        compiler/object.h
        compiler/scanner.c
        compiler/scanner.h
        compiler/table.c
        compiler/table.h
        compiler/value.c
        compiler/value.h
        compiler/vm.c
        compiler/vm.h
)
