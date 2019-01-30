I. my-cat.c

This is an relatively easy and short one. The function of it is to print contents of files to shell.

Here is how it works:

It opens files one by one. 
In each loop, it opens the file, then iteratively uses fgets to read contents to the buffer, 
then print the buffer to the shell, until all contents are read. 
Then it opens next file, until all files are done or an error occurs.


II. my-sed.c

The function of it is to replace the first instance of the find_term with the replace_term in one line.

Here is how it works:

I wrote a function that deal with two kinds of inputs: file and stdin.
The function, void dealwithFile(FILE *fp, char *find_term, char *replace_term), takes in a pointer to a stream, and two strings.
It uses getline to read lines from fp one by one and stores the line in buffer.
For each line, it goes through each character in the line to see if there is a match of find_term.
If the character isn't a part of find_term, then it prints it. 
If there is a match, then it prints replace_term and stop matching, and then prints the rest of the line after this first instance.
After that, it processes next line until all lines are done.

For the whole program, it first read find_term and replace_term.
If argc == 3, it will call dealwithFile with stdin.
If argc > 3, it will use fopen to read files one by one.
For each file, it calls dealwithFile with that file.
It keeps doing this until all files are done or an error occurs.


III. myuniq.c

The function of it is to replace adjacent duplicate lines from files or stdin.

Here is how it works:

Same as above, I wrote a function that deal with two kinds of inputs: file and stdin.
The basic idea of the function is similar to the one in my-sed.c. 
It uses getline to read lines from fp one by one.
Before it moves to next line, it will store the line to "prevline" so that it can compare the adjacent two lines. 
Then it moves to next line and compare them using function equals.
If they are same, it will print nothing so that the duplicate line is deleted. 
If they are different, it will print the current line.
It iteratively go through each line.

I wrote another function that compares two line and returns if they are the same.
The function, int equals(char *s1, char *s2), take in two strings.
If two lines have same length, it will go through each character of them and compare them to check if they are the same.
If two lines are same, it returns 1. Otherwise it returns 0.

For the whole program, if argc == 1, it will call dealwithFile with stdin.
If argc > 1, it will use fopen to read files one by one.
For each file, it calls dealwithFile with that file.
It keeps doing this until all files are done or an error occurs.
