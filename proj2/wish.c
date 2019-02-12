#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>
#define _GNU_SOURCE

/*typedef struct tokenList {
    char *word;
    struct tokenList *next;
} Token;*/

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

char* getOperator(char* line);
//int parseList(char *line, Token *head);
//void parseArgv(Token *commandList, char *argv[]);
int getTokNumber(char *str, char *delimiters);
void parseArgv(char *argv[], char* line, char* delimiters);
void addHistory(History *head, char* line);
void allHistory(History *current);
void limitedHistory(History *current, double n);
void history(int argc, char *argv[]);
void addPath(char* path);
void path(int argc, char* argv[]);
void cd(int argc, char* argv[]);
char* findPath(char* s);
void userCall(int argc, char* argv[], char *fp);

int main(int argc, char *argv[]) {

    FILE *fp;
    int mode;

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

    char** commandArgv;
    int size;

    if (mode == 1) {
        printf("wish> ");
    }

    char* file;

    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, fp) != -1) {

        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        if (line == NULL) {
            if (mode == 1) {
                printf("wish> ");
            }
            continue;
        }
        file = NULL;
        char* operator = getOperator(line);
        //printf("%s\n", operator);
        if (strcmp(operator, "error") == 0) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            addHistory(commandHistory, line);
            if (mode == 1) {
                printf("wish> ");
            }
            continue;
        }

        if (strcmp(operator, ">") == 0) {
            addHistory(commandHistory, line);
            if (line[0] == '>') {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }
            char* left = strtok(line, ">");
            char* right = strtok(NULL, ">");
            if (right == NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }
            //printf("%s\n", left);
            //printf("%s\n", right);

            int l = getTokNumber(left, "\t ");
            if (l == 0) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }


            int r = getTokNumber(right, "\t ");
            if (r != 1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }
            //printf("%s\n", left);

            size = l;
            commandArgv = (char**)malloc((size + 1) * sizeof(char*));
            parseArgv(commandArgv, line, "\t ");

            file = strtok(right, "\t ");
            //printf("%s\n", file);

            /*if (freopen(file, "w", stdout) == NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }*/
        }
        if (strcmp(operator, "none") == 0){
            size = getTokNumber(line, "\t ");
            if (size == 0) {
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }
            addHistory(commandHistory, line);
            commandArgv = (char**)malloc((size + 1) * sizeof(char*));
            //char* commandArgv[size + 1];
            parseArgv(commandArgv, line, "\t ");
        }

        /*Token *commandList = (Token*)malloc(sizeof(Token));
        int size = parseList(line, commandList);*/
        //printf("%s\n", line);

        //printf("%s\n", commandArgv[0]);

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
        } else {
            userCall(size, commandArgv, file);
        }

        //fclose(stdout);
        //freopen("/dev/tty","w",stdout);
        if (mode == 1) {
            printf("wish> ");
        }
    }
    return 0;
}

char* getOperator(char* line) {
    int len = strlen(line);
    int i;
    char* operator = (char*)malloc(sizeof(char*));
    operator = "none";
    for (i = 0; i < len; i++) {
        if (line[i] == '|') {
            if (strcmp(operator, "none") != 0) {
                operator = "error";
                return operator;
            } else {
                operator = "|";
                continue;
            }
        }
        if (line[i] == '>') {
            if (strcmp(operator, "none") != 0) {
                operator = "error";
                return operator;
            } else {
                operator = ">";
            }
        }
    }
    return operator;
}

int getTokNumber(char *str, char *delimiters)
{
    int n = strlen(str);
    char *s2 = (char *)malloc(n);
    memcpy(s2, str, n);

    n = 0;
    char *pch = strtok(s2, delimiters);
    while (pch != NULL)
    {
        n++;
        pch = strtok(NULL, delimiters);
    }
    free(s2);
    return n;
}

void parseArgv(char *argv[], char* line, char* delimiters) {
    int n = 0;
    char *pch = strtok(line, delimiters);
    while (pch != NULL)
    {
        argv[n] = pch;
        n++;
        pch = strtok(NULL, delimiters);
    }
    argv[n] = NULL;
}

void addHistory(History *head, char* line) {
    History* node = (History*)malloc(sizeof(History));
    int n = strlen(line);
    char *his = (char *)malloc(n);
    memcpy(his, line, n);
    node->next = head->next;
    node->command = his;
    head->next = node;
    return;
}

void history(int argc, char *argv[]) {
    if (argc > 2) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }
    if (argc == 1) {
        allHistory(commandHistory->next);
        return;
    }
    double n = atof(argv[1]);
    if (n <= 0.0) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }
    limitedHistory(commandHistory->next, n);
}

void allHistory(History *current) {
    if (current == NULL) {
        return;
    }
    allHistory(current->next);
    printf("%s\n", current->command);
}

void limitedHistory(History *current, double n) {
    if (current == NULL || n == 0) {
        return;
    }
    limitedHistory(current->next, n - 1);
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
    int n = strlen(path);
    char *p = (char *)malloc(n);
    memcpy(p, path, n);
    Path* node = (Path*)malloc(sizeof(Path));
    node->next = allPath->next;
    node->content = p;
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

void userCall(int argc, char* argv[], char *fp) {
    char* path = findPath(argv[0]);
    if (path == NULL) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }
    argv[0] = path;
    int rc = fork();
    if (rc == 0) {
        if (fp != NULL) {
            freopen(fp, "w", stdout);
            /*close(STDOUT_FILENO);
            open(fp, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);*/
        }
        int exec_rc = execv(path, argv);
        if (exec_rc == -1){
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(0);
        }
    } else {
        wait(NULL);
    }
}

char* findPath(char* s) {
    Path* current = allPath->next;
    while (current != NULL) {
        //printf("%s\n", current->content);
        int n = strlen(current->content) + strlen(s) + 1;
        char* path = (char *)malloc(n);
        memcpy(path, current->content, n);
        //printf("%s\n", current->content);
        //printf("%s\n", path);
        if (path[strlen(path) - 1] == '/') {
            strcat(path, s);
        } else {
            strcat(path, "/");
            strcat(path, s);
        }
        if (access(path, X_OK) == 0) {
            //printf("%s\n", path);
            return path;
        } else {
            free(path);
            current = current->next;
        }
    }
    return NULL;
}