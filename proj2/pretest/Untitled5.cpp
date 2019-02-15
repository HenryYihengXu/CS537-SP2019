#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <io.h>

int main(int argc, char *argv[]) {
	char s[] = "aa    bb\tcc";
	char *p = strtok(s, " \t");
	printf("%s\n", p);
    p = strtok(NULL, " \t");
	printf("%s\n", p);
	return 0;
}
