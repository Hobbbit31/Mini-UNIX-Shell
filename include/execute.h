#ifndef EXECUTE_H
#define EXECUTE_H

#include "parser.h"

// Execute a parsed command with redirection.
// Returns 0 on success, -1 on fork/exec/open errors.
int execute_cmd(command_t *cmd);
int run_pipeline(command_t *left, command_t *right);

#endif 
