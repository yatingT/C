///////////////////////////////////////////////////////////////////////////////
// Main File:        sendsig.c
// This File:        sendsig.c
// Other Files:      (name of all other files if any)
// Semester:         CS 354 Spring 2021
//
// Author:           Yating Tian
// Email:            ytian83@wisc.edu
// CS Login:         yating
//
/////////////////////////// OTHER SOURCES OF HELP //////////////////////////////
//                   Fully acknowledge and credit all sources of help,
//                   other than Instructors and TAs.
//
// Persons:          Identify persons by name, relationship to you, and email.
//                   Describe in detail the the ideas and help they provided.
//
// Online sources:   Avoid web searches to solve your problems, but if you do
//                   search, be sure to include Web URLs and description of
//                   of any information you find.
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>


/**
 * main:
 * Main parses command line args, match the arguments with correct signal and
 * send signals to other programs by using their pid.
 */  
int main(int argc, char *argv[]) {

    //check the number of argument is correct
    if (argc != 3){
        printf("Usage: sendsig <signal type> <pid>\n");
        exit(1);
    }
    else{ //if the number of arguemnet is correct

        int sig = atoi(argv[2]); //store the pid 
        int sa = 0;
        if (strcmp(argv[1], "-i") == 0){ 
            sa = 2;
        }
        else if (strcmp(argv[1], "-u") == 0){
            sa = 10;
        }
        else{ //check if the signal is correct, otherwise print the error message
            printf("The correct signal type is -i and -u\n");//update
        }

        //check the kill function while calling it. 
        if (kill(sig, sa) != 0){
            printf("kill error in the sendsig.c\n");
            exit(1);
        }

    }

}
