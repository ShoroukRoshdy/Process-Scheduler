#include <stdio.h>      //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

typedef short bool;
#define true 1
#define false 0

#define SHKEY 300


///==============================
//don't mess with this variable//
int * shmaddr;                 //
//===============================



int getClk()
{
    return *shmaddr;
}


/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
*/
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *) shmat(shmid, (void *)0, 0);
}

/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
*/

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}


////// Our ///////

typedef struct PCB{
    int id;
    int priority;
    int start_time;
    int running_time;
    int arrival_time;
    int state;
    int remaining_time;
    int waiting_time; // so far ll process 
    int turnaround_time;
    float weighted_turnaround_time;
    int wait_time; // total waiting time
    int finish_time;
    int pid;
    int cont;
    int last_stopped;
    int memSize;
} PCB;

// struct msgbuff
// {
//     long mtype;
//     //int Algorithm;
//    // int Quantum;
// };
struct msgbuff
{
    long mtype;
    PCB process; 
};
int init_SCH_GEN_MQueue()
{
    int msgq_id = msgget(1000, 0666 | IPC_CREAT);

    if (msgq_id == -1) // if failed to get id
    {
        perror("Error in create");
        exit(-1);
    }
    return msgq_id; 
}

////////////////////////////////////////////
struct memoryBlock{
    int memorySize;
    int startIndex;
    int endIndex;
    int pid;
    bool alloc;
} memoryBlock;
//////////////////////// shared memory between process and scheduler ///////////////////
int *SharedMemory;


void writeShM(int data)
{
    (*SharedMemory) = data;
}
int readShM()
{
    return (*SharedMemory);
}
void initSharedMemory()
{
    int shmid = shmget(700, 4, IPC_CREAT | 0644);
    while ((int)shmid == -1)
    {
        //Make sure that the clock exists
        printf("Wait! The SharedM not initialized yet!\n");
        sleep(1);
        shmid = shmget(700, 4, 0444);
    }
    SharedMemory = (int *) shmat(shmid, (void *)0, 0);
}
void destroyShM()
{
    shmdt(SharedMemory);

}