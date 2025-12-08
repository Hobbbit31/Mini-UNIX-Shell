#include "history.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define history_size 25
char path[10000];
char *line[history_size];
int hist = 0;

void initHistory() {
    char cwd[4000];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        snprintf(path, sizeof(path), "%s/include/log.txt", cwd);
    } else {
        perror("getcwd() error");
    }
    FILE *file = fopen(path, "r");
    if (!file) {
        return;
    }
    fclose(file);
}

void freeHistory() {
    for (int i = 0; i < history_size; i++) {
        if (line[i]) {
            free(line[i]);
            line[i] = NULL;
        }
    }
    hist = 0;
}

void clearHistory(void) {
    
    FILE *file = fopen(path, "w");
    if (file) {
        fclose(file);
    }else{
        printf("Failed to clear history file");
    }
}

void printHistory() {
    FILE *file = fopen(path, "r");
    if (!file) {
        printf("No history available.\n");
        return;
    }

    char buffer[1024];
    char last[history_size][1024];   // store last 25 lines
    int count = 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        buffer[strcspn(buffer, "\n")] = '\0';  // remove newline

        if (count < history_size) {
            strcpy(last[count++], buffer);
        } else {
            // shift left to remove oldest
            for (int i = 1; i < history_size; i++)
                strcpy(last[i-1], last[i]);

            strcpy(last[history_size-1], buffer);
        }
    }

    fclose(file);

    if (count == 0) {
        printf("No history available.\n");
        return;
    }

    for (int i = 0; i < count; i++)
        printf("%d: %s\n", i + 1, last[i]);
}

int addToHistory( char *cmd) {
    if(!cmd || cmd[0] == '\0'){ 
        return -1; 
    }
    FILE *file = fopen(path, "r");

    if(file){
        char buffer[1024];
        char last[1024] ="";
        
        while(fgets(buffer, sizeof(buffer), file)){
            strcpy(last, buffer);
        }
        fclose(file);
        if(strcmp(last, cmd) == 0){
            return 0; 
        }
    }
    file = fopen(path, "a");
    if(!file){
        printf("Failed to open history file for appending.\n");
        return -1;
    }
    if(fprintf(file, "%s\n", cmd) < 0){
        printf("Failed to write to history file.\n");
        fclose(file);
        return -1;
    }
    fclose(file);
    return 0;
}