#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#define MAX_LINE 80 /* The maximum length command */

char *args[MAX_LINE/2 + 1]; /* command line arguments */
char *copyArgs[MAX_LINE/2 + 1]; /* copy of command line arguments w/o redirection*/
char *copyArgs2[MAX_LINE/2 + 1]; /* copy of command line arguments w/o redirection*/
void writeToPipe(int, int);
void readFromPipe(int, int);

int main(void)
{
	// Declare variables

	int should_run = 1; /* flag to determine when to exit program */
	char command[MAX_LINE] = "";
	int i = 0;
	int indexOfAmp;
	int indexOfPipeInArgs;
	char* token;
	char* token2;
	char* token3;
	char* token4;
	char prevCommand[MAX_LINE/2 + 1];
	char theActualPrevCommand[MAX_LINE/2 + 1] = " ";
	bool nullifyCopyArgs = false;
	bool nullifyCopyArgs2 = false;
	bool ampPresent;
	int mypipe[2];
	pid_t pid, pidChild;

	while (should_run) {
		i = 0;
		nullifyCopyArgs = false;
		nullifyCopyArgs2 = false;
		ampPresent = false;
		// Print shell prompt
		printf("osh>");
		// Take in input from user of command
		fgets(command, MAX_LINE, stdin);
		//		fflush(stdin);	


		// If they want to exit
		if(strcmp(command, "exit\n") == 0){
			should_run = 0;
			break;
		}

		// Prliminary work with the &
		// Taking out the & and setting a boolean
		// Boolean will be checked before waits
		if(strchr(command, '&')) {
			ampPresent = true;
			indexOfAmp = strlen(command) - 2;
			command[indexOfAmp] = ' ';
		}


		strcpy(prevCommand, command);
	

		if(strchr(command, '|'))
		{
			if(pipe(mypipe))
			{
				printf("Pipe creation failed\n");
				return 1;
			}

			token4 = strtok(command, " \n");

			while(token4 != NULL)
			{
				args[i] = token4;
				token4 = strtok(NULL, " \n");
				if((strchr(args[i], '|')) || nullifyCopyArgs)
				{
					nullifyCopyArgs = true;
					copyArgs[i] = NULL;
				}else {
					copyArgs[i] = args[i];

					indexOfPipeInArgs = i;
				}
				i++;
			}
			// Assign  argument to NULL
			args[i] = token4;
			copyArgs[i] = token4;

			//int count = i - 1;
			int count = indexOfPipeInArgs + 2;
			int j;
			// Continue while the args array has items left
			for(j = 0; count < (MAX_LINE/2 + 1); j++)
			{
				if(args[count] == NULL || nullifyCopyArgs2)
				{
					nullifyCopyArgs2 = true;	
					copyArgs2[j] = NULL;
				}else {
					copyArgs2[j] = args[count];
				}
				count++;
			}

			//(1) fork a child process using fork()
			pid = fork();

			if(pid < 0)
			{
				printf("Fork Failed\n");
				return 1;
			}
			else if(pid == 0){
				// CHILD EXECUTES
				pidChild = fork();
				if(pidChild < 0)
				{
					printf("Fork failed\n");
					return 1;
				} else if(pidChild == 0) {
					//close(mypipe[0]);
					writeToPipe(mypipe[1], mypipe[0]);
					printf("HELLO THERE\n");
				} else {
					//wait(NULL);
					//close(mypipe[1]);
					readFromPipe(mypipe[0], mypipe[1]);
					wait(NULL);
					printf("I am the child who created a child and my child is done\n");
				}

			}else{
				// PARENT EXECUTES
				close(mypipe[1]);
				close(mypipe[0]);
				if(!ampPresent){
					wait(NULL);
				}
				printf("I am the parent and my child is finished.\n");
			}


		}
		// Redirection - PART 3
		else if(strchr(command, '<') || strchr(command, '>'))
		{
			token3 = strtok(command, " \n");

			// While the token is not null, set the arguments to the token
			// and then get another token and finally increment the counter
			while(token3 != NULL)
			{
				args[i] = token3;
				token3 = strtok(NULL, " \n");
				if((strchr(args[i], '<') || strchr(args[i], '>')) || nullifyCopyArgs)
				{
					nullifyCopyArgs = true;
					copyArgs[i] = NULL;
				}else {
					copyArgs[i] = args[i];
				}
				i++;
			}
			// Assign last argument to NULL
			args[i] = token3;
			copyArgs[i] = token3;

			//(1) fork a child process using fork()
			pid = fork();

			if(pid < 0)
			{
				printf("Fork Failed\n");
				return 1;
			}
			else if(pid == 0){
				// CHILD EXECUTES

				// Use the command before it was altered
				// which is prevCommand
				if(strchr(prevCommand, '>'))
				{
					FILE* writeToFile = fopen(args[--i], "w");
					int fd = fileno(writeToFile);
					if(writeToFile < 0)
					{
						printf("ERROR in opening file\n");
						return 1;
					}
					dup2(fd, 1);
					if((execvp(copyArgs[0], copyArgs)) < 0)
					{
						printf("The child did not execute correctly\n");
						return 1;

					}
					close(fd);
					fclose(writeToFile);

					return 0;
				} else if(strchr(prevCommand, '<')) {
					FILE* readFromFile = fopen(args[--i], "r");
					int readFd = fileno(readFromFile);
					if(readFromFile < 0)
					{
						printf("ERROR in opening file\n");
						return 1;
					}
					dup2(readFd, 0);

					if((execvp(copyArgs[0], copyArgs)) < 0)
					{
						printf("The child did not execute correctly\n");
						return 1;

					}
					close(readFd);
					fclose(readFromFile);

					return 0;
				}
			}
			else {
				// PARENT EXECUTES
				if(!ampPresent) {
					wait(NULL);
				}
			}
		}
		// History command - PART 2
		else if(command[0] == '!' && command[1] == '!')
		{
			// Strcmp returns 0 if it is the same
			if(!strcmp(theActualPrevCommand, " "))
			{
				printf("No commands in history.\n");
			} else {

				// Tokenize according to space or newline
				token2 = strtok(theActualPrevCommand, " \n");

				// While there is still something left in the string, 
				// keep tokenizing
				while(token2 != NULL)
				{
					args[i] = token2;
					token2 = strtok(NULL, " \n");
					i++;
				}
				// Assign last argument to NULL
				args[i] = token2;


				// Create child process
				pid = fork();
				if(pid < 0)
				{
					printf("Fork failed\n");
					return 1;
				}else if(pid == 0)
				{
					// CHILD EXECUTES
					if((execvp(args[0], args)) < 0)
					{
						printf("The child did not execute correctly\n");
						return 1;
					}  
				}else {
					// PARENT EXECUTES
					if(!ampPresent) {	
						wait(NULL);
					}	
				}
			}
		}else{ 
			// REGULAR COMMAND IN SHELL

			// Get the token which is the first characters before the space or newline
			// ie. space or newline is the delimiter in the tokenizer
			token = strtok(command, " \n");

			// While the token is not null, set the arguments to the token
			// and then get another token and finally increment the counter
			while(token != NULL)
			{
				args[i] = token;
				token = strtok(NULL, " \n");
				i++;
			}
			// Assign last argument to NULL
			args[i] = token;

			//After reading user input, the steps are:

			//(1) fork a child process using fork()
			pid = fork();

			// If the pid is less than 0, then the fork failed
			if(pid < 0)
			{
				printf("Fork failed\n");
				return 1;
			} else if(pid == 0) {
				// CHILD EXECUTES
				//(2) the child process will invoke execvp()

				if((execvp(args[0], args)) < 0)
				{
					printf("The child did not execute correctly\n");
					return 1;
				}
			} else {
				// PARENT EXECUTES
				//(3) if the last arg is &, parent will not wait()

				close(mypipe[1]);
				close(mypipe[0]);
				if(!ampPresent) {
					wait(NULL);
				}
			}
		}
		strcpy(theActualPrevCommand, prevCommand);
	}
	return 0;
}

void writeToPipe(int file, int file2) {
	close(file2);
	dup2(file, 1);
	close(file);
	if((execvp(copyArgs[0], copyArgs)) < 0)
	{
		printf("The child did not execute correctly,\n");
	}
	return;
}

void readFromPipe(int file, int file2) {
	close(file2);
	dup2(file, 0);
	close(file);
	if((execvp(copyArgs2[0], copyArgs2)) < 0)
	{
		printf("The child did not execute correctly,\n");
	}
	return;
}
