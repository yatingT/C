#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>



//a structure that holds input argument to pass to a thread
typedef struct thread_args{
  char* buffer; //a pointer to the buffer
  size_t start_index; //start index for the working thread
  size_t end_index; // end index for the working thread
} thread_args_t;

//check if a file exist (taken from https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-14-17-c)
int exist(const char *name)
{
  struct stat   buffer;
  return (stat (name, &buffer) == 0);
}

//the size of input buffer, could change to smaller
#define MAX_COUNT 1024*1024*1024 //defines the max size of buffer

//structure to hold the computed zipped result by a thread
typedef struct zipped_args{
  char my_char[MAX_COUNT]; //keeps track of the characters
  int my_count[MAX_COUNT]; //keep tracks of the counts of corresponding character
  int length; // size of zipped
} zipped_arr_t;


//a helper method to debud (we did not like hexdump)
void print_helper(zipped_arr_t * addr){
    for (int i=0;i<addr->length;i++){
      printf("The char at location %d is %c\n",i,(addr->my_char)[i]);
      printf("The count at location %d is %d\n",i,(addr->my_count)[i]);
    }
}

size_t num_thread; //holds the number of threads


//init the output array
zipped_arr_t * zipped; 
size_t zipped_size = MAX_COUNT; 


//check if the input file name is valid


//this function opend file, and zip the content, and write to a zipped_arr_t structure
void * mythread(thread_args_t * my_input){
        
        char * raw_slice=my_input-> buffer; //pointer to a the buffer that thread is working on
        int start=my_input->start_index;//start index for the thread
        int end=my_input->end_index;//end index for the thread
        char current=raw_slice[start];//holds the current character
        int c=start;//a counter to handle sparse chars
  
        while (current=='\0'){
            c++;
            current=raw_slice[c];
        }
	
        int count = 0;
        zipped_arr_t * zipped_tr = (zipped_arr_t *)malloc(sizeof(zipped_arr_t));//initializes memory for output of the thread
        zipped_tr->length=0; //initializes the length

        //loop over 
        for(size_t i=start;i<end;i++){
	  char next=raw_slice[i]; //holds the next char for comparing 
            if ((next=='\0') && (i!=end-1)){
                continue;
            }

            //if the current char is the last one in the buffer, store current and break
            if ( i==end-1){
                (zipped_tr->my_char)[zipped_tr->length]=current;
                (zipped_tr->my_count)[zipped_tr->length]=count;
                zipped_tr->length++;
                break;
            }
            //if same char, increase count
            else if(next==current){
                count++;
            //if different char, store the current count and update the next
            }else{
                //store
                (zipped_tr->my_char)[zipped_tr->length]=current;
                (zipped_tr->my_count)[zipped_tr->length]=count;
                zipped_tr->length++;
                current=next;
                count=1;
            }

        }
    return (void *)zipped_tr;
}



//MAIN program starts

int main( int argc, char *argv[] ) {

  //Handling file arguments
    if(argc == 1) {
		printf("pzip: file1 [file2 ...]\n");
		exit(1);
	}
    char **proc_argv;
    proc_argv=malloc(argc * sizeof(char*));
    for (int i = 0; i < argc-1; i++)
        {proc_argv[i] = malloc((100) * sizeof(char));}
    int file_counter=0;
    //FILE * fp;
    for (int i=1;i<argc;i++){
        
        if (exist(argv[i])){
            proc_argv[file_counter]=argv[i];

            file_counter++;

        }

    
    }


    
    int num_tr_over_all_files=3;
    int num_files=file_counter;//holds number of files (assuming all vaid)
    int nf=0;//holds the number of files (removing the invalid ones)


     int max_num_tr_per_file=100;
     size_t file_chunk=10000;
     pthread_t p[max_num_tr_per_file][num_tr_over_all_files]; //2d array of threads
     void *rv [max_num_tr_per_file][num_tr_over_all_files];//2d array of return values
     zipped_arr_t * rv_casted[max_num_tr_per_file][num_tr_over_all_files]; //casted array of return values
     thread_args_t my_input[max_num_tr_per_file][num_tr_over_all_files]; //holds the input for threads
     (void)p;
     (void) rv;
     (void) rv_casted;
     (void) my_input;
     (void) file_chunk;


    
    int * num_tr_per_file=malloc(num_tr_over_all_files * sizeof(int));
    //

    //hold the content of file
    
    char * raw[num_tr_over_all_files]; //pointer to the mapped file  
    zipped_arr_t * frv[num_files]; //an array  containes the results from files

    //allocating memory for the file outputs
    for (int i=0;i< num_files;i++){
         frv[i]=(zipped_arr_t *)malloc(sizeof(zipped_arr_t));
     }

    

    //looping over files SERIAL LOOP
    //for (int i=0; i<num_files;i++){
    int serial=0;
    while (serial<num_files){
        //handles the edge case
        if (num_files-serial< num_tr_over_all_files){
            num_tr_over_all_files=num_files-serial;
        }



        size_t filesize [num_tr_over_all_files];
        size_t tr_size [num_tr_over_all_files];

        for (int j=0;j<num_tr_over_all_files;j++){    
                    //maps the file into the buffer 
                char * fname= strdup(proc_argv[serial+j]); //holds the file name //FIXED
                raw[j]=NULL;//initializes the buffer pointer
                filesize[j]=0; //holds the file size
                


                //checks if file is accesible
                if(access(fname, F_OK ) == 0 ){ 

                        //use fstat to get the input file size
                    int fd; 
                        fd=open(fname,__O_LARGEFILE);
                        struct stat finfo;
                        fstat(fd, &finfo);
                        filesize[j] = finfo.st_size;//filesize
                            //map the input the file, put the content to a buffer
                        raw[j] = mmap(NULL, filesize[j], PROT_READ, MAP_PRIVATE, fd, 0);
                        if(raw[j] == MAP_FAILED) {
                            fprintf(stderr, "error when mmap file\n");
                            exit(0);
                    }        
                }
      

        int cores=get_nprocs_conf();
        if (cores>10){
		   num_tr_per_file[j]=10;
		 }
		 else {
             num_tr_per_file[j]=cores;
         } 
    
            tr_size[j]=filesize[j]/num_tr_per_file[j];//holds the thread size
   
            
            //initializes the input for threads
            for (size_t i=0;i<num_tr_per_file[j];i++){
                my_input[i][j].buffer=raw[j];
                my_input[i][j].start_index=i*tr_size[j];
                if (i==num_tr_per_file[j]-1){
                    my_input[i][j].end_index=filesize[j]+1;
                } else{
                    my_input[i][j].end_index=((i+1)*tr_size[j])+1;
                }
            }

            //allocates memory for return values of each thread
            for (int i=0;i< num_tr_per_file[j];i++){
                rv[i][j]=(zipped_arr_t *)malloc(sizeof(zipped_arr_t));
            }
        
        }       /////////////////////////////////////////////////////////////////////////////////////////////changed the bracket here


        //creates threads 
        for (int j=0;j<num_tr_over_all_files;j++){
            for (int i=0;i<num_tr_per_file[j];i++){
            
                pthread_create(&p[i][j],NULL,(void *)mythread,&my_input[i][j]);
            }
        }

        
        //waits for threads to complete
        for (int j=0;j<num_tr_over_all_files;j++){
            for (int i=0;i<num_tr_per_file[j];i++){
            
                pthread_join(p[i][j],(void **)&rv[i][j]);
            }
        }   

        //casts the return values
        for (int j=0;j<num_tr_over_all_files;j++){
            for (int i=0;i< num_tr_per_file[j];i++){
            
                rv_casted[i][j]=(zipped_arr_t *) rv[i][j];
	    //helped us for debugging
            //print_helper( rv_casted[i]);
	    //printf("Thread %d is done\n",i);
            }
        }


        
        //fixes the edge case computation over chunks of each file
        for (int j=0;j<num_tr_over_all_files;j++){
            for (int i=0;i<num_tr_per_file[j];i++){
                rv_casted[i][j]=(zipped_arr_t *) rv[i][j];
                if ((i>0) &&  ((rv_casted[i][j]->my_char)[0]== rv_casted[i-1][j]->my_char[rv_casted[i-1][j]->length -1]) ){
                    (rv_casted[i][j]->my_count)[0]= (rv_casted[i][j]->my_count)[0]+(rv_casted[i-1][j]->my_count)[rv_casted[i-1][j]->length -1];
                    rv_casted[i-1][j]->length--;
                }

            }
        } 

        
        
        
        //combines the work of threads into one zipped_arr_T  (frv[nf])
        for (int k=0; k<num_tr_over_all_files;k++){
            int my_counter=0;
            for (int i=0;i<num_tr_per_file[k];i++){
                for (int j=0;j<rv_casted[i][k]->length;j++){
                    (frv[nf]->my_char)[my_counter]=(rv_casted[i][k]->my_char)[j];
                    (frv[nf]->my_count)[my_counter]=(rv_casted[i][k]->my_count)[j];
                    my_counter++;
                }

            }
            frv[nf]->length=my_counter;

                if ((nf>0) &&  ((frv[nf]->my_char)[0]== frv[nf-1]->my_char[frv[nf-1]->length -1]) ){
                    (frv[nf]->my_count)[0]= (frv[nf]->my_count)[0]+(frv[nf-1]->my_count)[frv[nf-1]->length -1];
                    frv[nf-1]->length--;
                }
                nf++;
                munmap(raw[k],filesize[k]);
            }
        serial=serial+num_tr_over_all_files;

    }
    for (int i=0;i<nf;i++){
        //print_helper(frv[i]);
        for (int j=0;j<frv[i]->length;j++){
            fwrite(&((frv[i]->my_count)[j]),sizeof(int),1,stdout);
            fwrite(&((frv[i]->my_char)[j]),sizeof(char),1,stdout);
            }
    }
 
    return 0;

 
}
