//
// Created by wylan on 12/20/24.
//

#include "command_defs.h"
#include "../formatting/ansi_colors.h"

Command commands[] = {
    {"version", "--version", "| Lists the current Gecco version."},
    {"help", "--help", "   | Lists all available commands for Gecco."},
    {"run", "--run", "    | Flag before a file and then include <file path>."},
    {"credits", "--credits", "| Lists contributors to Gecco."},
    {"verbose", "--verbose", "| Verbose mode."}
};

char* examples[] = {
    {BOLD "gecco" RESET CYAN " --run" RESET BOLD MAGENTA " main.gec" RESET CYAN " --verbose" RESET " \n       | Runs a file called" BOLD MAGENTA " main.gec" RESET " in verbose mode."},
    {BOLD "gecco" RESET CYAN " --run" RESET BOLD MAGENTA " main.gec \n " RESET "      | Runs a file called" BOLD MAGENTA " main.gec" RESET "."}
};

static void example_command() {
    printf(BOLD "\n          📖   Example Command(s)  📖\n" RESET);
    for (int i = 0; i < sizeof(examples) / sizeof(examples[0]); i++) {
        const char* example = examples[i];
        printf("▫️ %s\n", example);
    }
}

static void helper() {
    printf(BOLD "\n          ⚠️ Why Am I Seeing This? ⚠️" RESET "\n");
    printf("You either inputted an invalid command or used the help command.\nThe above list provides all possible commands "
           "that can follow \nthe" BOLD " gecco " RESET "main command. Need more help? Go to https://gecco.dev");
    printf("\n");
}

void list_commands() {
    printf( BOLD "\n%2s\n" RESET, "           🔎  Available Commands  🔎           ");
    for (int i = 0; i < sizeof(commands) / sizeof(commands[0]); i++) {
        const Command command = commands[i];
        printf(GREEN BOLD "▫️ %s:" RESET BOLD " %10s" RESET BLUE " %10s\n" RESET, command.name, command.usage, command.helper);
    }

    helper();
    example_command();
}
