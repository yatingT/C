
#include <string.h>
#include <sys/select.h>
#include "udp.h"
#include "mfs.h"


struct sockaddr_in addrSnd;
struct sockaddr_in addrRcv;



char *hostname;
int port;
int fd;
int default_int=-1;
char * default_str="AAAA";
MFS_message_t * rec= NULL;//(struct __MFS_message_t *) malloc(sizeof(MFS_message_t));
MFS_message_t * msg= NULL;
void set_me (MFS_message_t * msg){
    msg->re=default_int;
    strcpy(msg->buffer,default_str);
    msg->type=default_int;
    msg->block=default_int;
    msg->inode_num=default_int;
}

int msg_send(MFS_message_t * my_msg, MFS_message_t * my_rec, char *hostname,int port ){
    int rc;
    int get_return=0;
    while(!get_return){
        fd_set rd;
        struct timeval time;
        FD_ZERO(&rd);
        FD_SET(fd,&rd);
        time.tv_sec=1;
        time.tv_usec=0;
        //printf("I am hereeee \n");
        rc =UDP_Write(fd,&addrSnd,(char*)my_msg,sizeof(MFS_message_t));
        printf("this is what i sent:\n inum: %d \n block: %d \n buffer: %s\n msg.type %d\n return: %d\n",my_msg->inode_num,my_msg->block,my_msg->buffer,my_msg->type,my_msg->re);
        //printf("This if I print: %s\n",msg.buffer);
        // printf("\n--------START----------------\n");
        //     for (int i=0;i<4096;i++){
        //         printf("%c",msg.buffer[i]);
        //     }
        // printf("\n---END-----------------\n");
        if (rc < 0) {
            printf("client:: failed to send\n");
            exit(1);
        }
        get_return=select(fd +1 ,&rd,NULL,NULL,&time);
        //printf("after select\n");
        if (get_return==0){
            printf("select time out\n");
        }else if(get_return>0){
            //set_me(&rec);
            set_me(my_rec);
            rc=UDP_Read(fd,&addrRcv,(char*)my_rec,sizeof(MFS_message_t));
            
            printf("this is what i received:\n inum: %d \n block: %d \n buffer: %s\n msg.type: %d \n return: %d\n",my_rec->inode_num,my_rec->block,my_rec->buffer,my_rec->type,my_rec->re);
            // printf("\n--------START----------------\n");
            // for (int i=0;i<4096;i++){
            //     printf("%c",my_rec->buffer[i]);
            // }
            // printf("\n---END-----------------\n");
            //return(0);
            return (my_rec->re);
        }
    }
    return(0);
}

int MFS_Init(char *my_hostname, int my_port){
    //printf("HEEWWWEEEE\n");
    fd = UDP_Open(0);
    //fd=socket(AF_INET,SOCK_DGRAM,0);
    //printf("Iam out\n");
    if (fd < 0){
        //printf("Heeeeeeee\n");
        return -1;
    }
    //int rc = UDP_FillSockAddr(&addrSnd, my_hostname, my_port); 
    //printf("HeeeeeGGGGee\n");
    UDP_FillSockAddr(&addrSnd, my_hostname, my_port); 

    //if(rc<0){
    //    return(-1);
    //}
    //printf("HeeeeeUUUUeee\n");
    hostname=my_hostname;
    //printf("HeeeRRRReeeee\n");
    port=my_port;
    //printf("reached the end of init\n");
    return(0);
}

int MFS_Lookup(int pinum, char *name){
    if (pinum<0 ||strlen(name)>28){
        return(-1);
    }
    msg= malloc(sizeof(MFS_message_t));
    rec=malloc(sizeof(MFS_message_t));
   
   //=(MFS_message_t *)malloc(sizeof(MFS_message_t));
    set_me(msg);
    set_me(rec);
    //MFS_message_t msg;
    //set_me(msg);
    msg->inode_num=pinum;
    msg->type=REQ_LOOKUP;
    strcpy(msg->buffer,name);
    //MFS_message_t rec;
    set_me(rec);
    int re = msg_send(msg,rec,hostname,port);
    free(msg);
    free(rec);
    return re;
}

int MFS_Stat(int inum, MFS_Stat_t *m){
    if(inum<0){
        return(-1);
    }
    msg= malloc(sizeof(MFS_message_t));
    rec=malloc(sizeof(MFS_message_t));
    set_me(msg);
    set_me(rec);
    msg->inode_num= inum;
    msg->type=REQ_STAT;
    //MFS_message_t rec;
    msg_send(msg,rec,hostname,port);
    //if (re<0){
    //    return -1;
    //}
    m->size=rec->inode_num;
    m->type=rec->type;
    free(msg);
    free(rec);
    return 0;
}

int MFS_Write(int inum, char* buffer, int block){ //todo: chick if the file is regular file
    if(inum<0 || block<0){
        return(-1);
    }
    msg= malloc(sizeof(MFS_message_t));
    rec=malloc(sizeof(MFS_message_t));
   
   //=(MFS_message_t *)malloc(sizeof(MFS_message_t));
    set_me(msg);
    set_me(rec);
    //MFS_message_t * msg =(MFS_message_t *)malloc(sizeof(MFS_message_t));
    //set_me(msg);
    msg->inode_num= inum;
    msg->type=REQ_WRITE;
    msg->block=block;
    //msg->buffer=malloc(sizeof(4096));
    //memset(msg->buffer,0,4096);
    memcpy(msg->buffer,(const char *)buffer,4096);
    //strcpy(msg->buffer,"PPPPPPPPP");
    //printf(" I am here: %s \n",msgbuffer);
    //MFS_message_t rec;
    set_me(rec);
    int re=msg_send(msg,rec,hostname,port);
    free(msg);
    free(rec);
    return re;
}


int MFS_Read(int inum, char *buffer, int block){
    msg= (MFS_message_t *)malloc(sizeof(MFS_message_t));
    rec=(MFS_message_t *) malloc(sizeof(MFS_message_t));
   
   //=(MFS_message_t *)malloc(sizeof(MFS_message_t));
    set_me(msg);
    set_me(rec);
    if(inum<0 || block <0){
        return(-1);
    }
    //MFS_message_t msg;
    //set_me(msg);
    msg->inode_num= inum;
    msg->type=REQ_READ;
    msg->block=block;
    //msg.buffer="";


    //MFS_message_t rec;
    int re=msg_send(msg,rec,hostname,port);
    //re=UDP_Read(fd,&addrRcv,(char*)&rec,sizeof(MFS_message_t));
    memcpy(buffer,rec->buffer,4096);
    free(msg);
    free(rec);
    return re;
}


int MFS_Creat(int pinum, int type, char *name){ //todo: if the name is already exists return success
    //MFS_message_t  msg;
    msg= malloc(sizeof(MFS_message_t));
    rec=malloc(sizeof(MFS_message_t));
   
   //=(MFS_message_t *)malloc(sizeof(MFS_message_t));
    set_me(msg);
    set_me(rec);
    msg->inode_num= pinum;
    msg->type=REQ_CREAT;
    msg->block=type;
    strcpy(msg->buffer,name);
    //strcpy(msg->buffer,name);
    //msg.block=type;
    //strcpy(msg.buffer,default_str);
    //msg.re=default_int;
    //strcpy(msg.buffer,"HELLO!");
    // printf("I am pinum ftom creat: %d\n",pinum);
    // printf("I am inode_num from creat: %d\n",msg.inode_num);
    // printf("I am inode_type from creat: %d\n",msg.type);
    // printf("I am inode_type from creat: %s\n",msg.buffer);
    //MFS_message_t rec;
    int re=msg_send(msg,rec,hostname,port);
    if (re<0){
        return (-1);
    }
    //set_me(&rec);
    free(msg);
    free(rec);
    return(re);
        
    //     if (rec.inode_num<0){
    //         return(-1);
    //     }else{
    //         return(0);
    //     }
    // } else{
    //     return(-1);
    // }
    
}
int MFS_Unlink(int pinum, char *name){  //to do: check if the dirctory is empty or not

    if(pinum<0 ||strlen(name)>28){
        return(-1);
    }
    msg= malloc(sizeof(MFS_message_t));
    rec=malloc(sizeof(MFS_message_t));
   
   //=(MFS_message_t *)malloc(sizeof(MFS_message_t));
    set_me(msg);
    set_me(rec);
    //MFS_message_t msg;
    msg->inode_num= pinum;
    msg->type=REQ_UNLINK;
    memcpy(msg->buffer,name,sizeof(28));
    msg->re=-1;
    //MFS_message_t rec;
    if (msg_send(msg,rec,hostname,port)<0){
        return(-1);
    }
    free(msg);
    free(rec);
    return(0);
        
    //     if (rec.inode_num<0){
    //         return(-1);
    //     }else{
            return(0);
    //     }
    // } else{
    //     return(-1);
    // }
    

}



int MFS_Shutdown(){

    //MFS_message_t msg;
    //MFS_message_t rec;
    msg= malloc(sizeof(MFS_message_t));
    rec=malloc(sizeof(MFS_message_t));
    set_me(msg);
    set_me(rec);
    msg->type=REQ_SHUTDOWN;
    //printf("I am here\n");
    int re=msg_send(msg,rec,hostname,port);
    //printf("get back from message: %d\n",re);
    //return re;
    free(msg);
    free(rec);
    return re;
    // if(re<0){
    //     return(-1);
    // }
    // return(0);
    
}