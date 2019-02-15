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
//void userCall(int argc, char* argv[], char *fp, char** pipeBuf, int pipeBufLen);
void userCall(int argc, char* argv[], char *fp, int isPipe);

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
    char** pipeBuf = NULL;
    int pipeBufLen = 0;
    int isPipe = 0;

    char *line = NULL;
    size_t len = 0;
    while (getline(&line, &len, fp) != -1) {
        fflush(stdin);
        fflush(stdout);
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

            size = l;
            commandArgv = (char**)malloc((size + 1) * sizeof(char*));
            parseArgv(commandArgv, left, "\t ");

            file = strtok(right, "\t ");
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
            if (pipeBuf != NULL) {
                char** temp = commandArgv;
                commandArgv = (char**)malloc((size + pipeBufLen + 1) * sizeof(char*));
                int i;
                for (i = 0; i < pipeBufLen; i++) {
                    commandArgv[i] = pipeBuf[i];
                }
                for (i = pipeBufLen; i < size + pipeBufLen; i++) {
                    commandArgv[i] = temp[i - pipeBufLen];
                }
                commandArgv[i] = NULL;

                pipeBuf = NULL;
                pipeBufLen = 0;
                isPipe = 0;
            }
        }

        if (strcmp(operator, "|") == 0) {
            addHistory(commandHistory, line);
            if (line[0] == '|') {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }
            char* left = strtok(line, "|");
            char* right = strtok(NULL, "|");
            if (right == NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }

            int l = getTokNumber(left, "\t ");
            if (l == 0) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }

            int r = getTokNumber(right, "\t ");
            if (r == 0) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }

            isPipe = 1;
            size = l;
            commandArgv = (char**)malloc((size + 1) * sizeof(char*));
            parseArgv(commandArgv, left, "\t ");

            int n = strlen(right);
            char *s2 = (char *)malloc(n);
            memcpy(s2, right, n);

            pipeBuf = (char**)malloc((r + 1) * sizeof(char*));
            pipeBufLen = r;
            parseArgv(pipeBuf, s2, "\t ");
        }

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
        } else if (strcmp("history", commandArgv[0]) == 0){
            history(size, commandArgv);
        } else if (strcmp("path", commandArgv[0]) == 0){
            path(size, commandArgv);
        } else {
            //userCall(size, commandArgv, file, pipeBuf, pipeBufLen);
            userCall(size, commandArgv, file, isPipe);
        }
        if (mode == 1) {
            printf("wish> ");
        }
    }
    return 0;
}

char* getOperator(char* line) {
    int len = strlen(line);
    int i;
    char* operator;
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

void userCall(int argc, char* argv[], char *fp, int isPipe) {
    char* path = findPath(argv[0]);
    int fd[2];
    pipe(fd);
    int rc = fork();
    if (rc == 0) {
        if (fp != NULL) {
            freopen(fp, "w", stdout);
            freopen(fp, "w", stderr);
            /*close(STDOUT_FILENO);
            open(fp, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);*/
            if (path == NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
            int exec_rc = execv(path, argv);
            if (exec_rc == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
        } else if (isPipe) {
            //freopen("stdin", "w", stdout);
            close(fd[0]);
            dup2(fd[1],STDOUT_FILENO);
            if (path == NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
            int exec_rc = execv(path, argv);
            if (exec_rc == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
        } else {
            if (path == NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
            int exec_rc = execv(path, argv);
            if (exec_rc == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
        }
    } else {
        wait(NULL);
        if (isPipe) {
            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
        }
        //fflush(stdout);
        //fflush(stdin);
    }
}

/*void userCall(int argc, char* argv[], char *fp, char** pipeBuf, int pipeBufLen) {
    char* path = findPath(argv[0]);
    int fd[2];
    //char * buffer;
    //argv[0] = path;
    if (pipeBuf != NULL) {
        pipe(fd);
        //buffer = malloc(1000);
    }
    int rc = fork();
    if (rc == 0) {
        if (fp != NULL) {
            freopen(fp, "w", stdout);
            freopen(fp, "w", stderr);
            *//*close(STDOUT_FILENO);
            open(fp, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);*//*
            if (path == NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
            int exec_rc = execv(path, argv);
            if (exec_rc == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
        } else if (pipeBuf != NULL) {
            //freopen(STDIN_FILENO, "w", stdout);
            //close(STDOUT_FILENO);
            *//*int i = open(STDIN_FILENO, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
            if (i != 0) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }*//*
            //open(STDIN_FILENO, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);



            *//*close(fd[0]);
            dup2(fd[1], 1);*//*



            //setbuf(stdout, buffer);

            if (path == NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
            int exec_rc = execv(path, argv);
            if (exec_rc == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
        } else {
            if (path == NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
            int exec_rc = execv(path, argv);
            if (exec_rc == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
        }
    } else {
        wait(NULL);
        if (pipeBuf != NULL) {
            int pid2 = fork();
            if (pid2 == 0){
                *//*close(fd[1]);
                dup2(fd[0], 0);*//*

                *//*char *line = NULL;
                size_t len = 0;
                getline(&line, &len, stdin);*//*

                char* line = read()

                fflush(stdin);
                fflush(stdout);
                if (line[strlen(line) - 1] == '\n') {
                    line[strlen(line) - 1] = '\0';
                }
                int size = getTokNumber(line, "\t ");
                char **commandArgv = (char**)malloc((size + 1) * sizeof(char*));
                parseArgv(commandArgv, line, "\t ");

                char** temp = commandArgv;
                commandArgv = (char**)malloc((size + pipeBufLen + 1) * sizeof(char*));
                int i;
                for (i = 0; i < pipeBufLen; i++) {
                    commandArgv[i] = pipeBuf[i];
                }
                for (i = pipeBufLen; i < size + pipeBufLen; i++) {
                    commandArgv[i] = temp[i - pipeBufLen];
                }
                commandArgv[i] = NULL;

                pipeBuf = NULL;


                char* path2 = commandArgv[0];
                int exec_rc = execv(path2, commandArgv);
                if (exec_rc == -1){
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    exit(0);
                }
            }else{
                wait(NULL);
                //printf("done\n");
            }
        }
    }
}*/

char* findPath(char* s) {
    Path* current = allPath->next;
    while (current != NULL) {
        int n = strlen(current->content) + strlen(s) + 1;
        char* path = (char *)malloc(n);
        memcpy(path, current->content, n);
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