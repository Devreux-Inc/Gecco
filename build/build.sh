#!/bin/bash

# Move to the project root directory
cd ..

# Create output directory if it doesn't exist
mkdir -p bin

# Compile all source files directly
clang \
  -o bin/Gecco \
  compiler/chunk/chunk.c \
  compiler/common.c \
  compiler/compiler/compiler.c \
  compiler/debug/debug.c \
  compiler/main.c \
  compiler/memory/memory.c \
  compiler/object.c \
  compiler/scanner.c \
  compiler/table.c \
  compiler/value.c \
  compiler/geccovm/vm.c \
  compiler/command/command_defs.c \
  compiler/command/command_handler.c \
  compiler/err/status.c \
  compiler/repl/repl.c \
  -I. \
  -Wall -Wextra -std=c11 -O3 -DDEBUG

echo "Build complete. Binary is in bin/Gecco"
