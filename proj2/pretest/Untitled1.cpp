#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <io.h>

char** parse(char *line) {
	int i = 0;
    char *argv[10];
    char *token = strtok(line, " ");
    while (token) {
        argv[i] = token;
        i++;
        token = strtok(NULL, "\t ");
    }
    return argv;
}

int main(int argc, char *argv[]) {
	char s[] = "aa bb cc dd";
	int i;
	char **result = parse(s);
	printf("%s\n", result[1]);
	printf("%s\n", result[2]);
	printf("%s\n", result[3]);
	printf("end");
	return 0;
}
