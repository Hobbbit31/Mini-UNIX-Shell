#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int tokenize(const char *line, char *tokens[]) {
    int i = 0;      // position in line
    int t = 0;      // number of tokens

    while (line[i] != '\0') {

        // Skip spaces
        while (line[i] != '\0' && isspace((unsigned char)line[i])) {
            i++;
        }
        if (line[i] == '\0') break;

        // ---------------------------
        // 1. Quoted strings
        // ---------------------------
        if (line[i] == '"' || line[i] == '\'') {
            char quote = line[i];
            int start, len;
            char *word;

            i++;            // skip opening quote
            start = i;

            while (line[i] != '\0' && line[i] != quote) {
                i++;
            }

            len = i - start;
            word = (char *)malloc(len + 1);
            if (!word) return t; // out of memory, return what we have

            memcpy(word, &line[start], len);
            word[len] = '\0';

            tokens[t++] = word;

            if (line[i] == quote) i++;  // skip closing quote
            continue;
        }

        // ---------------------------
        // 2. Operators (&, |, <, >)
        //    Support: &&, ||, >>, <<, and single ones
        // ---------------------------
        if (line[i] == '&' || line[i] == '|' || line[i] == '<' || line[i] == '>') {

            char op[3] = {0, 0, 0}; // max 2 chars + '\0'
            op[0] = line[i];

            // Check two-character operators
            if (line[i + 1] != '\0') {
                if ((line[i] == '&' && line[i+1] == '&') || (line[i] == '|' && line[i+1] == '|') || (line[i] == '<' && line[i+1] == '<') || (line[i] == '>' && line[i+1] == '>')) {
                    op[1] = line[i+1];
                    i += 2;
                } else {
                    i += 1;
                }
            } else {
                i += 1;
            }

            tokens[t++] = strdup(op);
            // if (t >= max) break;
            continue;
        }

        // ---------------------------
        // 3. Normal word
        // ---------------------------
        {
            int start = i;
            int len;
            char *word;

            while (line[i] != '\0' &&  !isspace((unsigned char)line[i]) && line[i] != '&' && line[i] != '|' && line[i] != '<' && line[i] != '>') {
                i++;
            }

            len = i - start;
            if (len <= 0) continue;

            word = (char *)malloc(len + 1);
            if (!word) return t;

            memcpy(word, &line[start], len);
            word[len] = '\0';

            tokens[t++] = word;
        }
    }

    return t;
}

