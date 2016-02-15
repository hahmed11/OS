/////////////////////////////////////////////////
//          ECSE 427 - Assignment 2            // 
//          Author- HABIB I. AHMED             // 
//              ID - 260464679                 //
//               printClient.c                 //
//                                             //
/////////////////////////////////////////////////
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <errno.h>
#include <fcntl.h>
#include <semaphore.h>

#define BUFF_SIZE   8       //max number of items that can be present in the buffer

typedef struct 
{
    int pcBuffer[BUFF_SIZE], cidBuffer[BUFF_SIZE], tBuffer[BUFF_SIZE];
    int in, out;
    sem_t full, empty, mutex;
} Buff;

Buff * buffer;
int cid, pgcnt, tj;         //Client ID, Pages to Print & Job duration


void attach_shared_mem(); // attach the shared memory segment
// put_a_job(&job);    // put the job record into the shared buffer
void release_share_mem(); // release the shared memory
void place_params();     // this is to place the parameters in the shared memory – in particular the job queue and semaphore
void get_job_params();   // read the terminal and get the job params

void bufferInitialize(){
    buffer->in = 0;
    buffer->out = 0;
    return;
}


// attach the shared memory segment
void attach_shared_mem(){
    const char *name = "/shm-temp";
    int shm_fd;

    // open the shared memory segment as if it was a file 
    shm_fd = shm_open(name, O_RDWR, 0666);

    if (shm_fd== -1) {
            printf("cons: Shared memory failed: %s\n", strerror(errno));
            exit(1);
    }

    //map the shared memory segment to the address space of the process 
    buffer = (Buff *) mmap(0,sizeof(Buff), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (buffer == MAP_FAILED) {
            printf("cons: Map failed: %s\n", strerror(errno));
            exit(1);
    }    
} 


 // release the shared memory
void release_share_mem(){

}

// this is to place the parameters in the shared memory – in particular the job queue and semaphore
void place_params(){

}  


// read the terminal and get the job params   
void get_job_params(){

}   


//method for the client part of the program
void client(int cid, int pgcnt, int tj){
    int val;
    
    //check if the buffer is empty
    sem_getvalue(&buffer->empty, &val);
    if (val <= 0) printf("Client %d has %d pages to print, buffer full, sleeps\n", cid, pgcnt);

    //values being locked before critical section
    sem_wait(&buffer->empty);
    sem_wait(&buffer->mutex);

    //stores to buffer
    buffer->cidBuffer[buffer->out] = cid;
    buffer->pcBuffer[buffer->out] = pgcnt;    
    buffer->tBuffer[buffer->out] = tj;

    printf("Client %d has %d pages to print, puts request in Buffer[%d]\n", cid, pgcnt, buffer->out);

    buffer->out = (buffer->out+1) % BUFF_SIZE;

    //end of critical section
    sem_post(&buffer->mutex);
    sem_post(&buffer->full);
}    
    
int main(int argc, char *argv[]){

    //checks for number of parameters entered
    if (argc != 4) {
        printf("Invalid input\n Please enter the following: <Executable Name> <Client ID> <Pages to Print> <Job Duration>\n");
        exit(1);
    } 
    
    //checks for positive integers
    if (sscanf(argv[1], "%d", &cid) != 1) {
        printf("Invalid input: The value you entered for 'Client ID' is not an integer.\n");
        exit(1);
    }

    else if (sscanf(argv[2], "%d", &pgcnt) != 1) {
        printf("Invalid input: The value you entered for 'Pages to Print' is not an integer.\n");
        exit(1);
    }

    else if (sscanf(argv[3], "%d", &tj) != 1) {
        printf("Invalid input: The value you entered for 'Job Duration' is not an integer.\n");
        exit(1);
    }

    else if (cid < 1 || pgcnt < 1 || tj < 1) {
        printf("Invalid input: One of the parameters is negative or null.\n");
        exit(1);
    }

    attach_shared_mem(); // attach the shared memory segment
    client(cid, pgcnt, tj);
}

