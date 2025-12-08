#ifndef PARSER_H
#define PARSER_H

typedef struct {
    char **argv;    // NULL-terminated argument list
    char *infile;   
    char *outfile;
    int append;
    int background;   
} command_t;

// Initialize fields to NULL
void init_cmd(command_t *cmd);

// Free everything inside command_t
void free_memory_cmd(command_t *cmd);

// Parse tokens[] (size nt) into command_t.
// Returns 0 on success, -1 on syntax error (prints message to stderr).
int parse_tokens(char *tokens[], int nt, command_t *cmd);

int parse_line_to_check_pipeline(char *tokens[], int n, command_t *left_cmd, command_t *right_cmd, int *is_pipeline);
#endif 