#include "headers.h"


int main()
{
    Create_Buffer();
    Create_index();
    Create_indexConsumer();
    int mutex = Create_BinarySemaphore(); // init with 1
    int full = Create_FullSemaphore(); // init with 0
    int empty = Create_EmptySemaphore(); // init with BUF_SIZE
    destroy();
    return 0;
}