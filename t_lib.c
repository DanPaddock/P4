#define _XOPEN_SOURCE 600
#include "t_lib.h"

//Running TCB pointer
struct tcb *running;

//Ready queue pointer
struct Queue *ready;
struct mbox *msgQueue;


/**
*@brief Performs a swapcontext() between the calling thread and the next thread in ready queue
*
*@return VOID
*/
void t_yield()
{
	//Declare a temporary tcb pointer for swapping
	struct tcb *tmp;
	
	//Set the currently running tcb to temp
	tmp = running;
	
	//Dequeue the next thread in ready
	struct QNode *new_running = deQueue(ready);

	if(new_running != NULL)
	{
		//Set running to point to TCB of the QNode we just dequeued
		running = new_running->tcb;
		
		free(new_running);
		
		//Enqueue the TCB that is pointed to by temp to back of ready queue
		enQueue(ready, tmp);
		
		//Swapcontext between the TCB we just enqueued and the one running points to
		swapcontext(tmp->thread_context, running->thread_context);
	}
}

/**
*@brief Initializes the Thread Library by creating an empty ready queue and the main TCB
*
*@return VOID
*/
void t_init()
{
	//Initialize the ready queue
	ready = createQueue();
	
	//Allocate memory for the main TCB
	tcb *tmp = (tcb *) malloc(sizeof(struct tcb));
	
	//Set its fields
	tmp->thread_id = 0;
	tmp->thread_priority = 0;
	
	//Allocate memory for the context of the main thread
	ucontext_t *uc;
	uc = (ucontext_t *) malloc(sizeof(ucontext_t));
	
	//Get the context of main
	getcontext(uc);
	
	//Set the thread_context field of tmp
	tmp->thread_context = uc;
	
	//Set running to point to tmp
	running = tmp;
}

/**
*@brief Creates a new thread with given pri, id and fct, adding to end of ready queue
*
*@param *fct: a pointer to a void function to be run by the thread
*
*@param id: an integer signifying the id of the thread to be created
*
*@param pri: an integer signifying the priority of the given thread
*
*@return the id of the created thread
*/
void t_create(void (*fct)(int), int id, int pri)
{
	//Size for stack allocation
	size_t sz = 0x10000;

	//Define a new ucontext and tcb for our new thread
	ucontext_t *uc;
	tcb *new_tcb;
	
	//Allocate space on the heap
	uc = (ucontext_t *) malloc(sizeof(ucontext_t));

	//Grab the current context so we can modify it
	getcontext(uc);

	//Set the new thread context appropriately
	uc->uc_stack.ss_sp = malloc(sz);
	uc->uc_stack.ss_size = sz;
	uc->uc_stack.ss_flags = 0;
	uc->uc_link = running; 
	
	//Make the thread context using the ucontext we just set up
	makecontext(uc, (void (*)(void)) fct, 1, id);
	
	//Setup the TCB using the ucontext we just created
	new_tcb = (tcb *) malloc(sizeof(tcb));
	new_tcb->thread_id = id;
	new_tcb->thread_priority = pri;
	new_tcb->thread_context = uc;
	
	//Add the TCB to the back of our queue
	//This simultaneously creates a new QNode with our TCB and adds to the back
	enQueue(ready, new_tcb);
	
	//Return the thread id upon success or errno otherwise
	//Right now we will just return  the thread id always
	//return id;
}

/**
*@brief Frees all memory associated with any structures (TCB, Queue etc)
*
*@return VOID
*/
void t_shutdown()
{
	//Declare a temp TCB, QNode and Queue structure for freeing
	struct tcb *tmp;
	struct QNode *qntmp = ready->front;
	
	//Loop through the ready Queue and free everything associated
	while(qntmp != NULL)
	{
		//Grab the TCB associated with the QNode
		tmp = qntmp->tcb;
		
		//Free the thread context of the TCB and the TCB
		if(tmp->thread_id > 0) {free(tmp->thread_context->uc_stack.ss_sp);}
		free(tmp->thread_context);
		free(tmp);
		
		//Free the QNode
		free(qntmp);
		
		qntmp = qntmp->next;
	}
	
	//Free the TCB associated with whatever thread is pointed to by running
	tmp = running;
	if(tmp->thread_id > 0) {free(tmp->thread_context->uc_stack.ss_sp);}
	free(tmp->thread_context);
	free(tmp);
	
	//Free the Ready Queue itself
	free(ready);
}

/**
*@brief Terminates the calling thread and swaps context to the next thread in running queue
*
*@return VOID
*/
void t_terminate()
{
	//Declare a temporary tcb pointer for swapping
	struct tcb *tmp;
	struct QNode *qntmp;
	
	//Set the currently running tcb to temp
	tmp = running;
	
	//Free the memory associated with the running thread
	free(tmp->thread_context->uc_stack.ss_sp);
	free(tmp->thread_context);
	free(tmp);
	
	//Grab the thread at the head of the queue
	qntmp = deQueue(ready);
	
	//Set running to point to this TCB
	running = qntmp->tcb;
	
	free(qntmp);
	
	//setcontext() to the new running thread
	setcontext(running->thread_context);
}

/**
*@brief Creates a new linked list node to add to the queue of nodes
*
*@param tcb: A Thread control block referencing the thread associated with the node
*
*@return A pointer to the newly created QNode
*/ 
// A utility function to create a new linked list node. 
struct QNode *newNode(tcb *tcb)
{
	//Create a new QNode and allocate space for it
    struct QNode *temp = (struct QNode*)malloc(sizeof(struct QNode)); 
	
	//Set the TCB and next fields
    temp->tcb = tcb; 
    temp->next = NULL; 
	
	//Return the new node
    return temp;  
}

/**
*@brief A utility function that creates an empty Queue and allocates memory for it
*
*@return A pointer to the newly created Queue
*/
struct Queue *createQueue() 
{
	//Allocate memory for a new queue
	struct Queue *q = (struct Queue*)malloc(sizeof(struct Queue)); 
	
	//Set the head and tail pointers to NULL
    q->front = q->rear = NULL; 
	
	//Return the new queue object
    return q; 
}

/**
*@brief Adds a thread control block to the back of the queue
*
*@param *q: A pointer to the queue the user wishes to add to
*
*@param tcb: A Thread control block referencing the thread to add
*
*@return VOID
*/ 
void enQueue(struct Queue *q, tcb *tcb) 
{ 
    //Create a new node to add to the list
    struct QNode *temp = newNode(tcb); 
  
    //Check if the queue is empty, if so front and rear are the same
    if (q->rear == NULL) 
    {
       q->front = q->rear = temp; 
       return; 
    }
  
    //Otherwise add the new node to the end of the queue
    q->rear->next = temp; 
    q->rear = temp; 
}

/**
*@brief Pops a QNode from the front of the queue
*
*@param *q: A pointer to the queue the user wishes to remove from
*
*@return A pointer to the QNode returned
*/ 
struct QNode *deQueue(struct Queue *q) 
{ 
    //If the queue is empty return NULL
    if (q->front == NULL) 
	{
       return NULL; 
	}
  
    //Store previous front and move front one node ahead 
    struct QNode *temp = q->front; 
    q->front = q->front->next; 
  
    //If front becomes NULL, then change rear to NULL as well
    if (q->front == NULL)
	{		
       q->rear = NULL; 
	}
	
	//Return the node we removed
    return temp; 
}

////////////////////
// Semaphore Code
///////////////////

/* Create a new semaphore pointed to by sp with a count value of sem_count. */
int sem_init(sem_t **sp, int sem_count)
{
    *sp = malloc(sizeof(sem_t));
    (*sp)->count = sem_count;
    (*sp)->q = createQueue();
}

 /* Current thread does a wait (P) on the specified semaphore. */
void sem_wait(sem_t *sem)
{
    tcb *tmp;
    sem->count--;
    
    if(sem->count < 0)
    {
        tmp = running;
        enQueue(sem->q, tmp);
        struct QNode *new_running = deQueue(ready);
        running = new_running->tcb;
        swapcontext(tmp->thread_context, running->thread_context);
        free(new_running);
        return;
    }
}

/* Current thread does a signal (V) on the specified semaphore. Follow the Mesa semantics (p. 9 of Chapter 30 Condition Variables) where the thread that signals continues, and the first waiting (blocked) thread (if there is any) becomes ready. */
void sem_signal(sem_t *sem)
{
    sem->count++;
    if(sem->count <= 0)
    {
        struct QNode *ready_q = deQueue(sem->q);
        enQueue(ready, ready_q->tcb);
        
        free(ready_q);
        return;
    }
}

/* Destroy (free) any state related to specified semaphore. */
void sem_destroy(sem_t **sem)
{
    struct tcb *tmp;
    struct QNode *qntmp = (*sem)->q->front;
    
    while(qntmp != NULL)
    {
        tmp = qntmp->tcb;
        
        if(tmp->thread_id > 0)
        {
            free(tmp->thread_context->uc_stack.ss_sp);
        }
        
        free(qntmp);
        free(tmp->thread_context);
        free(tmp);
        qntmp = qntmp->next;
    }
    
    free((*sem)->q);
    free(*sem);
    return;
}

 /* Create a mailbox pointed to by mb. */
int mbox_create(mbox **mb) {
    struct mbox* newMbox = (mbox *) malloc(sizeof(mbox));
    newMbox->msg = NULL;
    newMbox->mbox_sem = NULL;
    *mb = newMbox;
    return 1;
}

 /* Destroy any state related to the mailbox pointed to by mb. */
void mbox_destroy(mbox **mb) {
    struct mbox * tempBox = *mb;
    free(tempBox->msg);
    free(tempBox);
}

/* Deposit message msg of length len into the mailbox pointed to by mb. */
void mbox_deposit(mbox *mb, char *msg, int len) {
    struct messageNode * newMsg =(messageNode *) malloc(sizeof(messageNode));
    struct messageNode * headMsg = mb->msg;
    newMsg->message = malloc(len+1);
    strcpy(newMsg->message, msg);
    newMsg->len = len;
    newMsg->next = NULL;
    if (mb->msg == NULL) {
        mb->msg = newMsg;
    }
    else {
        while (headMsg->next) {
            headMsg = headMsg -> next;
        }
        headMsg->next = newMsg;
    }
}

 /* Withdraw the first message from the mailbox pointed to by mb into msg and set the message's length in len accordingly. The caller of mbox_withdraw() is responsible for allocating the space in which the received message is stored. If there is no message in the mailbox, len is set to 0. mbox_withdraw() is not blocking. Even if more than one message awaits the caller, only one message is returned per call to mbox_withdraw(). Messages are withdrew in the order in which they were deposited. */
void mbox_withdraw(mbox *mb, char *msg, int *len) {
    
   struct messageNode * headMsg = mb->msg;
    if (headMsg == NULL) {
        len = 0;
    }
    else {
        strcpy(msg, headMsg->message);
        len = headMsg->len;
    }
    if (mb->msg != NULL) {
        mb->msg = mb->msg->next;
        free(headMsg -> message);
        free(headMsg);
    }
}

/* Send a message to the thread whose tid is tid. msg is the pointer to the start of the message, and len specifies the length of the message in bytes. In your implementation, all messages are character strings. */
void send(int tid, char *msg, int len) {
    
    struct messageNode * sendMsg = (messageNode *) malloc(sizeof(messageNode));
    sendMsg->message = malloc(len+1);
    strcpy(sendMsg->message, msg);
    sendMsg->len = len;
    sendMsg->receiver = tid;
    sendMsg->sender = running->thread_id;
    sendMsg->next = NULL;
    
    running->msg = sendMsg;
   // sem_signal(msgQueue->mbox_sem);
}

/* Wait for and receive a message from another thread. The caller has to specify the sender's tid in tid, or sets tid to 0 if it intends to receive a message sent by any thread. If there is no "matching" message to receive, the calling thread waits (i.e., blocks itself). [A sending thread is responsible for waking up a waiting, receiving thread.] Upon returning, the message is stored starting at msg. The tid of the thread that sent the message is stored in tid, and the length of the message is stored in len. The caller of receive() is responsible for allocating the space in which the message is stored. Even if more than one message awaits the caller, only one message is returned per call to receive(). Messages are received in the order in which they were sent. The caller will not resume execution until it has received a message (blocking receive). */
void receive(int *tid, char *msg, int *len) {
    if(running->msg == msg && running->thread_id == tid){
        
        struct messageNode * recvMsg = (messageNode *) malloc(sizeof(messageNode));
        recvMsg->message = malloc(len+1);
        strcpy(recvMsg->message, msg);
        recvMsg->len = len;
        recvMsg->receiver = tid;
        recvMsg->sender = running->thread_id;
        recvMsg->next = NULL;
        
        if (running->thread_id == 0) {
            msgQueue->msg = recvMsg;
        } else {
            //sem_wait(msgQueue->mbox_sem);
            msgQueue->msg = recvMsg;
        }
    }
}

/* Send a message and wait for reception. The same as send(), except that the caller does not return until the destination thread has received the message. */
void block_send(int tid, char *msg, int length){
    
    struct messageNode * sendMsg = (messageNode *) malloc(sizeof(messageNode));
    sendMsg->message = malloc(length+1);
    strcpy(sendMsg->message, msg);
    sendMsg->len = length;
    sendMsg->receiver = tid;
    sendMsg->sender = running->thread_id;
    sendMsg->next = NULL;
    
    struct messageNode * headMsg = msgQueue->msg;
    
    block_receive(&tid, &msg, length);
    
    free(sendMsg->message);
    free(sendMsg->len);
    free(sendMsg->receiver);
    free(sendMsg->sender);
    free(sendMsg->next);
    free(sendMsg);
    free(headMsg);
    
    return;
}
void block_receive(int *tid, char *msg, int *length){
    if(&tid == tid){
        struct messageNode * recvMsg = (messageNode *) malloc(sizeof(messageNode));
        recvMsg->message = malloc(length+1);
        strcpy(recvMsg->message, msg);
        recvMsg->len = *length;
        recvMsg->receiver = *tid;
        recvMsg->sender = running->thread_id;
        recvMsg->next = NULL;
        
        free(recvMsg->message);
        free(recvMsg->len);
        free(recvMsg->receiver);
        free(recvMsg->sender);
        free(recvMsg->next);
        free(recvMsg);
    }
    
    return;
}

/*
 ////////////
 if (running->thread_id == tid) {
 return running->mailbox;
 } else {
 threadNode *currentNode;
 if (NULL != ready) {
 currentNode = ready->first;
 
 while (NULL != currentNode) {
 if (currentNode->thread_id == tid) {
 return currentNode->mailbox;
 }
 currentNode = currentNode->next;
 }
 
 }
 }
 return NULL;
 ///////
 
 void Producer(void){
 
 int item;
 Message m;
 
 while(1){
 
 receive(Consumer, &m);
 item = produce();
 build_message(&m , item ) ;
 send(Consumer, &m);
 }
 }
 void Consumer(void){
 
 int item;
 Message m;
 
 while(1){
 
 receive(Producer, &m);
 item = extracted_item();
 send(Producer, &m);
 consume_item(item);
 }
 }

 */

