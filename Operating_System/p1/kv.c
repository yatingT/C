#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

struct pair {
   int key;   
   char *value;
   struct pair *next;
};

struct pair *head = NULL;

void put(int key,char *value){
    struct pair *temp = (struct pair*) malloc(sizeof(struct pair));
    temp->key=key;
    temp->value=value;
    temp->next=NULL;
    struct pair *current = (struct pair*) malloc(sizeof(struct pair));
    if (head==NULL){
        head=temp;
    }else{
        current=head;
        while (current!=NULL){
            if(current->key==key){
                current->value=value;
                return;
            }
            current=current->next;
        }
        temp->next=head;
        head=temp;
    }
}

struct pair *get(int key) {
    struct pair *current = head;
    if(head == NULL) {
        return NULL;
    }
    while(current->key != key) {
        if(current->next == NULL) {
            return NULL;
        } else {
            current = current->next;
        }
    }   
    return current;
}

struct pair *del(int key){
    struct pair *current = head;
    struct pair *pre = NULL;

    if(head == NULL) {
        return NULL;
    }
    while(current->key != key) {
        if(current->next == NULL) {
            return NULL;
        } else {
            pre=current;
            current = current->next;
        }
    }   
    if (pre==NULL){
        free(head);
        head=current->next;
    }else{
        pre->next=current->next;
        free(current);
    }    
    return current;
}

void clear(){
    fclose(fopen("kv.txt", "w"));
    struct pair* current;
    while (head != NULL){
       current = head;
       head = head->next;
       free(current);
    }
}

char all(){
    struct pair *current = head;	
    while(current != NULL) {
        printf("%d,%s\n",current->key,current->value);
        current = current->next;
    }
}

void freell(struct pair* head){
   struct pair* current;
   while (head != NULL){
       current = head;
       head = head->next;
       free(current);
    }

}

int main( int argc, char *argv[] ) {   


    FILE *fp;
    if(access( "kv.txt", F_OK ) == 0 ){
        if (fp = fopen("kv.txt", "r")){
            char *line = NULL;
            size_t len = 0;
            while(getline(&line, &len, fp) != -1){
                char *token, *rest;
                token=strtok(line,",");
                rest=strtok(NULL,",");
                rest[strcspn ( rest, "\n" )] = '\0';
                put(atoi(token),strdup(rest));
            }
        }
    fclose(fp); 
    }

    for(int i=1; i<argc; i++){
        char *command;
        command = strdup(argv[i]);
        char *order;

        struct pair *v;
        if (strcmp(command,"c") == 0){
            clear();
        }
        else if (strcmp(command,"a") == 0){
            all();
        }else {
            int key;
            char *value;
            order = strtok(command,",");
            char *temp_key = strtok(NULL,",");
            if(!temp_key){
                printf("bad command\n");
                continue;
            }
            key=atoi(temp_key);
            value = strtok(NULL,",");

            if (strcmp(order,"p") == 0){
                if (!key || !value){
                    printf("bad command\n");
                } else{
                    put(key,value);
                }
               
            }
            else if (strcmp(order,"g") == 0){
                v= get(key);
                if (v!= NULL){
                    printf("%d,%s\n",v->key, v -> value);
                } else if(value){
                    printf("bad command\n");
                    continue;
                }else{
                    printf("%d not found\n",key);
                }
                   
            }
            else if (strcmp(order,"d") == 0){
                if (value){
                    printf("bad command\n");
                    continue;
                }
                del(key);
            }
        } 

    }

    FILE * fptr;
    fptr = fopen("kv.txt", "w+");
    struct pair * running = head;
    if(fptr==NULL){
        printf("file open error when store\n");
    }else{
        while(running!= NULL){
            fprintf(fptr, "%d,%s\n", running->key, running->value);
            running= running->next;
        }
    }
    fclose(fptr);  

    freell(head);

    return(0);
    
}