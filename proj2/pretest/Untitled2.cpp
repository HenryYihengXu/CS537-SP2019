#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <io.h>

int main() {
	char s[] = "aa      bb cc dd";
	int i = 0;
    char *argv[10];
    char *token = strtok(s, " ");
    while (token) {
    	printf("%d\n", i);
    	printf("%s\n", token);
        argv[i] = token;
        printf("%s\n", argv[i]);
        i++;
        token = strtok(NULL, "\t ");
    }
   	printf("%s\n", argv[1]);
	printf("%s\n", argv[2]);
	printf("%s\n", argv[3]);
	printf("end");
	return 0;
	return 0;
}
