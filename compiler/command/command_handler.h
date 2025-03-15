//
// Created by wylan on 12/26/24.
//

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <stdbool.h>

/**
 * Checks for a qualified command and executes it to any possible command.
 * @param command char array of the command.
 * @return true if the command can be self executed without need of follow up.
 */
extern bool qualified_command(const char *command);

#endif //COMMAND_HANDLER_H
