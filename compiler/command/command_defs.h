//
// Created by wylan on 12/20/24.
//

#ifndef COMMAND_DEFS_H
#define COMMAND_DEFS_H
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char* name;
    char* usage;
    char* helper;

} Command;

typedef struct {
    char* example;
} Example;

void list_commands();
void unknown_command(const char *command);
void print_version();
void print_credits();

#endif //COMMAND_DEFS_H
