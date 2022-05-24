#include "headers.h"

int main()
{
    // create semaphore between consumer and producer for the CS
    int mutex = Create_BinarySemaphore(); // init with 1
    int full = Create_FullSemaphore(); // init with 0
    int empty = Create_EmptySemaphore(); // init with BUF_SIZE
    // Create  (Shared Memory) Buffer to hold data 
    Create_Buffer(); 
    // Ceate (Shared Memory) index to hold current number of elements in the buffer
    Create_indexConsumer();

    while(1)
    {

        printf("\n ------------------------------------ \n");          
        // check if there's something to consume in the buffer
        // if not -> blocks until the producer produces an item
        down(full); 
        // check if it can enter the SHM (CS)
        // it blocks if another process in the CS
        down(mutex);
        // consumes item at index
        printf("\n Consumer : Consumes : %d from index %d\n",buffer[(*indexConsumer)],(*indexConsumer)); 
        if ((*indexConsumer) < BUF_SIZE -1)
            (*indexConsumer)++; // decrement number of items in the buffer
        else
            (*indexConsumer) =0;
        
        // leave Shared Memory
        up(mutex); 
        // increment number of empty slots in the buffer
        up(empty);        
        sleep(10);
    }
}
