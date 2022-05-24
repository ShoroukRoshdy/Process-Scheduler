#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <ctype.h>
#include <signal.h>


#define BUF_SIZE 8

////////// Semaphores ///////////

union Semun
{
    int val;               /* value for SETVAL */
    struct semid_ds *buf;  /* buffer for IPC_STAT & IPC_SET */
    ushort *array;         /* array for GETALL & SETALL */
    struct seminfo *__buf; /* buffer for IPC_INFO */
    void *__pad;
};

void down(int sem)
{
    struct sembuf p_op;

    p_op.sem_num = 0;
    p_op.sem_op = -1;
    p_op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &p_op, 1) == -1)
    {
        perror("Error in down()");
        exit(-1);
    }
}

void up(int sem)
{
    struct sembuf v_op;

    v_op.sem_num = 0;
    v_op.sem_op = 1;
    v_op.sem_flg = !IPC_NOWAIT;

    if (semop(sem, &v_op, 1) == -1)
    {
        perror("Error in up()");
        exit(-1);
    }
}
union Semun semun;
int mutex,full,empty;
int Create_EmptySemaphore()
{
    empty = semget(500, BUF_SIZE, 0666 | IPC_CREAT | IPC_EXCL);
    if (empty == -1)
    {
        int semC = semget(500, BUF_SIZE, 0666 | IPC_CREAT);
        if (semC == -1)
        {
            perror("Error  in sem empty");
            exit(-1);
        }
        empty = semC;
        return semC;
    }

        /* initial value of the semaphore, Binary semaphore */
        semun.val = BUF_SIZE; 
        if (semctl(empty, 0, SETVAL, semun) == -1)
        {
            perror("Error in semctl");
            exit(-1);
        }  
    return empty;
}
int Create_FullSemaphore()
{
    full = semget(400, BUF_SIZE, 0666 | IPC_CREAT | IPC_EXCL);
    if (full == -1)
    {
        int semC = semget(400, BUF_SIZE, 0666 | IPC_CREAT);
        if (semC == -1)
        {
            perror("Error  in sem empty");
            exit(-1);
        }
        full = semC;
        return semC;
    }
        /* initial value of the semaphore, Binary semaphore */

    semun.val = 0; 
    if (semctl(full, 0, SETVAL, semun) == -1)
    {
        perror("Error in semctl");
        exit(-1);
    }
    return full;
}
int Create_BinarySemaphore()
{
    mutex = semget(300, 1, 0666 | IPC_CREAT | IPC_EXCL);
    if (mutex == -1)
    {
        int semC = semget(300, 1, 0666 | IPC_CREAT);
        if (semC == -1)
        {
            perror("Error  in sem empty");
            exit(-1);
        }
        mutex = semC;
        return semC;
    }
        /* initial value of the semaphore, Binary semaphore */

    semun.val = 1; 
    if (semctl(mutex, 0, SETVAL, semun) == -1)
    {
    perror("Error in semctl");
    exit(-1);
    } 
    return mutex;
}

/////////////////// Shared Memory /////////////////////
int* buffer;	 // buffer to hold data
int* index;
int* indexConsumer;
int shmidindex,shmid_consumer;
int shmid;
void Create_Buffer()
{
    shmid = shmget(700, BUF_SIZE * sizeof(int),  IPC_EXCL |IPC_CREAT | 0644);
    if (shmid == -1)
    {
        int shmidC = shmget(700, BUF_SIZE * sizeof(int), IPC_CREAT | 0644);
        
        if (shmidC == -1)
        {
            perror("Error in create buffer");
            exit(-1);
        }
        shmid = shmidC;
        buffer = (int *)shmat(shmidC, (void *)0, 0);
        
    }
    else
    {
        buffer = (int *)shmat(shmid, (void *)0, 0);

    }   
}
void Create_index()
{
    shmidindex = shmget(2000,4, IPC_EXCL | IPC_CREAT | 0644);
    if (shmidindex == -1)
    {
        int shmidC = shmget(2000,4, IPC_CREAT | 0644);

        if (shmidC == -1)
        {
            perror("Error in create buffer");
            exit(-1);
        }
        shmidindex = shmidC;
        index = (int *)shmat(shmidC, (void *)0, 0);

    }
    else
    {
        index = (int *)shmat(shmidindex, (void *)0, 0);
        index[0] = 0;    
    }
    
    
}
void Create_indexConsumer()
{
    shmid_consumer = shmget(3000,4, IPC_EXCL | IPC_CREAT | 0644);
    if (shmid_consumer == -1)
    {
        int shmidC = shmget(3000,4, IPC_CREAT | 0644);

        if (shmidC == -1)
        {
            perror("Error in create buffer");
            exit(-1);
        }
        shmid_consumer = shmidC;
        indexConsumer = (int *)shmat(shmidC, (void *)0, 0);

    }
    else
    {
        indexConsumer = (int *)shmat(shmid_consumer, (void *)0, 0);
        *indexConsumer = 0;    
    }
    
    
}
void destroy()
{
    shmctl(shmidindex, IPC_RMID, (struct shmid_ds *)0);
    shmctl(shmid, IPC_RMID, (struct shmid_ds *)0);
    shmctl(shmid_consumer, IPC_RMID, (struct shmid_ds *)0);

    semctl(mutex,0,IPC_RMID,semun);
    semctl(full,0,IPC_RMID,semun);
    semctl(empty,0,IPC_RMID,semun);

}