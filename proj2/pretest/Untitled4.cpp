#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <io.h>

int main(int argc, char *argv[]) {
	char s[] = "aa bb, cc dd";
	int n = strlen(s);
    char *s2 = (char *)malloc(n);
    memcpy(s2, s, n);
	char *p = strtok(s, ",");
	char *q = strtok(NULL, ",");
	printf("%s\n", s);
	//printf("%s\n", s2);
	printf("%s\n", p);
	printf("%s\n", q);
	char *n1 = strtok(p, " ");
	char *m1 = strtok(NULL, " ");
	char *n2 = strtok(q, " ");
	char* m2 = strtok(NULL, " ");
	printf("%s\n", s);
	printf("%s\n", p);
	printf("%s\n", q);
	printf("%s\n", n1);
	printf("%s\n", m1);
	printf("%s\n", n2);
	printf("%s\n", m2);
	printf("end");
	return 0;
}
