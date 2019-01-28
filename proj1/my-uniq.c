#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]) {
	if (argc == 1) {
		char *prevline = NULL;
		char *line = NULL;
		size_t len = 10;
		while(getline(&line, &len, stdin) != -1) {
			if (prevline != NULL && strlen(prevline) == strlen(line)) {
				int flag = 1;
				int i;
				for (i = 0; i < strlen(line); i++) {
					if (line[i] != prevline[i]) {
						flag = 0;
						break;
					}
				}
				if (flag == 0) {
					printf("%s", line);
				}
			} else {
				printf("%s", line);
			}
			prevline = line;
			line = NULL;
			len = 10;
		}
		return 0;
	}
	int i;
	for (i = 1; i < argc; i++) {	
		FILE *fp = fopen(argv[i], "r");
		if (fp == NULL) {
			printf("my-uniq: cannot open file\n");
			exit(1); 
		}
		char *line = NULL;
		char *prevline = NULL;
		size_t len = 10;
		while(getline(&line, &len, fp) != -1) {
			if (prevline != NULL && strlen(prevline) == strlen(line)) {
				int flag = 1;
				int i;
				for (i = 0; i < strlen(line); i++) {
					if (line[i] != prevline[i]) {
						flag = 0;
						break;
					}
				}
				if (flag == 0) {
					printf("%s", line);
				}
			} else {
				printf("%s", line);
			}
			prevline = line;
			line = NULL;
			len = 10;
		}
		fclose(fp);
	}
	return 0;
}
