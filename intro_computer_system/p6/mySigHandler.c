///////////////////////////////////////////////////////////////////////////////
// Main File:        mySigHander.c
// This File:        mySigHander.c
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
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

//define global variable 
int total_SIGUSR1 = 0;
int second = 3;

/**
 * handler_SIGALRM:
 * Signal handler for the SIGALRM signal, if the signal happend, we construct
 * the current time and pid to print out for every 3 seconds. 
 */ 
void handler_SIGALRM(){
    
    time_t curtime;
    if (time(&curtime) == ((time_t) -1)){
        printf("Error while calling time()"); 
        exit(1);
    }
    printf("PID: %i ", getpid());
    char* curr_time = ctime(&curtime);
    if (curr_time == NULL){
        printf("Error while calling ctime()"); 
        exit(1);
    }
    printf("CURRENT TIME: %s", curr_time);
    alarm(second);  

}

/**
 * handler_SIGINT:
 * Signal handler for the SIGINT signal, if the signal happend, we count the 
 * total SIGUSER1 and print out, and then exit the program.
 */ 
void handler_SIGINT(){

    printf("\nSIGINT handled.\n");
    printf("SIGUSR1 was handled %i times\n", total_SIGUSR1);
    exit(0);

}

/**
 * handler_SIGUSR1:
 * Signal handler for the SIGUSR1 signal, if the signal happend, we increase 
 * total SIGUSER1 by one and notify that we get the signal.
 */
void handler_SIGUSR1(){

    total_SIGUSR1++;
    printf("SIGUSR1 handled and counted!\n");

}

/**
 * main:
 * create the sigation and action, catch the sagation error and print the 
 * instruction about how to interact.  
 */
int main() {

    //set up the sigation and check their return value 
    struct sigaction act;
    struct sigaction act_user;
    struct sigaction act_int;
    memset (&act, 0, sizeof(act)); 
    memset (&act_user, 0, sizeof(act_user)); 
    memset (&act_int, 0, sizeof(act_int)); 

    act.sa_handler = handler_SIGALRM;
    act_user.sa_handler = handler_SIGUSR1 ;
    act_int.sa_handler = handler_SIGINT;

    if (sigaction(SIGALRM, &act, NULL) != 0){
        printf("Error while sigation SIGALRM");
        exit(1);
    }

    if (sigaction(SIGINT, &act_int, NULL) != 0){
        printf("Error while sigation SIGINT");
        exit(1);
    }

     if (sigaction(SIGUSR1, &act_user, NULL) != 0){
        printf("Error while sigation SIGUSR1");
        exit(1);
    }

    printf("PID and time print every 3 seconds.\n");
    printf("Type Ctrl-C to end the program.\n");

    alarm(second); 

    while (1){
    }

}