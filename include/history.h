#ifndef HISTORY_H
#define HISTORY_H

#define HISTORY_MAX 25


// void loadHistory(char *path);
int addToHistory(char *cmd);
void printHistory();
// void saveHistory();
void freeHistory();
void clearHistory();
void initHistory();

#endif
