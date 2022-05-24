
#include "allocation.h"
typedef struct Node{

PCB data; /// ?? el datatype eh?
struct Node* next;
} Node;
struct Queue
{
    Node* front;
    Node* rear;
};
struct Queue* CreateQueue()
{
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = NULL;
    q->rear = NULL;

    return q;

}
void enQueue(struct Queue* q,PCB data)
{
    Node* newNode = (Node*)malloc(sizeof(Node));
    newNode->data = data;
    newNode->next = NULL;
    if (q->rear == NULL) // el queue fady
    {
        q->front = newNode;
        q->rear = newNode;
    }
    else
    {
        q->rear->next = newNode;
        q->rear = newNode;
    }
}
void deQueue(struct Queue* q)
{
    if (q->front !=NULL)
    {
        Node* temp = q->front;
        q->front = temp->next;

        if (q->front == NULL)
            q->rear = NULL;
        
        free(temp);
    }
}
////////////////////////////// Priority Queue ////////////////////
typedef struct PNode{
PCB data;
int priority;
struct PNode* next;
} PNode;

void push(PNode** head,PCB data,int priority)
{
    // Ascending order
    // Create New Node
    PNode* newNode = (PNode*)malloc(sizeof(PNode)); 
    newNode->data = data;
    newNode->priority = priority;
    newNode->next = NULL;
    if ((*head) == NULL)
    {
        *head = newNode;
    }
    // if head > new yb2a el new hwa el head
    else if ((*head)->priority > newNode->priority)
    {
        newNode->next = *head;
        *head = newNode;
    }
    else
    {
        PNode* temp = *head;
        while (temp->next != NULL && temp->next->priority <= newNode->priority)
        {
            temp = temp->next;
        }
        newNode->next = temp->next;
        temp->next = newNode;
    }
    

}
void pop(PNode** head)
{
    PNode* temp = *head;
    *head = (*head)->next;
    free(temp);
}
PCB peek(PNode** head) // data bta3et awl element
{
    return (*head)->data;
}
