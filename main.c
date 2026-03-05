/*
 * main.c
 *
 *  Created on: Mar 17 2017
 *      Author: david
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <string.h>
#include "dsh.h"

#define MAX_PROC 1024

/* 
	Remaining issues (that I know of):
	1. Mode One background mode unfortunately does not work
	2. The inbuilt exit command occasionally experiences a stack smashing error on exit
*/

int main(int argc, char *argv[]) {

	// DO NOT REMOVE THE BLOCK BELOW (FORK BOMB PREVENTION) //
	struct rlimit limit;
	limit.rlim_cur = MAX_PROC;
	limit.rlim_max = MAX_PROC;
	setrlimit(RLIMIT_NPROC, &limit);
	// DO NOT REMOVE THE BLOCK ABOVE THIS LINE //

	char *cmdline = malloc(MAXBUF); // buffer to store user input from commmand line
	char** tokens; 
	int quitter = 1;

	while (quitter == 1){
		printf("dsh> ");
		fgets(cmdline, MAXBUF, stdin);

		//trim preceding and trailing whitespace
		cmdline = trim(cmdline);
		
		if (strcmp(cmdline, "\0") == 0){
			//if our string is empty, then restart the input loop
			continue;
		}

		//tokenize input by spaces
		tokens = split(cmdline, " ");

		if (strcmp(tokens[0], "exit") == 0){
			//built-in command: exit

			quitter = 0;
			break;
		}
		else if (strcmp(tokens[0], "pwd") == 0){
			//built-in command: print working directory

			char cwd[] = "";
			getcwd(cwd, MAXBUF);
			printf("%s\n", cwd);
		}
		else if (strcmp(tokens[0], "cd") == 0){
			//built-in command: change directory

			if (tokens[1] != NULL){
				if(chdir(tokens[1]) != 0){
					printf("%s: no such file or directory\n", tokens[1]);
				}
			}
			else{
				//no path given, so change to home directory
				chdir(getenv("HOME"));
			}
		}
		else if (tokens[0][0] == '/'){
			//full path to executable given, so use mode one
			modeOne(tokens);
		}
		else{
			//full path to executable not given, so use mode two
			modeTwo(tokens);
		}
	}

	free(cmdline);
	freeArray(tokens);

	return 0;
}
