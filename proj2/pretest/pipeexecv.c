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

int main(){

    int fd[2];

    const char* cmd="ls | wc";//pipe to be coded
    //char* buf[100];// memory
    char* arg1[] = {"ls", NULL};
    char* arg2[] = {"grep", "i", NULL};
    pipe(fd);

    printf("1\n");
    pid_t pid=fork();
    printf("2\n");

    if(cmd){//just test the command

        if(pid==0){
            printf("3\n");
            close(fd[0]);
            dup2(fd[1], 1);
            execv("ls", arg1);
            //execlp("ls", "ls", NULL);
        }

        else{
            wait(NULL);
            printf("4\n");
            close(fd[1]);
            dup2(fd[0], 0);
            execv("grep", arg2);
            //execlp("grep", "grep", "i", NULL);
            printf("5\n");
        }
    }

    return 0;
}