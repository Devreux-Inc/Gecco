//
// Created by wylan on 12/20/24.
//

#include "command_defs.h"

#include <string.h>

#include "../common.h"
#include "../formatting/ansi_colors.h"
#include "../version/version.h"

Command commands[] = {
    {"version", "--version", "| Lists the current Gecco version."},
    {"help", "--help", "   | Lists all available commands for Gecco."},
    {"run", "--run", "    | Flag before a file and then include <file path>."},
    {"repl", "--repl", "   | Runs the command line repl."},
    {"credits", "--credits", "| Lists contributors to Gecco."},
    {"verbose", "--verbose", "| Verbose mode."}
};

Example examples[] = {
    {
        BOLD "gecco" RESET CYAN " --run" RESET BOLD MAGENTA " main.gec" RESET CYAN " --verbose" RESET
        " \n       | Runs a file called" BOLD MAGENTA " main.gec" RESET " in verbose mode."
    },
    {
        BOLD "gecco" RESET CYAN " --run" RESET BOLD MAGENTA " main.gec \n " RESET "      | Runs a file called" BOLD
        MAGENTA " main.gec" RESET "."
    }
};

static void example_command() {
    if (isWindows()) printf(BOLD "\n              Example Command(s)   \n" RESET);
    else printf(BOLD "\n          📖   Example Command(s)  📖\n" RESET);
    for (int i = 0; i < sizeof(examples) / sizeof(examples[0]); i++) {
        const char *example = examples[i].example;

        if (isWindows()) printf("%s\n", example);
        else printf("▫️ %s\n", example);
    }
}

static void helper() {
    if (isWindows()) printf(BOLD "\n             Why Am I Seeing This?   " RESET "\n");
    else printf(BOLD "\n          ⚠️ Why Am I Seeing This? ⚠️" RESET "\n");
    printf(
        "You either inputted an invalid command or used the help command.\nThe above list provides all possible commands "
        "that can follow\n the" BOLD " gecco " RESET "main command. Need more help? Go to https://gecco.dev");
    printf("\n");
}

void list_commands() {
    if (isWindows()) printf(BOLD "\n%2s\n" RESET, "              Available Commands              ");
    else printf(BOLD "\n%2s\n" RESET, "           🔎  Available Commands  🔎           ");
    for (int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        const Command command = commands[i];
        printf(GREEN BOLD "%s:" RESET BOLD " %10s" RESET CYAN " %10s\n" RESET, isWindows() ? command.name : strcat("▫️ ", command.name), command.usage,
               command.helper);
    }

    helper();
    example_command();
}

void print_version() {
    printf("Gecco Version: %s\n", GECCO_VERSION);
    printf("GeccoVM Version: %s\n", GECCO_VM_VERSION);
    printf("Gecco REPL Version: %s\n", GECCO_REPL_VERSION);
}

void print_credits() {
    if (isWindows()) printf("%s\n", "Copyright 2024 The Gecco Contributors.");
    else printf("%s\n", "2024 © The Gecco Contributors.");
}

void unknown_command(const char *command) {
    printf("Unknown command: %s\n", command);
    printf("Use flag '%s' for available commands.\n", "--help");
}
