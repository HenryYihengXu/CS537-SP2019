#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void dealwithFile(FILE *fp, char *find_term, char *replace_term)
{
	char *line = NULL;
	size_t len = 0;
	while(getline(&line, &len, fp) != -1) {
		if (strlen(line) < strlen(find_term)) {
			printf("%s", line);
			continue;
		}
		int j;
		for (j = 0; j < strlen(line) - strlen(find_term); j++) {
			int flag = 1;
			int k;
			for (k = 0; k < strlen(find_term); k++) {
				if (line[j + k] != find_term[k]) {
					flag = 0;
					break;
				}
			}
			if (flag == 0) {
				printf("%c", line[j]);
			} else {
				if (strlen(replace_term) != 2 || replace_term[0] != '"' || replace_term[1] != '"') {
					printf("%s", replace_term);
				}
				j = j + strlen(find_term);
				break;
			}
		}
		for (;j < strlen(line); j++) {
			printf("%c", line[j]);
		}
	}
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
	 	printf("my-sed: find_term replace_term [file ...]\n");
	 	exit(1);
 	}
 	if (argc == 3) {
	 	char *find_term = argv[1];
		char *replace_term = argv[2];		
		dealwithFile(stdin, find_term, replace_term);
		return 0;
	} else {
		char *find_term = argv[1];
		char *replace_term = argv[2];	
		int i;
		for (i = 3; i < argc; i++) {	
			FILE *fp = fopen(argv[i], "r");
			if (fp == NULL) {
				printf("my-sed: cannot open file\n");
				exit(1);
			}
			dealwithFile(fp, find_term, replace_term);
			fclose(fp);
		}
	} 
	return 0;
}
