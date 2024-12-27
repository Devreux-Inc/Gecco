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

#define GECCO_VERSION "0.1.0rc1"
#define GECCO_VM_VERSION "0.0.1"
#define GECCO_REPL_VERSION "1.0.0rc1"

void list_commands();
void unknown_command(const char *command);
void print_version();
void print_credits();

#endif //COMMAND_DEFS_H
