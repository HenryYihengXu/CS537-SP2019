#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#define _GNU_SOURCE

typedef struct tokenList {
    char *word;
    struct tokenList *next;
} Token;

typedef struct historyList {
    char *command;
    struct historyList *next;
} History;

typedef struct pathList {
    char *content;
    struct pathList *next;
} Path;

const char error_message[30] = "An error has occurred\n";
History *commandHistory;
Path *allPath;

int getStream(int argc, char *argv[], FILE *fp);
void getOperator(char* line, char* operator);
int parseList(char *line, Token *head);
void parseArgv(Token *commandList, char *argv[]);
void addHistory(History *head, char* line);
void allHistory(History *current);
void limitHistory(History *current, double n);
void history(int argc, char *argv[]);
void addPath(char* path);
void path(int argc, char* argv[]);
void cd(int argc, char* argv[]);

int main(int argc, char *argv[]) {

    FILE *fp;
    int mode;
    //int mode = getStream(argc, argv, stream);

    if (argc == 1) {
        fp = stdin;
        mode = 1;
    } else if (argc == 2) {
        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        mode = 0;
    } else {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }

    commandHistory = (History*)malloc(sizeof(History));
    commandHistory ->next = NULL;
    commandHistory ->command = NULL;

    allPath = (Path*)malloc(sizeof(Path));
    allPath->next = (Path*)malloc(sizeof(Path));
    allPath->next->next = NULL;
    allPath->next->content = "/bin";

    if (mode == 1) {
        printf("wish> ");
    }

    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, fp) != -1) {

        line[strlen(line) - 1] = '\0';
        if (line == NULL) {
            if (mode == 1) {
                printf("wish> ");
            }
            continue;
        }
        char* operator = "none";
        getOperator(line, operator);
        if (strcmp(operator, "error") == 0) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            addHistory(commandHistory, line);
            if (mode == 1) {
                printf("wish> ");
            }
            continue;
        }
        
        Token *commandList = (Token*)malloc(sizeof(Token));
        int size = parseList(line, commandList);
        if (size == 0) {
            if (mode == 1) {
                printf("wish> ");
            }
            continue;
        }
        addHistory(commandHistory, line);
        char* commandArgv[size + 1];
        parseArgv(commandList, commandArgv);
        free(commandList);

        if (strcmp("exit", commandArgv[0]) == 0){
            if (size != 1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }
            exit(0);
        } else if (strcmp("cd", commandArgv[0]) == 0){
            cd(size, commandArgv);
            if (mode == 1) {
                printf("wish> ");
            }
            continue;
        } else if (strcmp("history", commandArgv[0]) == 0){
            history(size, commandArgv);
            if (mode == 1) {
                printf("wish> ");
            }
            continue;
        } else if (strcmp("path", commandArgv[0]) == 0){
            path(size, commandArgv);
            if (mode == 1) {
                printf("wish> ");
            }
            continue;
        }
        if (mode == 1) {
            printf("wish> ");
        }
    }
    return 0;
}

/*int getStream(int argc, char *argv[], FILE *fp) {
    if (argc == 1) {
        fp = stdin;
        printf("a\n");
        return 1;
    } else if (argc == 2) {
        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        return 0;
    } else {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
}*/

void getOperator(char* line, char* operator) {
    int len = strlen(line);
    int i;
    for (i = 0; i < len; i++) {
        if (line[i] == '|') {
            if (operator != NULL) {
                operator = "error";
            } else {
                operator = "|";
                continue;
            }
        }
        if (line[i] == '>') {
            if (operator != NULL) {
                operator = "error";
            } else {
                operator = ">";
            }
        }
    }
}

int parseList(char *line, Token *head) {
    int i = 0;
    Token *end = head;
    char *token = strtok(line, "\t ");
    while (token) {
        Token *node = (Token*)malloc(sizeof(Token));
        node->word = token;
        end->next = node;
        end = node;
        i++;
        token = strtok(NULL, "\t ");
    }
    end->next = NULL;
    return i;
}

void parseArgv(Token *commandList, char *argv[]) {
    int i = 0;
    Token *current = commandList->next;
    while (current != NULL) {
        argv[i] = current->word;
        i++;
        current = current->next;
    }
    argv[i] = NULL;
    return;
}

void addHistory(History *head, char* line) {
    History* node = (History*)malloc(sizeof(History));
    node->next = head->next;
    node->command = line;
    head->next = node;
    return;
}

void history(int argc, char *argv[]) {
    if (argc > 2) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }
    if (argc == 1) {
        allHistory(commandHistory);
        return;
    }
    double n = atof(argv[1]);
    if (n <= 0.0) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }
    limitHistory(commandHistory, n);
}

void allHistory(History *current) {
    if (current == NULL) {
        return;
    }
    allHistory(current->next);
    printf("%s\n", current->command);
}

void limitHistory(History *current, double n) {
    if (current == NULL || n == 0) {
        return;
    }
    limitHistory(current->next, n - 1);
    printf("%s\n", current->command);
}

void path(int argc, char* argv[]) {
    int i;
    for (i = 1; i < argc; i++) {
        /*int len = strlen(argv[i]);
        if (argv[i][len - 1] == '/') {
            argv[i][len - 1] = '\0';
        }*/
        addPath(argv[i]);
    }
    return;
}

void addPath(char* path) {
    Path* node = (Path*)malloc(sizeof(Path));
    node->next = allPath->next;
    node->content = path;
    allPath->next = node;
    return;
}

void cd(int argc, char* argv[]) {
    if (argc == 1 || argc > 2) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }
    int i = chdir(argv[1]);
    if (i == -1) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }
}