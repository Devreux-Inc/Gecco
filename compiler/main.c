//
// Created by wylan on 12/19/24.
//

#include <stdio.h>
#include <stdlib.h>

#include "geccovm/vm.h"
#include "command/command_defs.h"
#include "command/command_handler.h"
#include "err/status.h"
#include "repl/repl.h"

/**
 * Gets the extension of a passed in file.
 * @param filename the fully qualified file name.
 * @return the extension.
 */
const char *get_file_extension(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if (!dot || dot == filename) return "";
    return dot + 1;
}

/**
 * Checks for a .gec or .gc file extension.
 * GEC of GC are the only valid file types
 * for Gecco to execute.
 *
 * @param extension Either .gec or .gc
 * @return true if the file extension is valid.
 */
bool file_extension_is_valid(const char *extension) {
    if (strcmp(extension, "gec") == 0 || strcmp(extension, "gc") == 0) return true;
    return false;
}

/**
 * Allocates a file into the memory of the computer.
 * @param path The path of the file.
 * @return a character array.
 */
static char *readFile(const char *path) {
    FILE *file = fopen(path, "rb");

    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(exit_status(FILE_NOT_FOUND));
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char *buffer = malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(exit_status(OUT_OF_MEMORY));
    }

    size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
    if (bytesRead < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(exit_status(FILE_NOT_READABLE));
    }

    buffer[bytesRead] = '\0';

    fclose(file);
    return buffer;
}

/**
 * Executes a .gec or .gc file.
 * @param path The file path of the runnable.
 */
static void runFile(const char *path) {
    char *source = readFile(path);
    InterpretResult result = interpret(source);
    free(source); // [owner]

    if (result == INTERPRET_COMPILE_ERROR) {
        freeVM();
        exit(exit_status(COMPILER_ERROR));
    }
    if (result == INTERPRET_RUNTIME_ERROR) {
        freeVM();
        exit(exit_status(RUNTIME_ERROR));
    }
}

/**
 * The main entry point for Gecco. This starts the program.
 * @param argc Arguments length.
 * @param argv Each appended argument.
 * @return EXIT_SUCCESS if the program was a success.
 */
int main(const int argc, const char *argv[]) {
    initVM();

    if (argc >= 2 && argc < 4) {
        if (qualified_command(argv[1])) {

            freeVM();
            return EXIT_SUCCESS;
        }

        if (strcmp(argv[1], "--run") == 0) {
            const char *file_type = get_file_extension(argv[2]);
            if (file_extension_is_valid(file_type)) {
                runFile(argv[2]);

                freeVM();
                return exit_status(EXIT_SUCCESS);
            }
            printf("File type not recognized '%s'.\n", file_type);

            freeVM();
            return exit_status(EXIT_FAILURE);
        }

        unknown_command(argv[1]);

        freeVM();
        return exit_status(EXIT_FAILURE_MAJOR);
    } else {
        //unknown_command(argv[2]);
        repl();

        freeVM();
        return exit_status(EXIT_FAILURE_MINOR);
    }

}
