#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

int main(int agrc, char * argv[])
{
    initClk();
    initSharedMemory();
    //TODO it needs to get the remaining time from somewhere
    // Get remaining time from argv -> initial = Running Time
    remainingtime =0;
    int start =  getClk();
    int wait=0;
    int last =0;
    //printf("\n start clk: %d\n",getClk());
    while (remainingtime < atoi(argv[1]))
    {
        remainingtime = (getClk() - start - readShM() );

    } 
   // printf("\n Clk Now: %d\n",getClk());
    exit(getClk());

    destroyClk(false);

    
    return 0;
}
