Data structure and initializatin:

	Create a linked list for history, and a linked list for path and add /bin to it.
	Create a string for batch file name.
	Create a char** for command arguments.
	Create a string for redirection file name.
	Create a char** for saving the command after "|" for pipe.
	Decide interactive mode or batch mode, and set where to get input.

Read and parse the input line:

	The program uses a while loop to iteratively read input either from 
	stdin or a batch file (using getline()).

	After read the line, it checks the length of the arguments and handle errors.

	If it is not an empty command, the program copy this line and adds it 
	to the front of the history list by calling addHistory().

	Then it looks for the operator: |, >, or none, and each of them has a brunch.

	For normal command (not > nor |), it calls getTokNumber() to know how 
	many arguments are there in the command, so that it can create an 
	char* array with right length.

	Then it calls parseArgv() to tokenize the input line into an array of 
	arguments, which is commandArgv[] in my program, for later use.

	For redirection command, it first breaks the input line into two parts 
	by ">". It treat the left part in the same way as the normal command. 
	Then it parses the right part to a file name for redirection.

	For pipe command, it breaks the line into two parts by "|". 
	It treat the left part in the same way as the normal command to 
	get commandArgv[]. It also treat the right part in the same way 
	as the normal command to get pipeBuf[], which is the arguments for 
	the command after "|".

Run command:

	Now we get the parsed command commandArgv[], it's time to run it.

	It first check the first argument in commandArgv[] to see if it is 
	a built-in command:

		error:

			First check # of arguments. If correct, exit(0).

		cd:

			First check # of arguments. If correct, call chidir(argv[1]).

		path:

			iteratively go through every argument in the array, 
			add each to the front of the path list.

		history:

			recursively go through every argument in the history list, 
			so that the output order is correct. 
			If it has one number argument, then do recursion with a depth limit.

	If it is not a built-in command, the call userCall() to handle 
	user processes, and here comes the core part of the project:

		For user call, it first find the complete path of the program by 
		calling findPath(). In that function, it go through every path in 
		the path list and uses access() to see if the program is in this 
		path. If found, then it concatenates the path with the program 
		name and return it.

		After find the path, it uses fork() to create a child process. 
		There are three brunches for redirection, pipe, and normal command.

		For normal command, in the child process, it simply call execv() 
		to execute the user program with the path and command arguments 
		array. In the parent process, it waits until the child process 
		finishes, and then return.

		For redirection command, in the child process, it first use 
		freopen() to redirect the stdout and stderr to the given file. 
		Then it does the same thing as the normal command.

		For pipe command, it will first create a pipe before fork() so 
		that two children process can communicate. After creating the pipe, 
		it creates the first child process.

		In the first child process, it closes the pipe end of the second 
		process. Then it connects its stdout and stderr to one side of 
		the pipe. Then it calls execv() to execute the first program with 
		the first half of the pipe command.

		Then in the parent process, after the first child finishes, 
		it creates a second child process. In the second child process, 
		it closes the pipe end of the first process. 
		Then it connects its stdin to other side of the pipe, 
		so that this process can receive the output from the last process 
		as its input. Finally, it calls execv() to execute the second program 
		with the right half of the pipe command.

		Finally, in the parent process, 
		it waits until the second process finishes, and returns.

For now, a single input line is finished. 
The wish will go back to the beginning and read other input lines.

-- the end --
