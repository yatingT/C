#include <stdio.h>
#include "udp.h"
#include "mfs.c"
#include "mfs.h"



#define BUFFER_SIZE (4096)
// client code
int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage: client [serverhost] [serverport]");
        exit(1);
    }

    int port = atoi(argv[2]);
    int rc = MFS_Init(argv[1],port);
    printf("init passes\n");
    //int rc=0;
    // printf("reach here?");
    if (rc < 0) {
        printf("Error: Client initialization failed!");
    }


    char buffer[BUFFER_SIZE];
    printf("%s", buffer);
    //strcat(buffer,"hello world");
    MFS_Shutdown();
    //MFS_Creat(0,1,"test");
    //MFS_Stat_t* stat=(MFS_Stat_t*)malloc(sizeof(MFS_Stat_t));
    //MFS_Stat(1,stat);
    //MFS_Write(1,buffer,1);
    //int try=MFS_Read(1,buffer,1);
    // printf("4\n");
    //printf("try: %d",try);

    //MFS_Lookup(1,"test");
    //MFS_Shutdown();


    return 1;
}

 