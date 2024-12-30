//
// Created by wylan on 12/27/24.
//

#include "repl.h"

#include <stdio.h>

#include "../geccovm/vm.h"
#include "../formatting/ansi_colors.h"

static void print_head() {
    if (isWindows()) printf(BOLD "                      Gecco REPL  \n" RESET);
    else printf(BOLD "                    🔁 Gecco REPL 🔁\n" RESET);
    printf("This is the command line REPL (read-eval-print-loop) for" BOLD " Gecco" RESET ". \n"
        "You can run any code in the terminal and it will run as if \nit is part of a" BOLD " .gec" RESET " file."
        " All code is ran through the interpreter.\n");
}

/**
 * Run -> Evaluate -> Print -> Loop.
 */
void repl() {
    char line[1024];

    print_head();

    for (;;) {
        printf(BOLD "\n> " RESET);

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }

        interpret(line);
    }
}
