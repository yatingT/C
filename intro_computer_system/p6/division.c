///////////////////////////////////////////////////////////////////////////////
// Main File:        division.c
// This File:        division.c
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

//count the total amount of division 
int count = 0 ; 

/**
 * handler_SIGINT:
 * Signal handler for the SIGINT signal, if the signal happend, we print the 
 * total number of operations completed successfully and then exit the program.
 */ 
void handler_SIGINT(){

    printf("\nTotal number of operations completed successfully: %i \n",count);
    printf("The program will be terminated.\n");
    exit(0);
}

/**
 * handler_SIGFPE:
 * Signal handler for the SIGFPE signal, if the signal happend, we print out the 
 * error message, and print the total number of operations completed
 * successfully,  and then exit the program.
 */ 
void handler_SIGFPE(){

    printf("Error: a division by 0 operation was attempted.\n");
    printf("Total number of operations completed successfully: %i \n",count);
    printf("The program will be terminated.\n");
    exit(0);
}

/**
 * main:
 * create the sigation and action, catch the sagation error and print the 
 * instruction about how to interact between the user and program. After 
 * getting two differnt input from user, print out the division result and
 * the remainder. Meanwhile, handle the divide-by-zero error. 
 */
int main() {

    //set up the sigation for the SIGINT
    struct sigaction act_int;
    memset (&act_int, 0, sizeof(act_int)); 
    act_int.sa_handler = handler_SIGINT;
    if (sigaction(SIGINT, &act_int,NULL) != 0){
        printf("Error in sigation of SIGINT in division file");
        exit(1);
    }

    struct sigaction act_fpe;
    memset (&act_fpe, 0, sizeof(act_fpe)); 
    act_fpe.sa_handler = handler_SIGFPE;
    if (sigaction(SIGFPE, &act_fpe,NULL) != 0){
        printf("Error in sigation of SIGFPE in division file");
        exit(1);
    }


    int buffer = 100;
    while (1){

        //get the user input 
        char str1[buffer];
        char str2[buffer];
        printf("Enter first integer: ");
        if (fgets(str1, buffer, stdin) == NULL) {
            printf("first fgets return a NULL");
            exit(1);
        }
        int first = atoi(str1);

        printf("Enter second integer: ");
        if (fgets(str2, buffer, stdin) == NULL) {
            printf("second fgets return a NULL");
            exit(1);
        }
        int second = atoi(str2);

        //get result and remainder and print
        int result = first / second;
        count++;
        int remainder = first % second;
        printf("%i / %i is %i with a remainder of %i\n", first, second, result, remainder);

    }

}