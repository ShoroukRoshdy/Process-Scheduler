#include "Queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h> 
///////////////// HPF ///////////////////
PNode*Head; 
struct memoryBlock * memoryArray;

struct Queue* waitingQueue;  
struct Queue*Fqueue;
int flag = 0;
int stat_loc;
void CollectChildren(int);
int NumberOfProcesses;
PCB RunningProcess;
FILE * outputFile;
FILE* memoryFile;
struct Queue* HPF(int NumOfProcesses,struct memoryBlock * memArray)
{
    memoryArray = memArray;
    initClk();
    NumberOfProcesses = NumOfProcesses;
    // if child terminate -> send signal "SIGCHLD " to parent
    signal(SIGCHLD, CollectChildren);
    // 1. Initiate Message Queue with PG and create data structure suitable for this algorithm
    int msgq_id = init_SCH_GEN_MQueue(); // to send the arrival processes 
    struct msgbuff message;
    // Ready Queue
    Head = (PNode*)malloc(sizeof(PNode));  // Create Node for Priority LinkedList  
    Head = NULL;
    // Finish Queue
    Fqueue = CreateQueue();
    // waiting Queue
    waitingQueue = CreateQueue();

    //////////
    int lastReceived=-1;
    memoryFile = fopen("memory.log","w");
    outputFile = fopen("scheduler.log","w");
    fprintf(outputFile,"#At time x process y state arr w total z remain y wait k\n");
    fprintf(memoryFile,"#At time x allocated y bytes for process z from i to j\n");
 
    while ( NumberOfProcesses > 0)
    {
        // RCV Message
        int rec = msgrcv(msgq_id, &message,sizeof(message.process),0, IPC_NOWAIT | MSG_NOERROR);
        if (rec != -1)
        {
            if (message.process.id != lastReceived)
            {
                lastReceived = message.process.id;
                int isAlloc = allocateProcess(message.process,memoryArray);
                if (isAlloc == -1) // not allocated
                {
                    // ha7otha fl waiting queue , ashelha mn el ready queue
                    message.process.remaining_time = message.process.running_time; // intially Running Time = Arrival Time;  

                    enQueue(waitingQueue,message.process);
                }
                else
                {
                    fprintf(memoryFile,"At time %d allocated %d for process %d from %d to %d\n",getClk(),message.process.memSize,message.process.id,memoryArray[isAlloc].startIndex,memoryArray[isAlloc].endIndex);
                    message.process.remaining_time = message.process.running_time; // intially Running Time = Arrival Time;  
                    push(&Head,message.process,message.process.priority);      

                }
            }   
        }
        if (flag == 0) // mafesh process ma3molha fork : mafesh process bt run
        {
            if (Head != NULL) // law el queue msh fady
            {
                RunningProcess = Head->data;

                RunningProcess.start_time = getClk();
            
                int pid = fork(); // ha fork el process ely 3aleha el dor
            
                if (pid == 0)
                {
                char rem[4];
                sprintf(rem,"%d",Head->data.running_time);
                char* const p[] = {"./process.o",rem,NULL};
                execvp(p[0],p);

                }
                RunningProcess.wait_time = RunningProcess.start_time - RunningProcess.arrival_time;
                fprintf(outputFile,"At time %d process %d started arr %d total %d remain %d wait %d\n",getClk(),RunningProcess.id,RunningProcess.arrival_time,RunningProcess.running_time,RunningProcess.running_time,RunningProcess.wait_time);
                pop(&Head); // hashelo mn el ready queue
                flag =1; // fe process bt run

                    
            }
                
                
        }
        
    }
    return Fqueue;
}   
void CollectChildren(int signal)
{
    if( waitpid(-1,&stat_loc,WNOHANG) > 0 ) 
    {
        if (WIFEXITED(stat_loc))
        {
            RunningProcess.finish_time = WEXITSTATUS(stat_loc);
            RunningProcess.turnaround_time = RunningProcess.finish_time - RunningProcess.arrival_time;
            RunningProcess.wait_time = RunningProcess.finish_time-RunningProcess.running_time - RunningProcess.arrival_time;
            RunningProcess.weighted_turnaround_time = (float)RunningProcess.turnaround_time / (float)RunningProcess.running_time;
            RunningProcess.remaining_time = 0;                
            fprintf(outputFile,"At time %d process %d finished arr %d  remain %d wait %d ",RunningProcess.finish_time,RunningProcess.id,RunningProcess.arrival_time,RunningProcess.remaining_time,RunningProcess.wait_time);    
            fprintf(outputFile,"TA %d WTA %0.2f \n",RunningProcess.turnaround_time,RunningProcess.weighted_turnaround_time);
            enQueue(Fqueue,RunningProcess);
            NumberOfProcesses--;
            RunningProcess.state=-1;
            flag =0; // harag3 el flag b zero 3shan a22dar a fork tany
            int deAlloc = deallocateProcces(RunningProcess,memoryArray);
            fprintf(memoryFile,"At time %d freed %d from process %d from %d to %d\n",getClk(),RunningProcess.memSize,RunningProcess.id,memoryArray[deAlloc].startIndex,memoryArray[deAlloc].endIndex);
            
            struct Queue* temp = CreateQueue();
            while (waitingQueue->front !=NULL)
            {
                
                int isAlloc =allocateProcess(waitingQueue->front->data,memoryArray);
                if (isAlloc != -1)
                {
                    fprintf(memoryFile,"At time %d allocated %d for process %d from %d to %d\n",getClk(),waitingQueue->front->data.memSize,waitingQueue->front->data.id,memoryArray[isAlloc].startIndex,memoryArray[isAlloc].endIndex);
                    push(&Head,waitingQueue->front->data,waitingQueue->front->data.priority);
                    deQueue(waitingQueue);
                }
                else
                {
                    enQueue(temp,waitingQueue->front->data);
                    deQueue(waitingQueue);
                }          
            }
            while (temp->front !=NULL)
            {
                enQueue(waitingQueue,temp->front->data);
                deQueue(temp);
            }
        
        
        }
        
        

    }    
}
////////////////////////////////// RR ////////////////////////////////
void CollectChildrenRR(int);
struct Queue* q ;
struct Queue* Fq ;

struct Queue* RR(int NumOfProcesses,int Quantum,struct memoryBlock * memArray)
{
    memoryArray = memArray;
    initClk();
    flag =0;
    NumberOfProcesses = NumOfProcesses;
    // if child terminate -> send signal "SIGCHLD " to parent
    signal(SIGCHLD, CollectChildrenRR);
    // 1. Initiate Message Queue with PG and create data structure suitable for this algorithm
    int msgq_id = init_SCH_GEN_MQueue(); // to send the arrival processes 
    struct msgbuff message;
    int Qclk = 0;    
    // Ready Queue
    q = CreateQueue();
    // Finish Queue
    Fq = CreateQueue();
    //////////
    waitingQueue = CreateQueue();
    int lastReceived=-1;
    RunningProcess.state = -1 ; // fadya

    memoryFile = fopen("memory.log","w");
    outputFile = fopen("scheduler.log","w");
    fprintf(outputFile,"#At time x process y state arr w total z remain y wait k\n");
    fprintf(memoryFile,"#At time x allocated y bytes for process z from i to j\n");

    while ( NumberOfProcesses > 0)
    {
        // RCV Message
        int rec = msgrcv(msgq_id, &message,sizeof(message.process),0, IPC_NOWAIT | MSG_NOERROR);
        if (rec != -1)
        {
            if (message.process.id != lastReceived)
            {

                lastReceived = message.process.id;
                int isAlloc = allocateProcess(message.process,memoryArray);
                if (isAlloc == -1) // not allocated
                {                    // ha7otha fl waiting queue , ashelha mn el ready queue
                    message.process.remaining_time = message.process.running_time; // intially Running Time = Arrival Time;  
                    message.process.state = 0;
                    message.process.waiting_time =0;
                    message.process.wait_time =0;
                    message.process.last_stopped=0;

                    enQueue(waitingQueue,message.process);
                }
                else
                {
                    fprintf(memoryFile,"At time %d allocated %d for process %d from %d to %d\n",getClk(),message.process.memSize,message.process.id,memoryArray[isAlloc].startIndex,memoryArray[isAlloc].endIndex);
                    message.process.remaining_time = message.process.running_time; // intially Running Time = Arrival Time;  
                    message.process.state = 0;
                    message.process.waiting_time =0;
                    message.process.wait_time =0;
                    message.process.last_stopped=0;

                    enQueue(q,message.process);  
                }  
            }   
        }
        if (RunningProcess.state == -1) // law mafesh process bt run
        {
            // hadkhol a fork process w akhleha t run
            if (q->front != NULL) // law el queue msh fady
            {
               if ( q->front->data.state == 2) // law el next fl queue kanet stopped abl keda
                {
                  
                    RunningProcess = q->front->data;
                    RunningProcess.state = 1;
                    deQueue(q);
                    RunningProcess.cont = getClk();
                    RunningProcess.waiting_time += (getClk() - RunningProcess.last_stopped);
                    RunningProcess.wait_time += RunningProcess.waiting_time;
                    fprintf(outputFile,"At time %d process %d resumed arr %d total %d remain %d wait %d\n",getClk(),RunningProcess.id,RunningProcess.arrival_time,RunningProcess.running_time,RunningProcess.remaining_time,RunningProcess.wait_time);
                    writeShM(RunningProcess.waiting_time);
                    kill(RunningProcess.pid,SIGCONT); // hakhaleha tkmel

                }
                else if (q->front->data.state== 0) // awl marra t run
                {
                    RunningProcess = q->front->data;
                    RunningProcess.state = 1;
                    RunningProcess.start_time = getClk(); 
                    RunningProcess.cont = getClk();  
                    // fork new process
                    RunningProcess.wait_time = RunningProcess.start_time -  RunningProcess.arrival_time;
                    fprintf(outputFile,"At time %d process %d started arr %d total %d remain %d wait %d\n",getClk(),RunningProcess.id,RunningProcess.arrival_time,RunningProcess.running_time,RunningProcess.remaining_time,RunningProcess.wait_time);                  
                    writeShM(0); // el waiting time b zero
                    int pid = fork(); // ha fork el process ely 3aleha el dor          
                    if (pid == 0)
                    {
                        char rem[4];
                        sprintf(rem,"%d",RunningProcess.running_time);
                        char* const p[] = {"./process.o",rem,NULL};
                        execvp(p[0],p);

                    }

                    RunningProcess.pid = pid;
                    deQueue(q); // hashelo mn el ready queue
                }
            }
        }
        else if (RunningProcess.state == 1) // 3ande process bt run
        {
            if (getClk() - RunningProcess.cont >= Quantum) // law el quantum bta3ha kheles
            {
               // printf ("\nHere\n");
                if (q->front != NULL) // law el queue msh fady
                {
                    RunningProcess.last_stopped = getClk();
                    RunningProcess.state = 2;
                    RunningProcess.remaining_time = RunningProcess.remaining_time - Quantum;
                    if (RunningProcess.remaining_time !=0)
                    {
                        kill(RunningProcess.pid,SIGSTOP); // stop el running process                    
                        fprintf(outputFile,"At time %d process %d stopped arr %d total %d remain %d wait %d\n",getClk(),RunningProcess.id,RunningProcess.arrival_time,RunningProcess.running_time,RunningProcess.remaining_time,RunningProcess.wait_time);        
                        enQueue(q,RunningProcess); // hahotha f akher el queue
                    
                        if ( q->front->data.state == 2) // law el next fl queue kanet stopped abl keda
                        {
                    
                            RunningProcess = q->front->data;
                            RunningProcess.state = 1;
                            deQueue(q);
                            RunningProcess.cont = getClk();
                            RunningProcess.waiting_time += (getClk() - RunningProcess.last_stopped);
                            RunningProcess.wait_time += RunningProcess.waiting_time; 
                            fprintf(outputFile,"At time %d process %d resumed arr %d total %d remain %d wait %d\n",getClk(),RunningProcess.id,RunningProcess.arrival_time,RunningProcess.running_time,RunningProcess.remaining_time,RunningProcess.wait_time);                        
                            writeShM(RunningProcess.waiting_time);
                            kill(RunningProcess.pid,SIGCONT); // hakhaleha tkmel

                        }
                        else if (q->front->data.state== 0) // awl marra t run
                        {
                            RunningProcess = q->front->data;
                            RunningProcess.state = 1;
                            RunningProcess.start_time = getClk();   
                            RunningProcess.wait_time = RunningProcess.start_time - RunningProcess.arrival_time;
                            fprintf(outputFile,"At time %d process %d started arr %d total %d remain %d wait %d\n",getClk(),RunningProcess.id,RunningProcess.arrival_time,RunningProcess.running_time,RunningProcess.remaining_time,RunningProcess.wait_time);
                            // fork new process
                            writeShM(0);
                            int pid = fork(); // ha fork el process ely 3aleha el dor          
                            if (pid == 0)
                            {
                                char rem[4];
                                sprintf(rem,"%d",RunningProcess.running_time);
                                char* const p[] = {"./process.o",rem,NULL};
                                execvp(p[0],p);

                            }
                            RunningProcess.cont = getClk();  
                            RunningProcess.pid = pid;
                            deQueue(q); // hashelo mn el ready queue
                        } 
                    }  
                }
                else
                {
                    RunningProcess.cont = getClk();
                    RunningProcess.remaining_time = RunningProcess.remaining_time - Quantum;
                }
                
            }
            
        }
        
    }
    return Fq;
}   
void CollectChildrenRR(int signal)
{
    if(waitpid(-1,&stat_loc,WNOHANG))
    {
        if (WIFEXITED(stat_loc))
        {
            RunningProcess.finish_time = WEXITSTATUS(stat_loc);
            RunningProcess.turnaround_time = RunningProcess.finish_time - RunningProcess.arrival_time;
            RunningProcess.wait_time = RunningProcess.finish_time-RunningProcess.running_time - RunningProcess.arrival_time;
            RunningProcess.weighted_turnaround_time = (float)RunningProcess.turnaround_time / (float)RunningProcess.running_time;
            RunningProcess.remaining_time = 0;
            RunningProcess.state = -1; // harg3o ady tany
            fprintf(outputFile,"At time %d process %d finished arr %d  remain %d wait %d ",RunningProcess.finish_time,RunningProcess.id,RunningProcess.arrival_time,RunningProcess.remaining_time,RunningProcess.wait_time);
            fflush(stdout);   
            fprintf(outputFile,"TA %d WTA %0.2f \n",RunningProcess.turnaround_time,RunningProcess.weighted_turnaround_time);
            fflush(stdout);          
            enQueue(Fq,RunningProcess);
            NumberOfProcesses--;                 
            flag =0; // harag3 el flag b zero 3shan a22dar a fork tany
            
            int deAlloc = deallocateProcces(RunningProcess,memoryArray);
            fprintf(memoryFile,"At time %d freed %d from process %d from %d to %d\n",getClk(),RunningProcess.memSize,RunningProcess.id,memoryArray[deAlloc].startIndex,memoryArray[deAlloc].endIndex);

            struct Queue* temp = CreateQueue();           
            while (waitingQueue->front !=NULL)
            {
                
                int isAlloc =allocateProcess(waitingQueue->front->data,memoryArray);
                if (isAlloc != -1)
                {
                    fprintf(memoryFile,"At time %d allocated %d for process %d from %d to %d\n",getClk(),waitingQueue->front->data.memSize,waitingQueue->front->data.id,memoryArray[isAlloc].startIndex,memoryArray[isAlloc].endIndex);
                    enQueue(q,waitingQueue->front->data);
                    deQueue(waitingQueue);
                }
                else
                {
                    enQueue(temp,waitingQueue->front->data);
                    deQueue(waitingQueue);
                }          
            }
            while (temp->front !=NULL)
            {
                enQueue(waitingQueue,temp->front->data);
                deQueue(temp);
            }
        }
    }       
}

////////////////////////////////// SRTN ////////////////////////////////
struct Queue*  srtnQueue;
struct Queue* FsrtnQueue;
struct Queue* SRTN(int NumOfProcesses,struct memoryBlock * memArray)
{

    ///////
    memoryArray = memArray;
    initClk();
    NumberOfProcesses = NumOfProcesses;
    // if child terminate -> send signal "SIGCHLD " to parent
    signal(SIGCHLD, CollectChildren);
    // 1. Initiate Message Queue with PG and create data structure suitable for this algorithm
    int msgq_id = init_SCH_GEN_MQueue(); // to send the arrival processes 
    struct msgbuff message;
    // Ready Queue
    Head = (PNode*)malloc(sizeof(PNode));  // Create Node for Priority LinkedList  
    Head = NULL;

    ////////// Finish Queue
    Fqueue = CreateQueue();
    ////////
    waitingQueue = CreateQueue();

    ////////
    int lastReceived=-1;
    RunningProcess.state = -1 ; // No current process running
    memoryFile = fopen("memory.log","w");
    outputFile = fopen("scheduler.log","w");
    fprintf(outputFile,"#At time x process y state arr w total z remain y wait k\n");
    fprintf(memoryFile,"#At time x allocated y bytes for process z from i to j\n");
 
    while ( NumberOfProcesses > 0)
    {
        // RCV Message
        int rec = msgrcv(msgq_id, &message,sizeof(message.process),0, IPC_NOWAIT | MSG_NOERROR);
        if (rec != -1)
        {
            if (message.process.id != lastReceived) // 3shan lw fi process bt-run, m3mlsh enqueue le nafs el msg martein 
            {
                lastReceived = message.process.id;
                int isAlloc = allocateProcess(message.process,memoryArray);
                if (isAlloc == -1) // not allocated
                {
                    // ha7otha fl waiting queue , ashelha mn el ready queue
                    message.process.remaining_time = message.process.running_time; // intially Running Time = Arrival Time;  
                    message.process.state = 0;
                    message.process.waiting_time =0;
                    message.process.wait_time =0;
                    message.process.last_stopped=0;
                    enQueue(waitingQueue,message.process);
                }
                else
                {
                    fprintf(memoryFile,"At time %d allocated %d for process %d from %d to %d\n",getClk(),message.process.memSize,message.process.id,memoryArray[isAlloc].startIndex,memoryArray[isAlloc].endIndex);
                    message.process.remaining_time = message.process.running_time; // intially Running Time = Arrival Time;  
                    message.process.state = 0;
                    message.process.waiting_time=0;
                    message.process.last_stopped=0;
                    message.process.wait_time =0;
                    push(&Head,message.process,message.process.remaining_time);  // Priotiry according to remaining time           

                }

            }   
        }
         if (RunningProcess.state == -1) // No currenlty running process
        {
               if (Head != NULL) // Queue isn't empty
                {
                    if ( Head->data.state == 2) // Means that this process ran before and got stopped
                    { 
                        RunningProcess = Head->data;
                        RunningProcess.state = 1; // This process will be currently running
                        RunningProcess.id= Head->data.id;
                        pop(&Head); // Removing the proccess from the ready queue
                        RunningProcess.cont = getClk();
                        RunningProcess.waiting_time += (getClk() - RunningProcess.last_stopped);
                        writeShM(RunningProcess.waiting_time);
                        RunningProcess.wait_time = RunningProcess.wait_time + RunningProcess.waiting_time;
                        fprintf(outputFile,"At time %d process %d resumed arr %d total %d remain %d wait %d\n",getClk(),RunningProcess.id,RunningProcess.arrival_time,RunningProcess.running_time,RunningProcess.remaining_time,RunningProcess.wait_time);
                        
                        kill(RunningProcess.pid,SIGCONT); // Sending continue signal to the process
                        //printf("STATE= %d, PROCESS ID: %d\n", RunningProcess.state,RunningProcess.id);

                    }
                    else if (Head->data.state== 0) // First run for the process
                    {
                        RunningProcess = Head->data;
                        RunningProcess.state = 1;
                        RunningProcess.start_time = getClk();  
                        writeShM(0);
                        // fork new process
                        RunningProcess.wait_time = RunningProcess.start_time - RunningProcess.arrival_time;
                        fprintf(outputFile,"At time %d process %d started arr %d total %d remain %d wait %d\n",getClk(),RunningProcess.id,RunningProcess.arrival_time,RunningProcess.running_time,RunningProcess.remaining_time,RunningProcess.wait_time);
                        int pid = fork(); // Forking the process to create it          
                        if (pid == 0)
                        {
                            char rem[4];
                            sprintf(rem,"%d",RunningProcess.running_time);
                            char* const p[] = {"./process.o",rem,NULL};
                            execvp(p[0],p);

                        }
                        RunningProcess.pid = pid;
                        RunningProcess.cont= getClk();
                       // printf("ANA AWEL MARRA 2-RUN, CONT=%d\n",RunningProcess.cont);
                        pop(&Head); // Removing the proccess from the ready queue

                    }
                }
        }  
        else if ( RunningProcess.state == 1) // 3ande process bt-run
        {
            if (Head!= NULL) // law el queue msh fady
            {
                // Shortest remaining time of in the ready queue is < the currently running process
                if (Head->data.remaining_time < (RunningProcess.remaining_time - (getClk() - RunningProcess.cont)) ) 
                {
                  
                        kill(RunningProcess.pid,SIGSTOP); // stop el running process
                        RunningProcess.last_stopped = getClk();
                        RunningProcess.state = 2;
                        //printf("CONT=%d\n",RunningProcess.cont);
                        RunningProcess.remaining_time -= (getClk() - RunningProcess.cont); ///////////
                        fprintf(outputFile,"At time %d process %d stopped arr %d total %d remain %d wait %d\n",getClk(),RunningProcess.id,RunningProcess.arrival_time,RunningProcess.running_time,RunningProcess.remaining_time,RunningProcess.wait_time);

                        push(&Head,RunningProcess,RunningProcess.remaining_time);
                       
                        if ( Head->data.state == 2) // law el next fl queue kanet stopped abl keda
                        {
                    
                            RunningProcess = Head->data;
                            RunningProcess.state = 1;
                            pop(&Head);
                            RunningProcess.cont = getClk();
                            RunningProcess.waiting_time += (getClk() - RunningProcess.last_stopped);
                            
                            RunningProcess.wait_time += RunningProcess.waiting_time;
                           // RunningProcess.remaining_time = getClk
                            fprintf(outputFile,"At time %d process %d resumed arr %d total %d remain %d wait %d\n",getClk(),RunningProcess.id,RunningProcess.arrival_time,RunningProcess.running_time,RunningProcess.remaining_time,RunningProcess.wait_time);
                            writeShM(RunningProcess.waiting_time);
                            kill(RunningProcess.pid,SIGCONT); // hakhaleha tkmel

                        }
                        else if (Head->data.state== 0) // awl marra t run
                        {
                            RunningProcess = Head->data;
                            RunningProcess.state = 1;
                            RunningProcess.start_time = getClk();   
                            // fork new process
                            RunningProcess.wait_time = RunningProcess.start_time - RunningProcess.arrival_time;
                            fprintf(outputFile,"At time %d process %d started arr %d total %d remain %d wait %d\n",getClk(),RunningProcess.id,RunningProcess.arrival_time,RunningProcess.running_time,RunningProcess.remaining_time,RunningProcess.wait_time);
                            writeShM(0);
                            int pid = fork(); // ha fork el process ely 3aleha el dor          
                            if (pid == 0)
                            {
                                char rem[4];
                                sprintf(rem,"%d",RunningProcess.running_time);
                                char* const p[] = {"./process.o",rem,NULL};
                                execvp(p[0],p);

                            }
                            RunningProcess.pid = pid;
                            RunningProcess.cont= getClk();

                           // RunningProcess.cont = getClk();
                            pop(&Head); // hashelo mn el ready queue
                        }   
                }

            }
            
        }
    }
    return Fqueue;
}