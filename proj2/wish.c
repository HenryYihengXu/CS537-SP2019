#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <io.h>

typedef struct tokenList {
    char *word;
    struct tokenList *next;
} Token;

typedef struct historyList {
    char *command;
    struct historyList *next;
} History;

const char error_message[30] = "An error has occurred\n";
//const char* builtIn[4] = {"exit", "cd", "history", "path"};

int getStream(int argc, char *argv[], FILE *fp);
char* getOperator(char* line);
int parseList(char *line, Token *head);
void parseArgv(Token *commandList, char *argv[]);
void addHistory(History *head, char* line);

int main(int argc, char *argv[]) {

    FILE *stream;
    int mode = getStream(argc, argv, stream);

    History *commandHistory = (History*)malloc(sizeof(History));
    history->next = NULL;
    history->command = NULL;

    char *line = NULL;
    size_t len = 0;

    if (mode == 1) {
        printf("wish> ");
    }

    while (getline(&line, &len, mode) != -1) {
        if (line == NULL) {
            continue;
        }
        char operator = getOperator(line);
        if (strcmp(operator, "error") != 0) {
            continue;
        }
        Token *commandList = (Token*)malloc(sizeof(Token));
        int size = parseList(line, commandList);
        if (size == 0) {
            continue;
        }
        addHistory(commandHistory, line);
        char* commandArgv[size + 1];
        parseArgv(commandList, commandArgv);
        free(commandList);

        if (strcmp("exit", commandArgv[0]) == 0){
            exit(0);
        } else if (strcmp("cd", commandArgv[0]) == 0){
            cd();
            continue;
        } else if (strcmp("history", commandArgv[0]) == 0){
            history();
            continue;
        } else if (strcmp("path", commandArgv[0]) == 0){
            path();
            continue;
        } else {

        }
    }
    return 0;
}

int getStream(int argc, char *argv[], FILE *fp) {
    if (argc == 1) {
        fp = stdin;
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
}



char* getOperator(char* line) {
    char* operator = NULL;
    int i;
    for (i = 0; i < strlen(line); i++) {
        if (line[i] == '|') {
            if (operator != NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                return "error";
            } else {
                operator = "|";
                continue;
            }
        }
        if (line[i] == '>') {
            if (operator != NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                return "error";
            } else {
                operator = ">";
            }
        }
    }
    return operator;
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