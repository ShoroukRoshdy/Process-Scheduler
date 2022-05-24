#include "Algorithms.h"
#include <math.h>
int main(int argc, char * argv[])
{
   // Reading Algo , Quantum from argument list
    int Algo=atoi(argv[1]);
    int Quantum=atoi(argv[2]),rec_val;
    int NumberOfProcesses = atoi(argv[3]);
    initClk();
    initSharedMemory();
    writeShM(0);
    //TODO implement the scheduler :)
   // 1. Choose Algorithm to work with
   
   if (Algo == 1)
   {
      struct Queue* Finishqueue = HPF(NumberOfProcesses);
      fclose(outputFile);
      int finishTime = getClk();
      double avgWTA =0,Avgwait =0,Std =0,std_sum =0;
      int count = 0,runningTimes =0;
      struct Queue* temp = CreateQueue();
      while(Finishqueue->front != NULL)
      {
         avgWTA += Finishqueue->front->data.weighted_turnaround_time;
         Avgwait += Finishqueue->front->data.wait_time;
         count++;
         runningTimes = runningTimes + Finishqueue->front->data.running_time;
         enQueue(temp,Finishqueue->front->data);
         deQueue(Finishqueue);
      }
      double cpu = ((double)runningTimes/(double)finishTime) * 100;
      avgWTA = avgWTA /count;
      Avgwait = Avgwait /count;
      while (temp->front !=NULL)
      {
         std_sum += pow((temp->front->data.weighted_turnaround_time - avgWTA),2);
         deQueue(temp);
      }
      Std = sqrt(std_sum/count);
      FILE* out = fopen("scheduler.perf","w");
      fprintf(out,"CPU utilization = %0.2f %%\n",cpu);      
      fprintf(out,"Avg WTA = %0.2f\n",avgWTA);
      fprintf(out,"Avg Waiting = %0.2f\n",Avgwait);
      fprintf(out,"Std WTA = %0.2f\n",Std);
      fclose(out);
      


   } 
   else if (Algo == 2) 
   {
     struct Queue* Finishqueue =  SRTN(NumberOfProcesses);
      fclose(outputFile);
      int finishTime = getClk();
      double avgWTA =0,Avgwait =0,Std =0,std_sum =0;
      int count = 0,runningTimes =0;
      struct Queue* temp = CreateQueue();
      while(Finishqueue->front != NULL)
      {
         avgWTA += Finishqueue->front->data.weighted_turnaround_time;
         Avgwait += Finishqueue->front->data.wait_time;
         count++;
         runningTimes = runningTimes + Finishqueue->front->data.running_time;
         enQueue(temp,Finishqueue->front->data);
         deQueue(Finishqueue);
      }
      double cpu = ((double)runningTimes/(double)finishTime) * 100;
      avgWTA = avgWTA /count;
      Avgwait = Avgwait /count;
      while (temp->front !=NULL)
      {
         std_sum += pow((temp->front->data.weighted_turnaround_time - avgWTA),2);
         deQueue(temp);
      }
      Std = sqrt(std_sum/count);
      FILE* out = fopen("scheduler.perf","w");
      fprintf(out,"CPU utilization = %0.2f %%\n",cpu);
      fprintf(out,"Avg WTA = %0.2f\n",avgWTA);
      fprintf(out,"Avg Waiting = %0.2f\n",Avgwait);
      fprintf(out,"Std WTA = %0.2f\n",Std);
      fclose(out);
   }  
   else if(Algo == 3)
   {
     struct Queue* Finishqueue =  RR(NumberOfProcesses,Quantum);
      fclose(outputFile);
      int finishTime = getClk();
      double avgWTA =0,Avgwait =0,Std =0,std_sum =0;
      int count = 0,runningTimes =0;
      struct Queue* temp = CreateQueue();
      while(Finishqueue->front != NULL)
      {
         avgWTA += Finishqueue->front->data.weighted_turnaround_time;
         Avgwait += Finishqueue->front->data.wait_time;
         count++;
         runningTimes = runningTimes + Finishqueue->front->data.running_time;
         enQueue(temp,Finishqueue->front->data);
         deQueue(Finishqueue);
      }
      double cpu = ((double)runningTimes/(double)finishTime) * 100;
      avgWTA = avgWTA /count;
      Avgwait = Avgwait /count;
      while (temp->front !=NULL)
      {
         std_sum += pow((temp->front->data.weighted_turnaround_time - avgWTA),2);
         deQueue(temp);
      }
      Std = sqrt(std_sum/count);
      FILE* out = fopen("scheduler.perf","w");
      fprintf(out,"CPU utilization = %0.2f %%\n",cpu);
      fprintf(out,"Avg WTA = %0.2f\n",avgWTA);
      fprintf(out,"Avg Waiting = %0.2f\n",Avgwait);
      fprintf(out,"Std WTA = %0.2f\n",Std);
      fclose(out);
   
   }
    
    //upon termination release the clock resources.
   destroyShM();
   destroyClk(true);
}

