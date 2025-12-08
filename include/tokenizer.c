#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

int tokenize(const char *line, char *tokens[], int max_tokens) {
    int i = 0;      // index in input string
    int t = 0;      // number of tokens

    while (line[i] != '\0' && t < max_tokens - 1) {

        if (isspace((unsigned char)line[i])) {
            i++;
            continue;
        }

        if (line[i] == '"' || line[i] == '\'') {
            char quote = line[i];
            int start, len;
            char *word;

            i++;               
            start = i;


            while (line[i] != '\0' && line[i] != quote) {
                i++;
            }


            if(line[i]!= quote){
                
                printf("tokenizer Error: unmatched quote\n");

                break;
            }

            len = i - start;
            word = malloc(len + 1);
            if (!word) break;

            memcpy(word, &line[start], len);
            word[len] = '\0';

            tokens[t++] = word;

            if (line[i] == quote) {
                i++;            
            }
            continue;
        }

        if (line[i] == '&' || line[i] == '|' || line[i] == '<' || line[i] == '>') {

            char op[3];
            int j = 0;

            op[j++] = line[i];

            // check for &&, ||, <<, >>
            if (line[i + 1] == line[i]) {
                op[j++] = line[i + 1];
                i += 2;
            } else {
                i += 1;
            }

            op[j] = '\0';

            tokens[t] = strdup(op);
            if (!tokens[t]) break;
            t++;

            continue;
        }

        // 4. Normal word
        else {
            int start = i;
            int len;
            char *word;

            // read until space, operator, or quote
            while (line[i] != '\0' && !isspace((unsigned char)line[i]) && line[i] != '&' && line[i] != '|' && line[i] != '<' && line[i] != '>' && line[i] != '"' && line[i] != '\'') {
                i++;
            }

            len = i - start;
            if (len == 0) {
                continue;
            }

            word = malloc(len + 1);
            if (!word) break;

            memcpy(word, &line[start], len);
            word[len] = '\0';

            tokens[t++] = word;
            continue;
        }
    }

    tokens[t] = NULL;   // for execvp
    return t;
}

void free_tokens(char *tokens[], int count) {
    for (int i = 0; i < count; i++) {
        free(tokens[i]);
    }
}
