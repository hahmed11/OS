/////////////////////////////////////////////////
//          ECSE 427 - Assignment 2            // 
//          Author- HABIB I. AHMED             // 
//              ID - 260464679                 //
//              printServer.c                  //
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
#include <signal.h>

#define BUFF_SIZE   8       //max number of items that can be present in the buffer

typedef struct 
{
    int pcBuffer[BUFF_SIZE], cidBuffer[BUFF_SIZE], tBuffer[BUFF_SIZE];
    int in, out;
    sem_t full, empty, mutex;
} Buff;

Buff * buffer;

void setup_shared_mem(); // create a shared memory segment
void init_semaphore(); // initialize the semaphore and put it in shared memory  

static volatile int keepRunning = 1;
const char *name = "/shm-temp";
int shm_fd;


//signal handler for CTRL C
void intHandler(int dummy) {
    keepRunning = 0;
    shm_unlink(name);
    if(munmap(buffer,sizeof(Buff))){
        printf("Exiting now...\n");
        exit(0);
    }
}

void bufferInitialize(){
    buffer->in = 0;
    buffer->out = 0;
    return;
}

// create a shared memory segment
void setup_shared_mem(){
    // open the shared memory segment as if it was a file 
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);

    if (shm_fd== -1) {
            printf("cons: Shared memory failed: %s\n", strerror(errno));
            exit(1);
    }


    //configure the size of the shared memory segment
    ftruncate(shm_fd, sizeof(Buff));

    //map the shared memory segment to the address space of the process 
    buffer = (Buff *) mmap(0,sizeof(Buff), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (buffer == MAP_FAILED) {
            printf("cons: Map failed: %s\n", strerror(errno));
            exit(1);
    }
} 

// initialize the semaphore and put it in shared memory
void init_semaphore(){
    if (sem_init(&buffer->mutex, 1, 1) == -1) printf("Failed to initialize mutex semaphore.\n");
    if (sem_init(&buffer->empty, 1, BUFF_SIZE) == -1) printf("Failed to initialize empty semaphore.\n");
    if (sem_init(&buffer->full, 1, 0) == -1) printf("Failed to initialize full semaphore.\n");  
} 


// 
void take_a_job(){
    
}

// sleep process
void go_sleep(){
    
}
    

//print method
void printer() {
    int val;
    
    //check if the buffer is empty
    sem_getvalue(&buffer->full, &val);
    if (val <= 0) printf("No request in buffer, Printer sleeps\n");

    //values being locked before critical section
    sem_wait(&buffer->full);
    sem_wait(&buffer->mutex);

    //takes a job from the buffer
    int bufferPosition = buffer->in;
    int currentPC = buffer->pcBuffer[bufferPosition];        
    int currentCID = buffer->cidBuffer[bufferPosition];
    int currentJobTime = buffer->tBuffer[bufferPosition];
    buffer->in = (buffer->in+1) % BUFF_SIZE;

    printf("Printer starts printing %d page(s) from Buffer[%d]\n", currentPC, bufferPosition);
    //end of critical section
    sem_post(&buffer->mutex);
    sem_post(&buffer->empty);
    
    //sleeps until the job duration is over i.e, "Printing" is taking place
    sleep(currentJobTime);
    printf("Printer has finished printing %d page(s) from Buffer[%d]\n", currentPC, bufferPosition);
    return;
}



int main(int argc, char *argv[]){

    //initialises semaphores and shared memory
    setup_shared_mem(); // create a shared memory segment
    init_semaphore();  
    signal(SIGINT, intHandler);
    while (keepRunning) {
        // take_a_job(&job); // this is blocking on a semaphore if no job
        // print_a_msg(&job); // duration of job, job ID, source of job are printed
        // go_sleep(&job); // sleep for job duration
        printer();
    }
}