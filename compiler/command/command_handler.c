//
// Created by wylan on 12/26/24.
//

#include <string.h>
#include <stdlib.h>  // For system() function
#include <stdbool.h>  // For bool type

#include "../common.h"
#include "command_handler.h"
#include "command_defs.h"
#include "../repl/repl.h"

static void execute(const char *command) {
    system(command);
}

static void clear() {
    if (isWindows()) execute("cls");
    else execute("clear");
}

bool qualified_command(const char *command) {
    if (strcmp(command, "--help") == 0 || strcmp(command, "?") == 0) {
        clear();
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

    if (strcmp(command, "--repl") == 0) {
        clear();
        repl();
        return false;
    }

    if (strcmp(command, "--run") == 0) {
        return false;
    }

    return false;
}
