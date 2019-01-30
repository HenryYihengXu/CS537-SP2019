#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int equals(char *s1, char *s2)
{
	if (strlen(s1) == strlen(s2))
	{
		int i = 0;
		for (i = 0; i < strlen(s1); i++)
			if (s1[i] != s2[i])
				return 0;
		return 1;
	}
	else
		return 0;
}

void dealwithFile(FILE *fp)
{
	char *prevline = NULL;
	char *line = NULL;
	size_t len = 0;
	while (getline(&line, &len, fp) != -1)
	{
		if (prevline == NULL)
		{
			printf("%s", line);
		}
		else if (!equals(prevline, line))
		{
			printf("%s", line);
		}
		free(prevline);
		prevline = line;
		line = NULL;
	}
}

int main(int argc, char *argv[])
{
	if (argc == 1)
	{
		dealwithFile(stdin);
		return 0;
	}
	int i;
	for (i = 1; i < argc; i++)
	{
		FILE *fp = fopen(argv[i], "r");
		if (fp == NULL)
		{
			printf("my-uniq: cannot open file\n");
			exit(1);
		}
		dealwithFile(fp);
		fclose(fp);
	}
	return 0;
}
