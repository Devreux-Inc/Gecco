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

#define VERSION "0.0.1"

void list_commands();

#endif //COMMAND_DEFS_H
