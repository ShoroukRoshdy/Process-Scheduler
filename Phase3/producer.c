#include "headers.h"

int main()
{
    int Seq_Num = (getpid() + getppid()) %100; 
    
    printf("\nIntial Seq_Num %d\n",(getpid() + getppid()) %100);
    
    // create semaphore between consumer and producer for the CS
    int mutex = Create_BinarySemaphore(); // init with 1
    int full = Create_FullSemaphore(); // init with 0
    int empty = Create_EmptySemaphore(); // init with BUF_SIZE
    // Create  (Shared Memory) Buffer to hold data 
    Create_Buffer(); 
    // Ceate (Shared Memory) index to hold current number of elements in the buffer
    Create_index();

    while(1)
    {
        printf("\n ------------------------------------ \n");          
        // check if there's empty slots in the buffer
        // if not -> blocks until the consumer consumes an item
        down(empty); // decrement empty count
        // check if it can enter the SHM (CS)
        // it blocks if another process in the CS
        down(mutex);
        //////////// Produce item
        buffer[index[0]] = Seq_Num; 
        printf("\n Producer produced %d at index %d\n",buffer[(*index)],(*index));
        Seq_Num++;
        if ((*index) < BUF_SIZE -1)
            (*index)++; // decrement number of items in the buffer
        else
            (*index) =0;
        // leave Shared Memory
        up(mutex); 
        // increment number of items in the buffer
        up(full); 

        sleep(1);
    }
}
