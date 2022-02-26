///////////////////////////////////////////////////////////////////////////////
//
// Copyright 2020 Jim Skrentny
// Posting or sharing this file is prohibited, including any changes/additions.
// Used by permission, CS 354 Spring 2021, Deb Deppeler
//
////////////////////////////////////////////////////////////////////////////////
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
// Main File:        myMagicSquare.c
// This File:        myMagicSquare.c
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

// Structure that represents a magic square
typedef struct {
    int size;           // dimension of the square
    int **magic_square; // pointer to heap allocated magic square
} MagicSquare;

/* 
 * Prompts the user for the magic square's size, reads it,
 * checks if it's an odd number >= 3 (if not display the required
 * error message and exit), and returns the valid number.
 */
int getSize() {
    int n=0;
    printf("Enter magic square's size (odd integer >=3)\n");
    scanf("%d", &n);  //get the input size

    if ( n % 2 != 1 ){ //check the odd/even
        printf("Magic square size must be odd\n");
        exit(1);
    } 
    if ( n < 3 ){ //check if it big enough
        printf("Magic square size must  be >=3\n");
        exit(1);
    }
    
     
    return n;   
} 
   
/* TODO:
 * Makes a magic square of size n using the alternate 
 * Siamese magic square algorithm from assignment and 
 * returns a pointer to the completed MagicSquare struct.
 *
 * n the number of rows and columns
 */
MagicSquare *generateMagicSquare(int n) {

    MagicSquare *ms = malloc(sizeof(MagicSquare)); //init the magic square
    if( ms == NULL ){
        printf("malloc error while assign magic square.\n");
        exit(1);
    }

    ms->size=n;
    ms->magic_square = malloc (sizeof(int *) * ms->size); //init the magic square pointer in the heap
    if( ms->magic_square == NULL ){
        printf("malloc error while assign array.\n");
        exit(1);
    }
    for (int i = 0; i < ms->size  ;i++){ 
        *(ms->magic_square+i) = malloc(sizeof(int) * ms->size ); // init the int pointers for the 2d array
         if( *(ms->magic_square+i) == NULL ){
            printf("malloc error while assign row.\n");
            exit(1);
            }
    }

    for(int i = 0; i < ms->size ; i++){ //set the square to 0
	    for (int j = 0; j < ms->size ; j++){
	      *(*(ms->magic_square + i) +j ) = 0;
        }
      }

    int x = ms->size  / 2; //set start index
    int y = ms->size - 1;
    for (int num = 1 ; num < ms->size * ms->size +1 ;){ //fill the number in
        //start with check the position
        if (x == -1 && y == ms->size){  //up right corner case
            x = 0;
            y = ms->size - 2;
        } else {
            if (x < 0) { // top boundary case
                x = ms->size - 1;  
            }
            if (y == ms->size ){ // right boundry case 
                y = 0;
            } 
        } 
        //then check if the target position has number already
        if ( *(*(ms->magic_square + x) + y)!= 0 ) { //if the target position has number, put the number to the left
            x += 1;   
            y -= 2;
            continue;
        } else { 
            *(*(ms->magic_square + x) +y )= num++; //if the target position is empty,fill the number in
        }
        x -= 1;
        y += 1;
    }
    return ms;    
} 

/*
 * Opens a new file (or overwrites the existing file)
 * and writes the square in the specified format.
 *
 * magic_square the magic square to write to a file
 * filename the name of the output file
 */
void fileOutputMagicSquare(MagicSquare *magic_square, char *filename) {

    FILE *ofp = fopen(filename ,"w"); //open file and check
    if (ofp == NULL){
        fprintf(stderr, "can't open output file %s!\n", filename);
        exit(1);
    }

    fprintf(ofp, "%d\n", magic_square->size); //print the size of suqre 


    //print the squre 
    for ( int i = 0; i < magic_square->size; i++) {
        for ( int j = 0; j < magic_square->size; j++){
            if (j==  magic_square->size-1){
                fprintf( ofp,"%d", *(*(magic_square->magic_square + i) +j ));
            } else{
                fprintf( ofp,"%d,", *(*(magic_square->magic_square + i) +j ));
            }
        }
        fprintf(ofp, "\n");
    }
    //close the file
    if (fclose(ofp) != 0) {
        printf("Error while closing the file.\n");
        exit(1);
    } 

}

/* 
 * Generates a magic square of the user specified size and
 * output the quare to the output filename
 */
int main(int argc, char *argv[]) {

    //  Check input arguments to get output filename
    if (argc != 2){
        printf("Usage: ./myMagicSquare <output_filename>\n");
        exit(1);
    } else {
        FILE *ofp = fopen(*(argv+1) ,"w"); 
        if (ofp == NULL){
        fprintf(stderr, "can't open output file %s!\n", *(argv+1));
        exit(1);
        }
    }

    char *filename = *(argv+1);  

    // Get magin square's size from user
    int n = getSize();
    //  Generate the magic square
    MagicSquare *square = generateMagicSquare(n);
    square->size = n;

    // Output the magic square
    fileOutputMagicSquare(square, filename);

    //free space 
    for (int i = 0; i < square->size; i++){
        free(*(square->magic_square + i));
    }
    free(square->magic_square);
    free(square);
    return 0;
} 






                                                         
//		s21		myMagicSquare.c      

