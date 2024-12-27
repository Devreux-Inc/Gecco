//
// Created by wylan on 12/26/24.
//

#include "status.h"

#include <stdio.h>
#include <stdlib.h>

#include "../formatting/ansi_colors.h"

/**
 * Prints an exit code message on exit.
 * @param code the exit code.
 * @param msg the exit message.
 */
static void print_with_code(int code, const char *msg) {
    printf("Exit with status code: %d. %s\n" RESET, code, msg);
}

/**
 * Switches on an exit status to print a predefined output to the console.
 * @see err/status.h for definable statuses.
 *
 * @param status The exit status.
 * @return The exit status to be used.
 */
int exit_status(const int status) {
    switch (status) {
        case EXIT_SUCCESS: print_with_code(EXIT_SUCCESS, GREEN BOLD "OK."); break;
        case EXIT_FAILURE: print_with_code(EXIT_FAILURE, RED BOLD "FAILED."); break;
        case EXIT_FAILURE_MAJOR: print_with_code(EXIT_FAILURE_MAJOR, RED BOLD "FAILED_MAJOR."); break;
        case EXIT_FAILURE_MINOR: print_with_code(EXIT_FAILURE_MINOR, RED BOLD "FAILED_MINOR"); break;
        case COMPILER_ERROR: print_with_code(COMPILER_ERROR, RED BOLD "Compiler error."); break;
        case COMPILER_WARNING: print_with_code(COMPILER_WARNING, RED BOLD"Compiler warning."); break;
        case COMPILER_INFO: print_with_code(COMPILER_INFO, RED BOLD "Compiler warning."); break;
        case RUNTIME_ERROR: print_with_code(RUNTIME_ERROR, RED BOLD "Runtime error."); break;
        case FILE_NOT_FOUND: print_with_code(FILE_NOT_FOUND, RED BOLD "File not found."); break;
        case FILE_NOT_READABLE: print_with_code(FILE_NOT_READABLE, RED BOLD "File not readable."); break;
        case FILE_NOT_WRITABLE: print_with_code(FILE_NOT_WRITABLE, RED BOLD "File not writable."); break;
        case INTERPRETER_ERROR: print_with_code(INTERPRETER_ERROR, RED BOLD "Interpreter error."); break;
        case OUT_OF_MEMORY: print_with_code(OUT_OF_MEMORY, RED BOLD "Out of memory."); break;
        default: print_with_code(EXIT_FAILURE, RED "Unknown error."); break;
    }

    return status;
}

/**
 * Pushes without modifying the exit status.
 * @param status The exit status.
 * @return The exit status.
 */
int exit_without_status(const int status) {
    return status;
}
