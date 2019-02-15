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

int parseList(char *line, Token *head);
void parseArgv(Token *commandList, char *argv[]);

int main(int argc, char *argv[]) {

    char line[] = "aa bb	cc 	dd";
    Token *commandList = (Token*)malloc(sizeof(Token));
    int size = parseList(line, commandList);
    char *commandArgv[size];
    parseArgv(commandList, commandArgv);
    int i;
    for (i = 0; i < size; i++) {
    	printf("%s\n", commandArgv[i]);
    }
    return 0;
}

int parseList(char *line, Token *head) {
    int i = 0;
    Token *end = head;
    end->next = NULL;
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
    return;
}
