/*
 * types used by thread library
 */
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/mman.h>

//A TCB structure containing the thread context and additional info
struct tcb
{
	int thread_id;
	int thread_priority;
	ucontext_t *thread_context;
};

typedef struct tcb tcb;

//A structure symbolizing a node in the linked list representing the queue
struct QNode 
{ 
    tcb *tcb; //The thread control block associated with the node
    struct QNode *next; //Pointer to next node in the queue
}; 
typedef struct QNode QNode;

struct sem_t {
    int count;
    struct Queue *q;
};

typedef struct sem_t sem_t;

//Simple Queue data structure made up of QNodes
//Implemented using a linked list of QNodes
struct Queue 
{
	struct QNode *front, *rear; //Pointers to the head and tail of the queue
};
typedef struct Queue Queue;

struct messageNode {
    char *message;
    int len;
    int sender;
    int receiver;
    struct messageNode *next;
};
typedef struct messageNode messageNode;

struct mbox {
    struct messageNode *msg;
    sem_t *mbox_sem;
};
typedef struct mbox mbox;


struct QNode *newNode(tcb *tcb);
struct Queue *createQueue();
void enQueue(struct Queue *q, tcb *tcb);
struct QNode *deQueue(struct Queue *q);
void t_yield();
void t_init();
void t_create(void (*fct)(int), int id, int pri);
void t_shutdown();
void t_terminate();
int sem_init(sem_t **sp, int sem_count);
void sem_wait(sem_t *sem);
void sem_signal(sem_t *sem);
void sem_destroy(sem_t **sem);
int mbox_create(mbox **mb);
void mbox_destroy(mbox **mb);
void mbox_deposit(mbox *mb, char *msg, int len);
void mbox_withdraw(mbox *mb, char *msg, int *len);
void send(int tid, char *msg, int len);
void receive(int *tid, char *msg, int *len);
