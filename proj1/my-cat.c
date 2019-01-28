#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	if (argc == 1) {
	 	exit(0);
 	}
 	int i;
	for (i = 1; i < argc; i++) {	
		FILE *fp = fopen(argv[i], "r");
		if (fp == NULL) {
			printf("my-cat: cannot open file\n");
			exit(1); 
		}
		char buffer[100]; 
		while (fgets(buffer, 100, fp)) {
			printf("%s", buffer);
		}
		fclose(fp);
	}
}
