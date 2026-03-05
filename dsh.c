/*
 * dsh.c
 *
 *  Created on: Aug 2, 2013
 *      Author: chiu
 */
#include "dsh.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <errno.h>
#include <err.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>

// TODO: Your function definitions below (declarations in dsh.h)

char** split(char *str, char *delim){
    int numTokens = 1;

    //count how many tokens are in this input
    for (int i = 0; i < strlen(str); i++){
        //if str[i] is a space, then increment our tokens count
        if (str[i] == delim[0]){
            numTokens++;
        }
    }

    //array to hold all of our char pointers
    char** array = malloc(numTokens * sizeof(char*));

    //allocate a pointer to each string in our tokenized input
    for (int j = 0; j < numTokens; j++){
        char* charPointer = malloc(MAXBUF);
        array[j] = charPointer;
    }

    //assign first character for strtok loop to work with
    char* token = strtok(str, delim);

    //assign pointers within array to tokenized input
    for (int k = 0; k < numTokens; k++){
        strcpy(array[k], token);

        token = strtok(NULL, delim);
    }

    //mark final element of array as NULL
    array[numTokens + 1] = NULL;

    return array;
}

char* trim(char* str){
    int index = 0;

    while (isspace(str[index])){
        //increment index until the first non-space character in our string
        index++;
    }

    //move string pointer forwards in the string by however many spaces we found
    str += index;

    index = strlen(str) - 1;
    
    while (isspace(str[index])){
        //replace ending spaces with null character until we reach non-space chars
        str[index] = '\0';
        index--;
    }

    return str;
}

void freeArray(char** array){
    int i = 0;

    while (array[i] != NULL){
        free(array[i]);
        i++;
    }

    free(array);
}

void modeOne(char** command){
    if (access(command[0], F_OK | X_OK) == 0) {
        //file exists and can be executed!

        //get index to the last arg of command so we can check fore or background
        int i = 0;
        while (command[i] != NULL){
            i++;
        }

        if (strcmp(command[i - 1], "&") == 0){
            //if last arg is &, then run in background: dsh won't wait for child to end

            pid_t forked = fork();
            if (forked < 0){
                perror("Failed to fork in Mode One background\n");
                exit(1);
            }
            else if (forked == 0){
                execv(command[0], command);
            }
            else{
                printf("Returning to shell\n");
            }
        }
        else{
            //run in foreground: dsh (parent) will wait for child to end

            pid_t forked = fork();
            if (forked < 0){
                perror("Failed to fork in Mode One foreground\n");
                exit(1);
            }
            else if (forked == 0){
                execv(command[0], command);
            }
            else{
                wait(NULL);
            }
        }
    }
    else{
        printf("Error: File does not exist or is not executable\n");
    }
}

void modeTwo(char** command){
    //start off testing path name with the current directory
    char* pathName = malloc(MAXBUF);
	getcwd(pathName, MAXBUF);

    strcat(pathName, "/");
    strcat(pathName, command[0]);

    if (access(pathName, F_OK | X_OK) == 0){
        //exec found in current directory - use modeOne to fork and exec
        command[0] = pathName;
        modeOne(command);
    }
    else{
        //exec not found in this directory - search other available paths

        char* pathOptions = malloc(MAXBUF);
        strcpy(pathOptions, getenv("PATH"));

        //tokenize PATH variable by colon
        char** pathTokens = split(pathOptions, ":");

        int i = 0;
        while (pathTokens[i] != NULL){
            char* thisPath = malloc(MAXBUF);

            //prep path to check
            strcpy(thisPath, "\0");
            strcat(thisPath, pathTokens[i]);
            strcat(thisPath, "/");
            strcat(thisPath, command[0]);

            if (access(thisPath, F_OK | X_OK) == 0) {
                command[0] = thisPath;
                modeOne(command);
                break;
            }

            free(thisPath);
            i++;
        }

        if(pathTokens[i] == NULL){
            //if we reached the end of our path options, then we failed to find a usable path, so print error message
            printf("Couldn't locate command\n");
        }
        
        free(pathOptions);
        free(pathTokens);
    }

    free(pathName);
}
