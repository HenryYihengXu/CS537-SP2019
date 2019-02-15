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

// Linked list for storing histories.
typedef struct historyList {
    char *command;
    struct historyList *next;
} History;

// Linked list for storing paths.
typedef struct pathList {
    char *content;
    struct pathList *next;
} Path;

//Error message
const char error_message[30] = "An error has occurred\n";
//Head node of history list
History *commandHistory;
//Head node of path list
Path *allPath;

// All function declaration
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
void userCall(int argc, char* argv[], char *fp, char** pipeBuf);

int main(int argc, char *argv[]) {

    // file for batch mode
    FILE *fp;
    int mode;

    //decide interactive or bath mode
    if (argc == 1) {
        fp = stdin;
        mode = 1; // interactive mode
    } else if (argc == 2) {
        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            exit(1);
        }
        mode = 0; //batch mode
    } else {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1); // bad arguments
    }

    //initialize history list
    commandHistory = (History*)malloc(sizeof(History));
    commandHistory ->next = NULL;
    commandHistory ->command = NULL;

    //initialize path list, add /bin to it
    allPath = (Path*)malloc(sizeof(Path));
    allPath->next = (Path*)malloc(sizeof(Path));
    allPath->next->next = NULL;
    allPath->next->content = "/bin";

    //tokenized command arguments
    char** commandArgv;
    // length of command
    int size;

    if (mode == 1) {
        printf("wish> ");
    }

    // redirection file name
    char* file;
    // for tokenized command after "|" when using pipe
    char** pipeBuf = NULL;

    // input line
    char *line = NULL;
    size_t len = 0;
    // iteratively read input
    while (getline(&line, &len, fp) != -1) {
        fflush(stdin);
        fflush(stdout);
        // deal with the \n at the end
        if (line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }
        // if it is an empty command, just continue
        if (line == NULL) {
            if (mode == 1) {
                printf("wish> ");
            }
            continue;
        }
        file = NULL;
        pipeBuf = NULL;
        // get the type of the command: redirection, pipe, or normal
        char* operator = getOperator(line);
        if (strcmp(operator, "error") == 0) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            addHistory(commandHistory, line);
            if (mode == 1) {
                printf("wish> ");
            }
            continue;
        }

        // if it is a redirection command
        if (strcmp(operator, ">") == 0) {
            //add the command to history
            addHistory(commandHistory, line);
            // if nothing before ">"
            if (line[0] == '>') {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }
            //separate the command to two half
            char* left = strtok(line, ">");
            char* right = strtok(NULL, ">");
            // if nothing after ">", then error
            if (right == NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }

            // get the length of the left half command
            int l = getTokNumber(left, "\t ");
            if (l == 0) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }

            // get the length of the right half command
            int r = getTokNumber(right, "\t ");
            // if no file or more than one file, then error
            if (r != 1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }

            // tokenize the left command (parse)
            size = l;
            commandArgv = (char**)malloc((size + 1) * sizeof(char*));
            parseArgv(commandArgv, left, "\t ");

            // get the redirection file name from the right half
            file = strtok(right, "\t ");
        }

        // normal command
        if (strcmp(operator, "none") == 0){
            size = getTokNumber(line, "\t ");
            //if empty command, just continue
            if (size == 0) {
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }
            //add the command to history
            addHistory(commandHistory, line);
            // tokenize the command
            commandArgv = (char**)malloc((size + 1) * sizeof(char*));
            parseArgv(commandArgv, line, "\t ");
        }

        // pipe command
        if (strcmp(operator, "|") == 0) {
            //add the command to history
            addHistory(commandHistory, line);
            // if nothing before "|", error
            if (line[0] == '|') {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }
            // break the command into two half
            char* left = strtok(line, "|");
            char* right = strtok(NULL, "|");
            //if nothing after "|", then error
            if (right == NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }

            // get length of the left half
            int l = getTokNumber(left, "\t ");
            if (l == 0) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }

            // get length of the right half
            int r = getTokNumber(right, "\t ");
            if (r == 0) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }

            // parse the command before "|"
            size = l;
            commandArgv = (char**)malloc((size + 1) * sizeof(char*));
            parseArgv(commandArgv, left, "\t ");

            int n = strlen(right);
            char *s2 = (char *)malloc(n);
            memcpy(s2, right, n);

            // parse the command after "|" and save it for later use
            pipeBuf = (char**)malloc((r + 1) * sizeof(char*));
            parseArgv(pipeBuf, s2, "\t ");
        }

        //built-in commands
        //exit
        if (strcmp("exit", commandArgv[0]) == 0){
            if (size != 1) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                if (mode == 1) {
                    printf("wish> ");
                }
                continue;
            }
            exit(0);
        } else if (strcmp("cd", commandArgv[0]) == 0){         // cd
            cd(size, commandArgv);
        } else if (strcmp("history", commandArgv[0]) == 0){    // history
            history(size, commandArgv);
        } else if (strcmp("path", commandArgv[0]) == 0){       // path
            path(size, commandArgv);
        } else {
            userCall(size, commandArgv, file, pipeBuf);        // user command
        }
        if (mode == 1) {
            printf("wish> ");                                  // next prompt
        }
    }
    return 0;
}

// get the operator of the command: none, |, or >.
// and handle errors with mutiple operators
char* getOperator(char* line) {
    int len = strlen(line);
    int i;
    char* operator;
    operator = "none";
    // go through the command to find and count operators
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

//get the number of tokens in the command
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

// parse a line to a command arguments array (tokenize)
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

// add a new command line to the history list
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

// implement built-in command history
void history(int argc, char *argv[]) {
    if (argc > 2) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        return;
    }
    // without argument
    if (argc == 1) {
        allHistory(commandHistory->next);
        return;
    }
    // with one argument
    double n;
    int i;
    int flag = 0;
    for (i = 0; i < strlen(argv[1]); i++) {
        if (argv[1][i] != '0') {
            flag = 1;
            break;
        }
    }
    if (flag == 0) {
        n = 0;
    } else {
        n = atof(argv[1]);
        if (n == 0.0) {
            write(STDERR_FILENO, error_message, strlen(error_message));
            return;
        }
        if (n < 0) {
            n = 0;
        }
    }

    limitedHistory(commandHistory->next, n);
}

// recursively go through history list so that the output order is correct
void allHistory(History *current) {
    if (current == NULL) {
        return;
    }
    allHistory(current->next);
    printf("%s\n", current->command);
}

// recursively go through history list with a limited depth
void limitedHistory(History *current, double n) {
    if (current == NULL || n <= 0) {
        return;
    }
    limitedHistory(current->next, n - 1);
    printf("%s\n", current->command);
}

// add all path to the path list
void path(int argc, char* argv[]) {
    int i;
    for (i = 1; i < argc; i++) {
        addPath(argv[i]);
    }
    return;
}

// add one path to the path list
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

// cd
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

// deal with all user call
// !!!!!! core function of the project !!!!!!!!
void userCall(int argc, char* argv[], char *fp, char** pipeBuf) {
    // get the complete path for the program
    char* path = findPath(argv[0]);
    // create a pipe for pipe command

    int fd[2];
    if (pipeBuf != NULL) {
        pipe(fd);
    }
    // create child process
    int rc = fork();
    if (rc < 0) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(0);
    }
    if (rc == 0) { // in child process
        if (fp != NULL) {            // !! redirection command !!
            // redirect output and error to the file
            freopen(fp, "w", stdout);
            freopen(fp, "w", stderr);
            if (path == NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
            // execute the user process
            int exec_rc = execv(path, argv);
            if (exec_rc == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
        } else if (pipeBuf != NULL) {  // !!! pipe command !!!
            // close the pipe end of the second process
            // connect its output to one side of the pipe
            // connect its stderr to one side of the pipe
            close(fd[0]);
            dup2(fd[1],STDOUT_FILENO);
            dup2(fd[1], STDERR_FILENO);
            if (path == NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
            // execute the first process
            int exec_rc = execv(path, argv);
            if (exec_rc == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
        } else {                   // normal command
            if (path == NULL) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
            // execute the user process
            int exec_rc = execv(path, argv);
            if (exec_rc == -1){
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
        }
    } else {
        wait(NULL);
        if (pipeBuf != NULL) {   // !!! second half of pipe command !!!
            // create another child process for the second half of pipe command!
            int rc2 = fork();
            if (rc2 < 0) {
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(0);
            }
            if (rc2 == 0) {      // in the second child process!
                // close the pipe end of the first process
                // connect its input to the other side of the pipe
                close(fd[1]);
                dup2(fd[0], 0);
                char* path2 = findPath(pipeBuf[0]);
                // execute the second half of pipe command using what we saved previously!
                int execv_rc2 = execv(path2, pipeBuf);
                if (execv_rc2 == -1) {
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    exit(0);
                }
            } else {
                // close both end of the pipe
                close(fd[0]);
                close(fd[1]);
                wait(NULL);
                pipeBuf = NULL;
            }

        }
    }
}

// find and return the whole path given the program name
char* findPath(char* s) {
    Path* current = allPath->next;
    // go through path list
    while (current != NULL) {
        // concatenate the path with the program name
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
