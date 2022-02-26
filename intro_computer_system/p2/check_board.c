///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2021 Deb Deppeler
// Posting or sharing this file is prohibited, including any changes/additions.
//
// We have provided comments and structure for this program to help you get 
// started.  Later programs will not provide the same level of commenting,
// rather you will be expected to add same level of comments to your work.
//
////////////////////////////////////////////////////////////////////////////////
// Main File:        check_board.c
// This File:        check_board.c
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
#include <stdlib.h>
#include <string.h>
     
char *DELIM = ",";  // commas ',' are a common delimiter character for data strings
     
/* COMPLETED (DO NOT EDIT):       
 * Read the first line of input file to get the size of that board.
 * 
 * PRE-CONDITION #1: file exists
 * PRE-CONDITION #2: first line of file contains valid non-zero integer value
 *
 * fptr: file pointer for the board's input file
 * size: a pointer to an int to store the size
 */
void get_board_size(FILE *fptr, int *size) {      
    char *line1 = NULL;
    size_t len = 0;
    if ( getline(&line1, &len, fptr) == -1 ) {
        printf("Error reading the input file.\n");
        exit(1);
    }

    char *size_chars = NULL;
    size_chars = strtok(line1, DELIM);
    *size = atoi(size_chars);
}



/* TODO:
 * Returns 1 if and only if the board is in a valid Sudoku board state.
 * Otherwise returns 0.
 * 
 * A valid row or column contains only blanks or the digits 1-size, 
 * with no duplicate digits, where size is the value 1 to 9.
 * 
 * Note: p2A requires only that each row and each column are valid.
 * 
 * board: heap allocated 2D array of integers 
 * size:  number of rows and columns in the board
 */
int valid_board(int **board, int size) {
    for (int i = 0; i < size; i++){
	    for (int j = 0; j < size; j++){
            if (*(*(board+i)+j) != 0){ //after check the current is not 0, compare current with the row
                for (int r = 0;r < size; r++){
                    if (*(*(board+i)+r) != 0&& r != j){
                        if (*(*(board+i)+j) == *(*(board+i)+r)){
                            return 0;
                        }

                    }
                    
                }

                for (int c = 0; c < size; c++){ //after check the current is not 0, compare current with the column
                    if (*(*(board+c)+j) != 0 && c != i){
                        if((*(*(board+i)+j) == *(*(board+c)+j))){
                            return 0;
                        }
                    }
                }
            }
        }
     }

     if ( *(*(board+0)+0) != 0 &&*(*(board+(size-1))+(size-1)) != 0 ) {
         if ( *(*(board+0)+0) == *(*(board+ (size-1))+ (size-1)) ){
             return 0;
         }
     }
    if ( *(*(board+ (size-1))+0) != 0 && *(*(board+0)+ (size-1)) != 0 ){
        if ( *(*(board + (size-1))+0) == *(*(board+0)+ (size-1)) ) {
            return 0;
        }
    } 

    return 1;
   }  
  
 
   
/* TODO: COMPLETE THE MAIN FUNCTION
 * This program prints "valid" (without quotes) if the input file contains
 * a valid state of a Sudoku puzzle board wrt to rows and columns only.
 *
 * argc: CLA count
 * argv: CLA value
 */
int main( int argc, char *argv[] ) {              
     
    // TODO: Check if number of command-line arguments is correct.
    if (argc < 2) {
        printf("missing argument\n");
        exit(-1);
        }
    if (argc > 2) {
        printf("too much argument\n");
        exit(-1);
        }

    // Open the file and check if it opened successfully.
    FILE *fp = fopen(*(argv + 1), "r");
    if (fp == NULL) {
        printf("Can't open file for reading.\n");
        exit(1);
    }

    // Declare local variables.
    int size;

    // TODO: Call get_board_size to read first line of file as the board size.
    get_board_size(fp, &size);

    if (size<1 || size>9){
         printf("invalid\n");
         exit(1);
    }

    // TODO: Dynamically allocate a 2D array for given board size.

    int **board = malloc (sizeof(int *) * size);  
    if( board == NULL ){
        printf("malloc error while assign array.\n");
        exit(1);
    }
    for (int i=0;i< size;i++){
        *(board+i)=malloc(sizeof(int) * size);
         if( *(board+i) == NULL ){
            printf("malloc error while assign row.\n");
            exit(1);
            }
    }

    // Read the rest of the file line by line.
    // Tokenize each line wrt the delimiter character 
    // and store the values in your 2D array.
    char *line = NULL;
    size_t len = 0;
    char *token = NULL;
    for (int i = 0; i < size; i++) {

        if (getline(&line, &len, fp) == -1) {
            printf("Error while reading line %i of the file.\n", i+2);
            exit(1);
        }

        token = strtok(line, DELIM);
        for (int j = 0; j < size; j++) {
            // TODO: Complete the line of code below
            // to initialize your 2D array.
            /* ADD ARRAY ACCESS CODE HERE */ 
           
            *(*(board+i)+j) = atoi(token);
            token = strtok(NULL, DELIM);
        }
    }

    // TODO: Call the function valid_board and print the appropriate
    //       output depending on the function's return value.
    int answer = valid_board(board, size); 

    // TODO: Free all dynamically allocated memory.
    for (int i = 0; i < size; i++){
        free(*(board + i));
    }
    free(board);

    //Close the file.
    if (fclose(fp) != 0) {
        printf("Error while closing the file.\n");
        exit(1);
    } 
     
    if (answer == 1){
        printf("valid\n");
    }else{
        printf("invalid\n");
    }

    return 0;       
}       






// s21

