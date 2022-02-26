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


typedef struct thread_args{
char* buffer;
size_t start_index;
size_t end_index;
} thread_args_t;



//the size of input buffer, could change to smaller
#define MAX_COUNT 1024*1024*1024
typedef struct zipped_args{
char my_char[MAX_COUNT];
int my_count[MAX_COUNT];
int length;
} zipped_arr_t;

void print_helper(zipped_arr_t * addr){
    for (int i=0;i<addr->length;i++){
      printf("The char at location %d is %c\n",i,(addr->my_char)[i]);
      printf("The count at location %d is %d\n",i,(addr->my_count)[i]);
    }
}
size_t num_thread;

//init the output array
zipped_arr_t * zipped; 
size_t zipped_size = MAX_COUNT;
//check if the input file name is valid


//this is the function called by thread
//this function opend file, and zip the content, and write to a new output file
void * mythread(thread_args_t * my_input){
        
        char * raw_slice=my_input-> buffer;
        int start=my_input->start_index;
        int end=my_input->end_index;
        char current=raw_slice[start];
        int c=start;
        while (current=='\0'){
            c++;
            current=raw_slice[c];
        }
        int count = 0;
        zipped_arr_t * zipped_tr = (zipped_arr_t *)malloc(sizeof(zipped_arr_t));
        zipped_tr->length=0;

        //loop over 
        for(size_t i=start;i<end;i++){
            char next=raw_slice[i];
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


int main( int argc, char *argv[] ) { 
    if(argc == 1) {
		printf("pzip: file1 [file2 ...]\n");
		exit(1);
	}
    int num_tr_per_file=get_nprocs();
    //printf("NUM CORES:%d", num_tr_per_file);
    int num_files=argc-1;
    int nf=0;

    //hold the content of file
    char * raw; 
    zipped_arr_t * frv[num_files]; //containes the results from files 
    for (int i=0;i< num_files;i++){
         frv[i]=(zipped_arr_t *)malloc(sizeof(zipped_arr_t));
     }

    for (int i=0; i<num_files;i++){
        
        
        //maps the file into the buffer 
        char * fname= strdup(argv[i+1]);
        raw=NULL;
        size_t filesize=0;
        // size_t size=0;
        if(access(fname, F_OK ) == 0 ){ 

            //use fstat to get the input file size
            int fd;
            
            // FILE * fp=fopen(fname,"r");
            fd=open(fname,__O_LARGEFILE);
            struct stat finfo;
            fstat(fd, &finfo);
            filesize = finfo.st_size;
            // raw=malloc(filesize);
            //map the input the file, put the content to a buffer
            raw = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, fd, 0);
            // fseek(fp, 0, SEEK_SET);
            // size=fread(raw, filesize, 1, fp);
            if(raw == MAP_FAILED) {
                fprintf(stderr, "error when mmap file\n");
                exit(0);
            // fclose(fp);
        }        
        }

        // if (size==0){
        //     continue;
        // }



        // Need to divide the buffer into num_tr_per_file
        //For Each Chunk, figur-out what are the start and end index
        //Create out put for each thread
        //Joint the output and write it into the file
        //size_t raw_1_len=filesize/num_tr_per_file;
        size_t tr_size=filesize/num_tr_per_file;
        //printf("I am size: %ld\n",tr_size);
         if (raw==NULL){
             continue;
         }
        //pthread_t p;
        //thread_args_t my_input_1;
        pthread_t p[num_tr_per_file]; //array of threads
        void *rv [num_tr_per_file];//[num_tr_per_file]; //array of return values
        zipped_arr_t * rv_casted[num_tr_per_file]; //casted array of return values
        thread_args_t my_input[num_tr_per_file]; //holds the input for thread
        
        //initializes the input for threads
        for (int i=0;i<num_tr_per_file;i++){
            my_input[i].buffer=raw;
            my_input[i].start_index=i*tr_size;
            if (i==num_tr_per_file-1){
                my_input[i].end_index=filesize+1;
            } else{
                my_input[i].end_index=((i+1)*tr_size)+1;
            }
        }

        //allocates memory for return values of each thread
        for (int i=0;i< num_tr_per_file;i++){
            rv[i]=(zipped_arr_t *)malloc(sizeof(zipped_arr_t));
        }



        //creates threads 
        for (int i=0;i<num_tr_per_file;i++){
        pthread_create(&p[i],NULL,(void *)mythread,&my_input[i]);
        }

        //waits for threads to complete
        for (int i=0;i<num_tr_per_file;i++){
        pthread_join(p[i],(void **)&rv[i]);
        }   

        //casts the return values
        for (int i=0;i< num_tr_per_file;i++){
            rv_casted[i]=(zipped_arr_t *) rv[i];
            //print_helper( rv_casted[i]);
            //printf("Thread %d is done\n",i);
        }
        
        //fixes the edge case computation
        for (int i=0;i<num_tr_per_file;i++){
            rv_casted[i]=(zipped_arr_t *) rv[i];
            if ((i>0) &&  ((rv_casted[i]->my_char)[0]== rv_casted[i-1]->my_char[rv_casted[i-1]->length -1]) ){
                (rv_casted[i]->my_count)[0]= (rv_casted[i]->my_count)[0]+(rv_casted[i-1]->my_count)[rv_casted[i-1]->length -1];
                rv_casted[i-1]->length--;
            }

        } 
        
        
        //combines the work of threads into one zipped_arr_T  (frv[nf])
        //frv[nf]=(zipped_arr_t *) rv;
        int my_counter=0;
        for (int i=0;i<num_tr_per_file;i++){
            for (int j=0;j<rv_casted[i]->length;j++){
                (frv[nf]->my_char)[my_counter]=(rv_casted[i]->my_char)[j];
                (frv[nf]->my_count)[my_counter]=(rv_casted[i]->my_count)[j];
                my_counter++;
            }

        }
        frv[nf]->length=my_counter;


        // for (int i=0;i<num_tr_per_file;i++){
        //     //print_helper(frv[i]);
        //     //print_helper( rv_casted[i]);
        //     for (int j=0;j<rv_casted[i]->length;j++){
        //         //print_helper( )
        //         fwrite(&((rv_casted[i]->my_count)[j]),sizeof(int),1,stdout);
        //         fwrite(&((rv_casted[i]->my_char)[j]),sizeof(char),1,stdout);
        //     }
        // } 



        //handling edge case between files
        if ((nf>0) &&  ((frv[nf]->my_char)[0]== frv[nf-1]->my_char[frv[nf-1]->length -1]) ){
            (frv[nf]->my_count)[0]= (frv[nf]->my_count)[0]+(frv[nf-1]->my_count)[frv[nf-1]->length -1];
            frv[nf-1]->length--;
        }
        nf++;
        munmap(raw,filesize);
    }
    for (int i=0;i<nf;i++){
        //print_helper(frv[i]);
        for (int j=0;j<frv[i]->length;j++){
            fwrite(&((frv[i]->my_count)[j]),sizeof(int),1,stdout);
            fwrite(&((frv[i]->my_char)[j]),sizeof(char),1,stdout);
            }
    }
 
    // fwrite(&x,sizeof(int),1,fp);
    // fwrite(&y,sizeof(int),1,fp);
    // fclose(fp);
    return 0;

 
}
