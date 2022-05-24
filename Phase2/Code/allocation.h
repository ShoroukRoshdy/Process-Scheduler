#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h> 
#include <math.h>
#include "headers.h"

/////////////////////////////////////
//struct memoryBlock * memoryArray = (struct memoryBlock *)malloc(2047*sizeof(struct memoryBlock));


int get_Sibling(int node)
{
    if (node%2==0)
       return (node-1);
    else
        return (node+1); 
       
}

int get_Parent(int node)
{
   return ((node-1)/2);
}

int get_LeftChild(int node)
{
     return ((node*2)+1);
}


int get_RightChild(int node)
{
      return ((node*2)+2);
}

void allocateInTree(int node,struct memoryBlock * memoryArray)
{
   // If memory is allocated, i allocate all of its children, 3shan hya hya nafs el memory, duh
    if (node>2047)
    {
        return ;
    }

    memoryArray[node].alloc=true;
    allocateInTree(get_LeftChild(node), memoryArray);
    allocateInTree(get_RightChild(node),memoryArray);
}

void deallocateInTree(int node,struct memoryBlock * memoryArray)
{
     // If memory is deallocated, i deallocate all of its children, 3shan hya hya nafs el memory, duh
    if (node>2047)
    {
        return;
    }

    memoryArray[node].alloc=false;
    deallocateInTree(get_LeftChild(node),memoryArray);
    deallocateInTree(get_RightChild(node),memoryArray);
}



void createAllPossibleMemories(struct memoryBlock *memoryArray)
{

    int level = 0;
    int nodeCount = 0;
    int start=0;
    for (int i = 0; i < 2047; i++)
    {
        memoryArray[i].memorySize = 1024 / pow(2, level);
        memoryArray[i].startIndex = start;
        memoryArray[i].endIndex = start + (1024 / pow(2, level)) -1;
        memoryArray[i].alloc = false;
        
        start = start + (1024 / pow(2, level));
        nodeCount++;
        if (nodeCount == pow(2, level))
        {
            nodeCount=0;
            start = 0;
            level++;
        }
    }
 
}
/// 

int allocateProcess(PCB process,struct memoryBlock * memoryArray)
{
//-------------------------------- Get index in array  --------------------------------//
 
    int level = log2(1024/process.memSize);
    int blockSize = 1024/pow(2,level); // Get size in terms of powers of 2
    int i = (1024/blockSize)-1; // To get its index in the array of the left have of the memory
    int size= memoryArray[i].memorySize;
   
    int flag =0;
    while (size== memoryArray[i].memorySize)
    {
        if (memoryArray[i].alloc==false)
        {
            flag =1;
            break;
        }
        // Lw ana hna yb2a both halves are occupied
        i++; // To skip el sibling bta3i
    }
    if (flag == 0)
        i=-1; // No free memory was found
    //-------------------------------- allocate process --------------------------------//

    if (i!=-1) // Memory is free
    {
        memoryArray[i].alloc=true;
        memoryArray[i].pid= process.id;
        allocateInTree(i,memoryArray);
        int p= get_Parent(i);
        // Setting all parents with false, because part of the memory is occupied
        while (p>0) ////////////////////////////////
        {
            memoryArray[p].alloc=true;
            p=get_Parent(p);
        }
    }
   // else Then there's no place for the process
   return i;
   //////////////////////// 23MEL EH 27OTTO FL WAITING QUEUE
    
}

//-------------------------------- Merging Process --------------------------------//
void mergeMemories(int node,struct memoryBlock * memoryArray)
{

    while(node > 0)
    {
        if (memoryArray[get_Sibling(node)].alloc==false)
        {
    

            memoryArray[get_Parent(node)].alloc=false;
        }
        else
        {
            break;
        }
        

        node = get_Parent(node);
    } 
}
//-------------------------------- De-allocating process --------------------------------//
int deallocateProcces(PCB process,struct memoryBlock * memoryArray)
{
    // mem = 1024/2^l
    // mem * 2^l = 1024
    // 2^l = 1024/mem
    // l = log2(1024/mem)

   int level = log2(1024/process.memSize);
   int blockSize = 1024/pow(2,level); // Get size in terms of powers of 2

    int i = (1024/blockSize)-1; // To get its index in the array of the left have of the memory
    int size= memoryArray[i].memorySize;

    while (size== memoryArray[i].memorySize)
    {
        if (memoryArray[i].pid==process.id)
        {
            memoryArray[i].alloc=false;
            deallocateInTree(i,memoryArray);
            memoryArray[i].pid=-2;
            mergeMemories(i,memoryArray);
            break;
        }
        else // memory is occuppied
        {
            i++;
        }
    }
    return i;
}




