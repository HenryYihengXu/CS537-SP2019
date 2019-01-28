I. my-cat.c

This is an relatively easy and short one. The function of it is to print contents of files to shell.

Here is how it works:

It opens files one by one. In each loop, it opens the file, then iteratively uses fgets to read contents to the buffer, then print the buffer to the shell, until all contents are read. Then it opens next file, until all files are done or an error occurs.


II. my-sed.c

The function of it is to replace the first instance of the find_item with the replace_item in one line.

Here is how it works:

I devided the program into two part:

If argc == 3, then it reads texts from stdin. It first read find_term and replace_term. Then it uses getline to read lines from stdin one by one and stores the line in buffer. For each line, it goes through each character in the line to see if there is a match of find_term. If the character isn't a part of find_term, then it prints it. If there is a match, then it prints replace_term and stop matching. Then it prints the rest of the line after this first instance. After that, it processes next line until all lines are done.

If argc > 3, it reads texts from files. It opens files one by one. For each file, it iteratively uses getline to read lines and stores the line in buffer. Then it does the same thing as the first part. It keeps doing this until all files are done or an error occurs.


III. myuniq.c

The function of it is to replace adjacent duplicate lines from files or stdin.

Here is how it works:

Same as above, there are two part, one reads contents from files, the other reads contents from stdin, but doing the same thing.

It iteratively go through each line. It uses getline to read the current line and stores it in "line." Before it moves to next line, it will store the line to "prevline" so that it can compare the adjacent two lines. Then it moves to next line. If two lines have same length, it will go through each character of them to check if they are the same. If they are same, it will print nothing so that the duplicate line is deleted. If they are different, it will print the current line. It keeps doing this until all files are done or an error occurs.