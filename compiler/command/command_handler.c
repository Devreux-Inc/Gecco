//
// Created by wylan on 12/26/24.
//

#include <string.h>
#include "command_handler.h"

#include "command_defs.h"

bool qualified_command(const char *command) {
    if (strcmp(command, "--help") == 0 || strcmp(command, "?") == 0) {
        list_commands();
        return true;
    }

    if (strcmp(command, "--version") == 0 || strcmp(command, "--v") == 0) {
        print_version();
        return true;
    }

    if (strcmp(command, "--credits") == 0) {
        print_credits();
        return true;
    }

    if (strcmp(command, "--run") == 0) {
        return false;
    }

    return false;
}
