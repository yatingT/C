
#include "udp.h"

#define BUFFER_SIZE (1000)

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "mfs.h"
#include <stdlib.h>
#include <string.h>


//SIZES
  int check_point_size=1028; //holds check_point_region size
  int block_size=4096; //holds block size
  int inode_size=64;  //holds the size of the inode
  int inode_piece_size=64; //holds the inode piece size
  int dir_entry_size=32; //hold the size of directory entry
  int max_dir_entry_num=128; //holds the max dir entry number
  int num_inodes_index=0;
  int real_num_inodes_index=0;
  
  char null_char='\0'; //holds default value for null-char
  int null_int=-1; //holds default value for a non-set four bytes
//BLOCK SIZE=4096 bytes

//CR size on disk 256*4+4=1028
struct CR{
  int  end_log;
  int  inode_map[256];
};

//Inode size on disk is 4+4+14*4=64
struct Inode{
int size;
int type;//0 for directory; 1 for regular  
int block_addr[14];
};
int default_int=-1;
char * default_str="AAAAA";
//Inode piece 
void set_me (MFS_message_t * msg){
    msg->re=default_int;
    strcpy(msg->buffer,default_str);
    msg->type=default_int;
    msg->block=default_int;
    msg->inode_num=default_int;
}

// a helper function to print the contents of CR
void CR_print(struct CR * my_CR){
    printf("----------------------CheckPointRegion----------------------\n");
    
    for (int i=0;i<256;i++){
      printf("The in-disk location of %d-th inode piece is: %d\n",i,my_CR->inode_map[i]);
    }
    printf("The in-disk location of last byte of log is %d\n", my_CR->end_log);
  printf("----------------------CheckPointRegion----------------------\n");

}

void CR_region_print(int fd){
  lseek(fd,0,SEEK_SET);
  int * value_adr=(int *) malloc(sizeof(int));
  
  printf("------------------------CR_Region----------------------------------\n");
  for (int i=0;i<1028;i=i+4){
    read(fd,value_adr,4);
    printf("at location %d : %d\n",i,*value_adr);
  } 
  printf("------------------------CR_Region----------------------------------\n");
  free(value_adr);
}


//a helper function to print content of an inode piece
void Inode_Piece_print(int fd, int piece_adr){
  printf("-----------------Inode Piece Info-------------\n");
  lseek(fd,piece_adr,SEEK_SET);
  int * value_adr=(int *) malloc(sizeof(int));
  for (int i=0;i<inode_piece_size;i=i+4){
    read(fd,value_adr,4);
    printf("element %d: %d\n",i,*value_adr);

  }
  printf("-----------------Inode Piece Info-------------\n");
  free(value_adr);
}

// a helper function to print content of a directory block
void Dir_block_print(int fd, int block_adr){

printf("-----------------Directory Block-------------\n");
lseek(fd,block_adr,SEEK_SET);
int * value_adr=(int *) malloc(sizeof(int));
char * name_adr=(char *) malloc(28);
for (int i=0;i<max_dir_entry_num;i++){
  read(fd,value_adr,4);
  read(fd,name_adr,28);
  
  printf("Entry %d name: %s\n",i, name_adr);
  printf("Entry %d inode number: %d\n",i, *value_adr);
}

printf("-----------------Directory Block-------------\n");
free(value_adr);
free(name_adr);
}
//a helper function to print the content of an inode
void Inode_print(int fd,int inode_addr){
  printf("-----------------Inode Info-------------\n");
   lseek(fd,inode_addr,SEEK_SET);
   int * my_int=(int *) malloc(sizeof(int));
   read(fd,my_int,4);
   printf("Inode size: %d\n",*my_int);
  read(fd,my_int,4);
   printf("Inode type: %d\n",*my_int);
   for (int i=0;i<14;i++){
       read(fd,my_int,4);
       printf("The block %i loc is: %d\n",i,*my_int);
   }
   free(my_int);
  printf("-----------------Inode Info-------------\n");
}

// a helper function to update the number of bytes in the check point region
void CR_set_log_end(struct CR * my_CR, int num_bytes){
  my_CR->end_log=my_CR->end_log+num_bytes;
  //printf("%d\n",my_CR->end_log);

}

// a helper function that updates the checkpoint size value
void set_check_point_size_value(int fd, struct CR * my_CR, int check_point_size){
  lseek(fd,check_point_size-4,SEEK_SET);
  write(fd,&my_CR->end_log,4);
}

// a helper function to set inode values in the disk
void inode_info_set(int fd, int inode_addr, int * inode_size, int * inode_type){
     lseek(fd,inode_addr,SEEK_SET);
     write(fd,inode_size,4);
     write(fd,inode_type,4);
}



// int write_void(int num_bytes, int start_index, char * my_char){
//     int fd=open("my_file_system_image",O_CREAT|O_RDWR,0666);
//     lseek(fd,start_index,SEEK_SET);
//      for (int i=0;i<num_bytes;i++){
//         write(fd,my_char,1);
//      }
//      close(fd);
// }
// int a=1;


int file_system_boot_up(int fd, struct CR* my_CR){

  //sets the my_CR values
  my_CR->end_log=0;
  for (int i=0;i<256; i++){
    my_CR->inode_map[i]=-1;
  }
  //initialize CheckRegion
  lseek(fd,0,SEEK_SET);

  for (int i=0;i<check_point_size;i=i+4){
    write(fd,&null_int,4);
  }
  
  //updates log_end
  CR_set_log_end(my_CR, check_point_size);

  //updates checkpoint size value
  set_check_point_size_value(fd, my_CR,check_point_size);
  
  //CR_print(my_CR);
  
  //add a block
   for (int i=0;i<block_size;i++){
     write(fd,&null_char,1);
   }
   CR_set_log_end(my_CR,block_size);
   set_check_point_size_value(fd,my_CR,check_point_size);
  
  //CR_print(my_CR);
  
  
  //add an inode
  num_inodes_index=0;
  real_num_inodes_index=0;
  lseek(fd,my_CR->end_log,SEEK_SET);
   for (int i=0;i<inode_size;i++){
     write(fd,&null_char,1);
   }

   //fill the inode
   int * inode_size_1=(int *) malloc(sizeof(int));
   int * inode_type=(int *) malloc(sizeof(int));
   *inode_size_1=1;
   *inode_type=0; //0 is directory
   inode_info_set(fd,my_CR->end_log,inode_size_1,inode_type);
  
  //updates log_end
  CR_set_log_end(my_CR, inode_size);
  set_check_point_size_value(fd,my_CR,check_point_size);

  
  //add inode piece
  lseek(fd,my_CR->end_log,SEEK_SET);
   for (int i=0;i<inode_piece_size;i=i+4){
     write(fd,&null_int,4);
   }

  
  //updates log_end
  CR_set_log_end(my_CR, inode_piece_size);
  set_check_point_size_value(fd,my_CR,check_point_size);

  // //fill inode piece
  // for (int i=0;i<inode_piece_size;i=i+4){
  //    write(fd,&null_int,4);
  // }
  
  //set the ipiece location in CR
  lseek(fd,0,SEEK_SET);
  int * inode_piece_addr=(int *) malloc(sizeof(int));
  *inode_piece_addr=1028+4096+64;
  write(fd,inode_piece_addr,4);
  my_CR->inode_map[0]=*inode_piece_addr;

  //set the inode value
  lseek(fd,*inode_piece_addr,SEEK_SET);
  int* inode_addr=(int *)malloc(sizeof(int));
  *inode_addr=1028+4096;
  write(fd,inode_addr,4);
  
  //set the inode block addr
  lseek(fd,*inode_addr+8,SEEK_SET);
  // printf("I am the inode adr from bootup: %d\n",*inode_addr+8);
  int * block_addr=(int *) malloc(sizeof(int));
  *block_addr=1028;
  // printf("I am the block adr from bootup: %d \n",*block_addr);
  write(fd,block_addr,4);
  //set other addr to -1 
  for (int i=1;i<14;i++){
    lseek(fd,*inode_addr+8+4*i,SEEK_SET);
    write(fd,&null_int,4);
  }

  //set root directory entry
  lseek(fd,*block_addr,SEEK_SET);
  char* current =(char *) malloc(28);
  strcpy(current,".");

  int* root_inode=(int *) malloc(sizeof(int));
  *root_inode=0;
 
  write(fd,root_inode,4);
  write(fd,current,28);

  lseek(fd,(*block_addr)+32,SEEK_SET);
  char* parent=(char *) malloc(28);
  strcpy(parent,"..");
  
  write(fd,root_inode,4);
  write(fd,parent,28);

  free(inode_size_1);
  free(inode_type);
  free(parent);
  free(block_addr);
  return 0;
}

int inode_Lookup(int fd, struct CR* my_CR,int pinum, char *name){

  //Update data block of the parent directoy
  int * parent_inode_adr=(int *) malloc(sizeof(int)); //hold loacation of the parent inode addr
  int parent_CR_index=pinum/16;
  int parent_piece_index=pinum % 16;
  //printf("Parent directory CR index: %d\n",parent_CR_index);
  //printf("parent directory piece index: %d\n",parent_piece_index);

  
  int * parent_inode_piece_loc_adr=(int *) malloc(sizeof(int)); //holds location of the parent inode piece
  lseek(fd,parent_CR_index*4,SEEK_SET);
  read(fd,parent_inode_piece_loc_adr,4);
  lseek(fd,*parent_inode_piece_loc_adr+4*parent_piece_index,SEEK_SET);
  read(fd,parent_inode_adr,4);
  // printf("parent inode addr: %d\n",*parent_inode_adr);
  //go to parent data block and update it
  lseek(fd,(*parent_inode_adr)+4,SEEK_SET);

  //finds the parent inode available datablock
  int* inode_type_adr=(int*) malloc(sizeof(int));
  read(fd,inode_type_adr,4);
  // if (*inode_type_adr==1){
  //   return -1;
  // }
  lseek(fd,*parent_inode_adr+8,SEEK_SET);
  //search in a block
  for (int i=0;i<14;i++){
    int* my_block_loc_adr=(int *) malloc(sizeof(int));
    // printf("parent inode addr: %d\n",*parent_inode_adr);
    lseek(fd,(*parent_inode_adr)+8+4*i,SEEK_SET);
    read(fd,my_block_loc_adr,4);
    if (*my_block_loc_adr!= -1){
       //search in the block
      lseek(fd,*my_block_loc_adr,SEEK_SET);
      for (int j=0;j<129;j++){
        // printf("block loc is: %d\n", *my_block_loc_adr+(j*32));
        lseek(fd,*my_block_loc_adr+(j*32),SEEK_SET);
        char * my_name=(char *) malloc(28);
        int * my_inum=(int *) malloc(sizeof(int));
        read(fd,my_inum,sizeof(int));
        read(fd,my_name,28);
        // printf("The one that I read: %s\n",my_name);
        // printf("The one that is passed: %s\n",name);
        if (strcmp(my_name,name)==0){
          return *my_inum;
        }
        free(my_name);
        free(my_inum);
      }

    }
  }
  return -1;

}

int my_MFS_Stat(int fd, struct CR * my_CR,int inum, MFS_Stat_t *m){
    //Update data block of the parent directoy
  int * inode_adr=(int *) malloc(sizeof(int)); //hold loacation of the parent inode addr
  int CR_index=inum/16;
  int piece_index=inum % 16;
  //printf("Parent directory CR index: %d\n",parent_CR_index);
  //printf("parent directory piece index: %d\n",parent_piece_index);

  
  int * inode_piece_loc_adr=(int *) malloc(sizeof(int)); //holds location of the parent inode piece
  lseek(fd,CR_index*4,SEEK_SET);
  read(fd,inode_piece_loc_adr,4);
  lseek(fd,*inode_piece_loc_adr+4*piece_index,SEEK_SET);
  read(fd,inode_adr,4);

  //go to parent data block and update it
  lseek(fd,*inode_adr,SEEK_SET);
  // printf("I am the inode add from MFS_stat: %d\n",*inode_adr);
  int * file_size=(int *)malloc(sizeof(int));
  int * file_type=(int *)malloc(sizeof(int));
  read(fd,file_size,4);
  read(fd,file_type,4);
  m->size=*file_size;
  m->type=*file_type;
  free(file_type);
  free(file_size);
  free(inode_piece_loc_adr);
  free(inode_adr);
  return(0);


}

//int MFS_Write(int inum, char *buffer, int block){

//}

//int MFS_Read(int inum, char *buffer, int block){}

// a helper function to check if an inode is valid or not
int valid_inode(int fd, struct CR* my_CR,int inum){
  if (real_num_inodes_index<block_size){
    if (inum> real_num_inodes_index){
      return -1;
    }
  }
  //look for inodes addr
  //Update data block of the parent directoy
  int * inode_adr=(int *) malloc(sizeof(int)); //hold loacation of the parent inode addr
  int CR_index=inum/16;
  int piece_index=inum % 16;
  //printf("Parent directory CR index: %d\n",parent_CR_index);
  //printf("parent directory piece index: %d\n",parent_piece_index);

  
  int * inode_piece_loc_adr=(int *) malloc(sizeof(int)); //holds location of the parent inode piece
  lseek(fd,CR_index*4,SEEK_SET);
  read(fd,inode_piece_loc_adr,4);
  lseek(fd,*inode_piece_loc_adr+4*piece_index,SEEK_SET);
  read(fd,inode_adr,4);

  //go to inode block and update it
  lseek(fd,*inode_adr,SEEK_SET);
  int * size=(int *) malloc(sizeof(int));
  read(fd,size,4);
  // printf("I am size in valid inode %d\n",*size);
  if (*size ==-1){
    return -1;
  }
  return 0;
}

int inode_Unlink(int fd, struct CR * my_CR,int pinum, char *name){
  //Update data block of the parent directoy
  int * parent_inode_adr=(int *) malloc(sizeof(int)); //hold loacation of the parent inode addr
  int parent_CR_index=pinum/16;
  int parent_piece_index=pinum % 16;
  //printf("Parent directory CR index: %d\n",parent_CR_index);
  //printf("parent directory piece index: %d\n",parent_piece_index);

  
  int * parent_inode_piece_loc_adr=(int *) malloc(sizeof(int)); //holds location of the parent inode piece
  lseek(fd,parent_CR_index*4,SEEK_SET);
  read(fd,parent_inode_piece_loc_adr,4);
  lseek(fd,*parent_inode_piece_loc_adr+4*parent_piece_index,SEEK_SET);
  read(fd,parent_inode_adr,4);

  //go to parent data block and update it
  lseek(fd,*parent_inode_adr,SEEK_SET);

  //we can check if pinum is directory or not 
  //TODO
  int * inum_candid=(int *) malloc(sizeof(int));
  //int found=0; //PEYMAN
  int* block_loc_adr=(int *) malloc(sizeof(int));
  for (int i=0;i<14;i++){
      lseek(fd,*parent_inode_adr+8+i*4,SEEK_SET);
      read(fd,block_loc_adr,4);
      // printf("I am block location adr from unlink %d\n",*block_loc_adr);
      if (*block_loc_adr!=-1){
        lseek(fd,*block_loc_adr,SEEK_SET);
        //search the block
        for (int j=0;j<128;j++){
          char * name_candid=(char *)malloc(28);
          //int * inum_candid=(int *) malloc(sizeof(int));
          lseek(fd,*block_loc_adr+j*32+4,SEEK_SET);
          read(fd,name_candid,28);
          // printf("I am name_candid from unlink: %s\n",name_candid);
          // printf("I am namefrom unlink: %s\n",name);
          //return -1;
          if (strcmp(name_candid,name)==0){
            //return -1;
            // printf("FOUND!\n");
            //found=1; //PEYMAN

            /////////////////////////////////////////////////////////////////////
            lseek(fd,*block_loc_adr+j*32,SEEK_SET);
            read(fd,inum_candid,4); //holds the candid inum
            //find the node
            int * invalid_inode_adr=(int *) malloc(sizeof(int)); //hold loacation of the parent inode addr
            int invalid_CR_index=*inum_candid/16;
            int invalid_piece_index=*inum_candid % 16;
            //printf("Parent directory CR index: %d\n",parent_CR_index);
            //printf("parent directory piece index: %d\n",parent_piece_index);

            
            int * invalid_inode_piece_loc_adr=(int *) malloc(sizeof(int)); 
            lseek(fd,invalid_CR_index*4,SEEK_SET);
            read(fd,invalid_inode_piece_loc_adr,4);
            lseek(fd,*invalid_inode_piece_loc_adr+4*invalid_piece_index,SEEK_SET);
            read(fd,invalid_inode_adr,4);
            int * node_type=(int *)malloc(sizeof(int));
            lseek(fd,*invalid_inode_adr+4,SEEK_SET);
            read(fd,node_type,4);
            if (*node_type==0){//we are in a directoy
            //return -1;
            int * dir_db_loc_adr= (int *) malloc(sizeof(int));
            lseek(fd,*invalid_inode_adr+8,SEEK_SET);
            read(fd,dir_db_loc_adr,4);
            if (dir_db_loc_adr<=0){
              return -1;
            }
            lseek(fd,*dir_db_loc_adr+32*2,SEEK_SET);
            //char * tmp= (char *) malloc(32);
                        //lseek(fd,*dir_db_loc_adr+32+32+k*32,SEEK_SET);
                         //read(fd,tmp,32);
                         //if (strlen(tmp)==0){
                          // return -1;
                         //}
             //int count=0;
                   for (int k=0;k<126;k++){
                         char * tmp= (char *) malloc(32);
                         lseek(fd,*dir_db_loc_adr+32+32+k*32,SEEK_SET);
                         read(fd,tmp,32);
                         if (strlen(tmp)!=0){
                           return -1;
                         }
                       }
            // if (count<126){
            //   return -1; //dict is non-empty
            // }
            }

            //go to parent data block and update it
            lseek(fd,*invalid_inode_adr,SEEK_SET);
            // printf("I am inode_adr from unlink: %d\n",*invalid_inode_adr);
            write(fd,&null_int,4);

            /////////////////////////////////////////////////////////////////////
            lseek(fd,*block_loc_adr+j*32,SEEK_SET);
            read(fd,inum_candid,4);
            // printf(" I am inum_candid: %d\n",*inum_candid);
            //delete
            lseek(fd,*block_loc_adr+j*32,SEEK_SET);
            for (int k=0;k<32;k++){
              write(fd,&null_char,1);
            }
          }
        }
      }

  }

  //make the found inode invalid
  
  // if (found==1){
  //   // printf("I found in unlink \n" );
  //   //find the node
  //     int * invalid_inode_adr=(int *) malloc(sizeof(int)); //hold loacation of the parent inode addr
  //     int invalid_CR_index=*inum_candid/16;
  //     int invalid_piece_index=*inum_candid % 16;
  //     //printf("Parent directory CR index: %d\n",parent_CR_index);
  //     //printf("parent directory piece index: %d\n",parent_piece_index);

      
  //     int * invalid_inode_piece_loc_adr=(int *) malloc(sizeof(int)); //holds location of the parent inode piece
  //     lseek(fd,invalid_CR_index*4,SEEK_SET);
  //     read(fd,invalid_inode_piece_loc_adr,4);
  //     lseek(fd,*invalid_inode_piece_loc_adr+4*invalid_piece_index,SEEK_SET);
  //     read(fd,invalid_inode_adr,4);

  //     //go to parent data block and update it
  //     lseek(fd,*invalid_inode_adr,SEEK_SET);
  //     // printf("I am inode_adr from unlink: %d\n",*invalid_inode_adr);
  //     write(fd,&null_int,4);

  // }




  return 0;
}


int inode_creat(int fd, struct CR * my_CR, int pinum, int type, char *name){
  //num_inodes
   //pinum does not exit
   if(pinum<0 ||strlen(name)>28){
        return(-1);
  }  
  if (valid_inode(fd,my_CR,pinum)==-1){
    return -1;
  }

  //num_inodes_index=(num_inodes_index+1) % block_size; //increment inode num
  //real_num_inodes_index++;
  for (int kk=0;kk<block_size;kk++){
    if (valid_inode(fd,my_CR,kk)==-1){
      num_inodes_index=kk;
      break;
    }
  }
  
  //add a block
  int * my_block_loc_adr=(int *) malloc(sizeof(int)); //holds new block loc address
    *my_block_loc_adr=my_CR->end_log; //sets the new inode address
  lseek(fd,*my_block_loc_adr,SEEK_SET);
  //  printf("I am block address from creat: %d\n",*my_block_loc_adr);
   for (int i=0;i<block_size;i++){
     write(fd,&null_char,1);
   }
   CR_set_log_end(my_CR,block_size);
   set_check_point_size_value(fd,my_CR,check_point_size);

  ////////////////////////////////////////////////////////////////////

  //Update data block of the parent directoy
  int * parent_inode_adr_1=(int *) malloc(sizeof(int)); //hold loacation of the parent inode addr
  int parent_CR_index_1=pinum/16;
  int parent_piece_index_1=pinum % 16;
  // printf("Parent directory CR index: %d\n",parent_CR_index);
  // printf("parent directory piece index: %d\n",parent_piece_index);

  
  int * parent_inode_piece_loc_adr_1=(int *) malloc(sizeof(int)); //holds location of the parent inode piece
  lseek(fd,parent_CR_index_1*4,SEEK_SET);
  read(fd,parent_inode_piece_loc_adr_1,4);
  // printf("I am parent inode piece loc from creat: %d\n",*parent_inode_piece_loc_adr);
  lseek(fd,*parent_inode_piece_loc_adr_1+4*parent_piece_index_1,SEEK_SET);
  read(fd,parent_inode_adr_1,4);
  // printf("I am parent inode loc from creat: %d\n",*parent_inode_adr);
  //go to parent data block and update it
  lseek(fd,*parent_inode_adr_1,SEEK_SET);

  //finds the parent inode available datablock
  // Inode_print(fd,*parent_inode_adr);
  lseek(fd,*parent_inode_adr_1+4,SEEK_SET);
  int* parent_inode_type=(int *) malloc(sizeof(int));
  read(fd,parent_inode_type,4);
  if (*parent_inode_type==1){
    return -1;
  }
  // printf("I am parent inode data block loc from creat: %d\n",*parent_inode_datablock_adr);
  /////////////////////////////////////////////////////////////////////
  
  
  


   if (type==0){
     //write to the data block directory entry of . and ..
     //set root directory entry
    lseek(fd,*my_block_loc_adr,SEEK_SET);
    char* current =(char *) malloc(28);
    strcpy(current,".");

    int* my_inode=(int *) malloc(sizeof(int));
    *my_inode=num_inodes_index;
    write(fd,my_inode,4);
    write(fd,current,28);

    lseek(fd,(*my_block_loc_adr)+32,SEEK_SET);
    char* parent=(char *) malloc(28);
    strcpy(parent,"..");
    *my_inode=pinum;
    write(fd,my_inode,4);
    write(fd,parent,28);
  }


  
  
  //add an inode
  // printf("Num inodes: %d\n",num_inodes_index); 
  
  int * inode_loc_adr=(int *) malloc(sizeof(int)); //holds new inode address
  *inode_loc_adr=my_CR->end_log; //sets the new inode address
  // printf("new inode loc is : %d\n",*inode_loc_adr);
  lseek(fd,my_CR->end_log,SEEK_SET); //moves to end of the log
  
  for (int i=0;i<inode_size;i++){ //adds a block with null char 
     write(fd,&null_char,1);
   }

   //fill the inode with default values
   int * inode_size_1=(int *) malloc(sizeof(int)); //holds the inode size
   int * inode_type=(int *) malloc(sizeof(int)); //holds inode type
   *inode_size_1=0;
   *inode_type=type; //0 is directory
   inode_info_set(fd,my_CR->end_log,inode_size_1,inode_type); //sets the inode values
   lseek(fd,*inode_loc_adr+8,SEEK_SET);
   for (int i=0;i<14;i++){
     if (i==0){
      //  printf("I am block addr from creat: %d\n",*my_block_loc_adr);
       write(fd,my_block_loc_adr,4);
     }
     else{
       write(fd,&null_int,4);
     }
   }
  // Inode_print(fd,*inode_loc_adr);
  
  
  //updates log_end
  CR_set_log_end(my_CR, inode_size);
  set_check_point_size_value(fd,my_CR,check_point_size);
  
  //update check point region and corresponding inode piece
  // printf(" I am numinode index from creat: %d\n",num_inodes_index);
  int piece_index=num_inodes_index/16; //holds the piece index
  int offset=num_inodes_index %16;  //holds the inode index in the piece
  // printf("the piece index is: %d\n",piece_index);
  // printf("the piece offset is: %d\n",offset);
  int * piece_loc_adr=(int *) malloc(sizeof(int)); //hold loacation of the piece
  *piece_loc_adr=0;
  

  if (offset ==0){
    
    //need to add an inode piece 
    lseek(fd,my_CR->end_log,SEEK_SET);
    for (int i=0;i<inode_piece_size;i=i+4){
        write(fd,&null_int,4);
    }
    int * new_piece_adr=(int *) malloc(sizeof(int));
    *new_piece_adr=my_CR->end_log;

    //updates log_end
    CR_set_log_end(my_CR, inode_piece_size);
    set_check_point_size_value(fd,my_CR,check_point_size);
    
    //updates inode loc in CR
    lseek(fd,piece_index*4,SEEK_SET); //goes to the location for the index of inode 
    write(fd,new_piece_adr,4);  //write the address of inode piece
    
    //write the address of inode
    lseek(fd,*new_piece_adr,SEEK_SET);
    write(fd,inode_loc_adr,4);
  } else {
    
    //updates inode loc in CR
    lseek(fd,piece_index*4,SEEK_SET); //goes to the location for the index of inode  
    read(fd,piece_loc_adr,4);      //read the location of inode piece
    // printf("The inode piece loc is: %d\n",*piece_loc_adr);
    lseek(fd,*piece_loc_adr+offset*4,SEEK_SET);
    write(fd,inode_loc_adr,4);
}
  
  // //updates log_end
  // CR_set_log_end(my_CR, inode_piece_size);
  // set_check_point_size_value(fd,my_CR,check_point_size);
  
  //Update data block of the parent directoy
  int * parent_inode_adr=(int *) malloc(sizeof(int)); //hold loacation of the parent inode addr
  int parent_CR_index=pinum/16;
  int parent_piece_index=pinum % 16;
  // printf("Parent directory CR index: %d\n",parent_CR_index);
  // printf("parent directory piece index: %d\n",parent_piece_index);

  
  int * parent_inode_piece_loc_adr=(int *) malloc(sizeof(int)); //holds location of the parent inode piece
  lseek(fd,parent_CR_index*4,SEEK_SET);
  read(fd,parent_inode_piece_loc_adr,4);
  // printf("I am parent inode piece loc from creat: %d\n",*parent_inode_piece_loc_adr);
  lseek(fd,*parent_inode_piece_loc_adr+4*parent_piece_index,SEEK_SET);
  read(fd,parent_inode_adr,4);
  // printf("I am parent inode loc from creat: %d\n",*parent_inode_adr);
  
  
  
  //go to parent data block and update it
  
  lseek(fd,*parent_inode_adr,SEEK_SET);
  //pass over each block if a block is full and next one is not allocated allocate it 
  int allocate=0;
  for (int t=0;t<14;t++){
    if (allocate==1){
      break;
    }
    //finding the block address of t-th data block for parent
    lseek(fd,*parent_inode_adr+8+4*t,SEEK_SET);
    int* parent_inode_datablock_adr=(int *) malloc(sizeof(int));
    read(fd,parent_inode_datablock_adr,4);
    if (*parent_inode_datablock_adr==default_int){
      //we should create a block
        *parent_inode_datablock_adr=my_CR->end_log; //sets the new inode address
        //write the new data block address in parents inode
        lseek(fd,*parent_inode_adr+8+4*t,SEEK_SET);
        write(fd,parent_inode_datablock_adr,4);

        lseek(fd,*parent_inode_datablock_adr,SEEK_SET);
        for (int tt=0;tt<block_size+100;tt++){
          write(fd,&null_char,1);
        }
        CR_set_log_end(my_CR,block_size);
        set_check_point_size_value(fd,my_CR,check_point_size);
    }
    //going to the t-th parent data block and search for an available space
    lseek(fd,*parent_inode_datablock_adr,SEEK_SET);

    //find an empty space in the block 
    for (int ii=0;ii<128;ii++){
    char * tmp= (char *) malloc(32);
    lseek(fd,*parent_inode_datablock_adr+ii*32,SEEK_SET);
    read(fd,tmp,32);
    if (strlen(tmp)==0){
      lseek(fd,*parent_inode_datablock_adr+ii*32,SEEK_SET);
       char* my_name =(char *) malloc(28);
       ////////////////////////
        strcpy(my_name,name);

        int* dir_num=(int *) malloc(sizeof(int));
        *dir_num=num_inodes_index;
        write(fd,dir_num,4);
        write(fd,my_name,28);
        allocate=1;
        free(tmp);
        
        break;
        ///////////////////
        }
      }
    }
  real_num_inodes_index++;
  return 0;
}

//////////////////////////////
int server_write(int fd,struct CR* my_CR,int inum, char *buffer, int block){
    if (block>=14){
      return -1;
    }
    //add a block
    int * my_block_loc_adr=(int *) malloc(sizeof(int)); //holds new inode address
    *my_block_loc_adr=my_CR->end_log; //sets the new inode address
    lseek(fd,*my_block_loc_adr,SEEK_SET);
    write(fd,buffer,4096);    //finally write here
    // printf("I am writing at location: %d\n",*my_block_loc_adr);
    // printf("writing content is: %s",buffer);
    //update the CR
    CR_set_log_end(my_CR,4096);
    set_check_point_size_value(fd,my_CR,check_point_size);
    ///done with ading block///
    
    
    int *inode_adr=(int *) malloc(sizeof(int)); 
    int CR_index=inum/16;
    int piece_index=inum % 16;
    // printf(" CR index: %d\n",CR_index);
    // printf(" piece index: %d\n",piece_index);

    int *inode_piece_loc_adr=(int *) malloc(sizeof(int));
    lseek(fd,CR_index*4,SEEK_SET);
    read(fd,inode_piece_loc_adr,4);
    lseek(fd,*inode_piece_loc_adr+4*piece_index,SEEK_SET);
    read(fd,inode_adr,4);

    //lseek(fd,4+*inode_adr,SEEK_SET);

    //if (*inode_adr<0){ //we want to see if size is -1 
    //    return(-1);
    //}
    // int* data_adr=(int*) malloc(sizeof(int));
    //lseek(fd,*inode_adr + 4+4+ (block%14)*4,SEEK_SET);
    //write(fd,my_block_loc_adr,4);
    //if (*data_adr==-1){
      //ADD a block at the end 
      //update
    //}

    //lseek(fd,*data_adr,SEEK_SET);
    
     //updates file size
     int * temp=(int *) malloc(sizeof(int));
     lseek(fd,*inode_adr,SEEK_SET);
     read(fd,temp,4);
     if (*temp<(block+1)*block_size){
          lseek(fd,*inode_adr,SEEK_SET);
          *temp=(block+1)*block_size;
          write(fd,temp,4);
     }

        lseek(fd,*inode_adr+(4+4)+4*block,SEEK_SET);
        read(fd,temp,4);
        //if(*temp!=-1){
            lseek(fd,*inode_adr+(4+4+4*block),SEEK_SET);
            write(fd,my_block_loc_adr,4);
        //}

    //update in the inode, the block pointer array
    //for (int i=0;i<14;i++){
    //    int * temp=(int *) malloc(sizeof(int));
    //    lseek(fd,*inode_adr+(4+4)+4*i,SEEK_SET);
    //    read(fd,temp,4);
    //    if(*temp==-1){
    //        lseek(fd,*inode_adr+(4+4)+4*i,SEEK_SET);
    //        write(fd,my_block_loc_adr,4);
        //}
    //}
free(temp);
free(inode_adr);
free(inode_piece_loc_adr);
 
	return 0;
}


int server_read(int fd,struct CR* my_CR,int inum, char *buffer, int block){
   if (block>=14){
     return -1;
   }
	//if (inum>num_inodes_index){
	//	return(-1);
	//}
    
    int *inode_adr=(int *) malloc(sizeof(int)); 
    int CR_index=inum/16;
    int piece_index=inum % 16;
    // printf("CR index: %d\n",CR_index);
    // printf(" piece index: %d\n",piece_index);

    int *inode_piece_loc_adr=(int *) malloc(sizeof(int));
    lseek(fd,CR_index*4,SEEK_SET);
    read(fd,inode_piece_loc_adr,4);
    lseek(fd,*inode_piece_loc_adr+4*piece_index,SEEK_SET);
    read(fd,inode_adr,4);

    lseek(fd,4+*inode_adr,SEEK_SET);

    //finds inode available datablock
     int* inode_type_adr=(int*) malloc(sizeof(int));
     read(fd,inode_type_adr,4);
     if (inode_type_adr<0){
         return -1;
     }

    int* data_adr=(int*) malloc(sizeof(int));
    lseek(fd,*inode_adr + 4+4+ (block*4),SEEK_SET);
    read(fd,data_adr,4);

    lseek(fd,*data_adr,SEEK_SET);
    //lseek(fd,(my_CR->end_log)-4096,SEEK_SET);
    char * test=(char *) malloc(4096);
    read(fd,test,4096);
    memcpy(buffer,test,4096);
    //strcpy(buffer,"Peyman");
    // memcpy(rec.buffer,buffer,4096);
    free(inode_adr);
    free(inode_piece_loc_adr);
    free(inode_type_adr);
    free(data_adr);
    free(test);
    return(0);
}

//////////////////////////////
int find_the_next_inode(int fd, struct CR* my_CR){
  for (int i=0;i<block_size;i++){
    if (valid_inode(fd, my_CR, i)==-1){
      return i;
    }
  }
  return -1;
}



int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage: server [portnum] [file-system-image]");
        return 0;
        //exit(1);
    }
    int port = atoi(argv[1]);
    // // int rc = fsi_init(argv[2]);


    int fd; //holds the file descriptor
    struct CR* my_CR=(struct CR *) malloc(sizeof(struct CR));
    //initialize the file_system
    if( access( argv[2], F_OK ) != 0 ) {
    // file doesn't exists
    fd=open(argv[2],O_CREAT|O_RDWR,0666);
    //file system bootsup

    file_system_boot_up(fd,my_CR);
} else {
    // file exist
    fd=open(argv[2],O_RDWR,0666);
    lseek(fd,0,SEEK_SET);
    for (int i=0;i<256;i++){
      lseek(fd,i*4,SEEK_SET);
      int my_temp=-1;
      read(fd,&my_temp,4);
      my_CR->inode_map[i]=my_temp;
    }
    lseek(fd,4*256,SEEK_SET);
    int my_temp1=-1;
    read(fd,&my_temp1,4);
    my_CR->end_log=my_temp1;
}



    
    int sd = UDP_Open(port);
    assert(sd > -1);


    // MFS_message_t* message=(MFS_message_t*)malloc(sizeof(MFS_message_t));
    // MFS_message_t* reply=(MFS_message_t*)malloc(sizeof(MFS_message_t));
    //MFS_message_t* my_reply =(MFS_message_t *) malloc(sizeof(MFS_message_t));
    MFS_message_t * reply =(MFS_message_t *) malloc(sizeof(MFS_message_t));
    MFS_message_t * message=(MFS_message_t *) malloc(sizeof(MFS_message_t));
 
    while (1) {
        // int reply;
           set_me(message);
           set_me(reply);
        struct sockaddr_in addr;

        // printf("server:: waiting...\n");
        int rc;
        rc=UDP_Read(sd, &addr, (char*)message, sizeof(MFS_message_t));
        printf("I read the message\n");
        // printf("server:: read message [size:%d contents:(%s)]\n", rc, message.buffer);
        if (rc>0){
          

            // printf("i am reply: %d\n",reply.re);
            if(message->type==REQ_LOOKUP){
              set_me(reply);
              int child_inum=inode_Lookup(fd,my_CR,message->inode_num,message->buffer);
              reply->re=child_inum;
              rc = UDP_Write(sd, &addr, (char*)reply, sizeof(MFS_message_t));
            }
            else if(message->type==REQ_STAT){
              set_me(reply);

              MFS_Stat_t stat;
              my_MFS_Stat(fd,my_CR,message->inode_num,&stat);
              reply->inode_num=stat.size;
              reply->type=stat.type;
              rc = UDP_Write(sd, &addr, (char*)reply, sizeof(MFS_message_t));
            }
            else if(message->type==REQ_WRITE){
              set_me(reply);
              reply->re=server_write(fd,my_CR,message->inode_num,message->buffer,message->block);
              memcpy(reply->buffer,message->buffer,4096);
              rc = UDP_Write(sd, &addr, (char*)reply, sizeof(MFS_message_t));
            }
            else if(message->type==REQ_READ){
              //MFS_Stat_t *  my_reply= (MFS_Stat_t *) malloc(sizeof(MFS_Stat_t));
              set_me(reply);
              //my_reply.buffer=(char *)malloc(4096);
              //char * test=(char *) malloc(4096);
              reply->re=server_read(fd,my_CR,message->inode_num,reply->buffer,message->block);
              //memcpy(reply->buffer,test,4096);
              //free(test);
              rc = UDP_Write(sd, &addr, (char*)reply, sizeof(MFS_message_t));
            }
            else if(message->type==REQ_CREAT){
              //printf("--------------------------server----------------------");
              set_me(reply);
              reply->re=inode_creat(fd,my_CR,message->inode_num,message->block,message->buffer);
              //printf("the return value of create is %d\n", reply.re);
              rc = UDP_Write(sd, &addr, (char*)reply, sizeof(MFS_message_t));
            }
            else if(message->type==REQ_UNLINK){
              set_me(reply);
              reply->re=inode_Unlink(fd,my_CR, message->inode_num,message->buffer);
              rc = UDP_Write(sd, &addr, (char*)reply, sizeof(MFS_message_t));
            }
            else if(message->type==REQ_SHUTDOWN){
                // reply.type=REQ_SHUTDOWN;
                fsync(fd);
                close(fd);

                set_me(reply);
                reply->re=0;
                rc = UDP_Write(sd, &addr, (char*)reply, sizeof(MFS_message_t));
                exit(0);
                
                // char str[4];
                // sprintf(str, "%d", reply);
                // rc = UDP_Write(sd, &addr, (char*)str, sizeof(int));
                // printf("server:: reply\n");
                // if(rc<0){
                //     return(-1);
                // }
               
            }
            else{
                printf("Error, no such command");
            }
            
          // // char str[4];
          // // sprintf(str, "%d", reply);
          // printf("what is the reply value????????????? %d\n",reply.re);
          // rc = UDP_Write(sd, &addr, (char*)&reply, sizeof(int));
          // printf("server:: reply\n");
          // if(rc<0){
          //     return(-1);
          // }

        }

        // if(message->type==REQ_SHUTDOWN){
        //   MFS_message_t reply;
        //   set_me(&reply);
        //   rc = UDP_Write(sd, &addr, (char*)&reply, sizeof(MFS_message_t));
        //    exit(0);
        // }


	} 

  return 0; 
}
    
 

 
