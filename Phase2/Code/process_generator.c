//#include "headers.h"
#include "Queue.h"
#include <stdlib.h>
void clearResources(int);
int sPID ,cPID;
int msgq_id;
int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    struct Queue* q = CreateQueue();
    FILE * processFile;
    processFile = fopen("processes.txt","r");
    if (processFile == NULL)
    {
        printf("\nError Reading file! \n");
        exit(0);
    }
    int temp;
    int count = -1; // number of processes
    char* buf = NULL;
    ssize_t read;
    size_t len =0;
    read = getline(&buf,&len,processFile);
    
    while(!feof(processFile))
    { 
        count++;
        struct PCB temp;
        fscanf(processFile,"%d",&temp.id);
        fscanf(processFile,"%d",&temp.arrival_time);
        fscanf(processFile,"%d",&temp.running_time); 
        fscanf(processFile,"%d",&temp.priority); 
        fscanf(processFile,"%d",&temp.memSize);
 
        enQueue(q,temp);   
    }
    printf("\n count %d\n",count);

    fclose(processFile);
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    int algo, quant;

    printf("\n Choose scheduling algorithm ");
    printf("\n Enter 1 For Non-Preemtive high priority first ");
    printf("\n Enter 2 For Shortest remaining time first ");
    printf("\n Enter 3 For Round robin \n");
    scanf("%d",&algo);
    if (algo == 3)
    {
        fflush(stdin);
        printf("\nEnter Quantum\n");
        scanf("%d",&quant);
        fflush(stdin);
    }
    // 3. Initiate and create the scheduler and clock processes.   
    msgq_id = init_SCH_GEN_MQueue();

    cPID = fork();
    if (cPID == 0)
    {
        char* const c[] = {"./clk.o",NULL};
        execvp(c[0],c);
    }
    sPID = fork();
    if (sPID ==0)
    {   
        char algorithm [2];
        char quantum [2];
        char proc_count[4];
        sprintf(algorithm,"%d",algo);
        sprintf(quantum,"%d",quant);
        sprintf(proc_count,"%d",count);

        char* const sch[] = {"./scheduler.o",algorithm,quantum,proc_count,NULL};
        execvp(sch[0],sch);
    }
    
    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int x = getClk();
    //printf("current time is %d\n", x);
 
    // TODO Generation Main Loop    
    // // 6. Send the information to the scheduler at the appropriate time.
    struct msgbuff message;
    message.mtype = 1000;
    while (q->front !=NULL)
    {
        while (q->front->data.arrival_time > getClk()){}
        //printf("\nClk : %d , Arrival : %d\n",getClk(),q->front->data.arrival_time);   
        message.process = q->front->data;
        int send_val = msgsnd(msgq_id, &message, sizeof(message.process),!IPC_NOWAIT); // send message to server
        if (send_val == -1)
            perror("Process Generator : Errror in send");
        deQueue(q);
    }
    while(1){}
    // 7. Clear clock resources
   // destroyClk(true);
}

void clearResources(int signum)
{
    msgctl(msgq_id, IPC_RMID, (struct msqid_ds *)0);
//    kill(sPID,SIGKILL);
//    kill(cPID,SIGKILL);
    exit(0);
  // kill(getpid(),SIGKILL);

    //TODO Clears all resources in case of interruption
}
