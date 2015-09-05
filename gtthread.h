#ifndef __GTTHREAD_H
#define __GTTHREAD_H

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>   
#include <string.h>   
#include <sys/time.h>


/*Used for the timer to differentiate user program and library*/
#define LIB 0        /*Library Mode*/
#define APP 1		  /*App Mode*/

/*Used for state of thread*/
#define WAIT 0		/*Thread is waiting other thread*/
#define READY 1		/*Thread is ready but not finished*/
#define FINISHED 2   /*Thread is finished*/	
#define CANCELED 3  /*Thread is canceled*/

/*Used for mutex*/
#define UNLOCKED 0
#define LOCKED 1

#define MAX_THREAD 1000   /*Assume there are at most 1000 threads in the program*/

typedef struct gtthread_t				/*Node for scheduling*/
{
	ucontext_t context;  
	int ID;									/*ID of the thread*/
	void *exit_value;             			/*pointer to the exit value*/
	struct gtthread_t *next;				/*pointer to the next thread in the scheduling queue */
	struct gtthread_t *waiting_thread;      /*pointer to the thread that is waiting for the current thread (Used for join)*/
	int status;                            /*status of of current thread (Ready/Waiting/Finished)*/
}gtthread_t;


typedef struct{
 	gtthread_t *head;
	gtthread_t *tail;
	int length;	
}Queue;


typedef struct{			/*Used for mutext*/
	int state;
	int ownerID;         /*The thread ID of the mutext owner*/
}gtthread_mutex_t;


/* Must be called before any of the below functions. Failure to do so may
 * result in undefined behavior. 'period' is the scheduling quantum (interval)
 * in microseconds (i.e., 1/1000000 sec.). */
void gtthread_init(long period);

/* see man pthread_create(3); the attr parameter is omitted, and this should
 * behave as if attr was NULL (i.e., default attributes) */
int  gtthread_create(gtthread_t *thread,
                     void *(*start_routine)(void *),
                     void *arg);

/* see man pthread_join(3) */
int  gtthread_join(gtthread_t thread, void **status);

/* gtthread_detach() does not need to be implemented; all threads should be
 * joinable */

/* see man pthread_exit(3) */
void gtthread_exit(void *retval);

/* see man sched_yield(2) */
int gtthread_yield(void);

/* see man pthread_equal(3) */
int  gtthread_equal(gtthread_t t1, gtthread_t t2);

/* see man pthread_cancel(3); but deferred cancelation does not need to be
 * implemented; all threads are canceled immediately */
int  gtthread_cancel(gtthread_t thread);

/* see man pthread_self(3) */
gtthread_t gtthread_self(void);


/* see man pthread_mutex(3); except init does not have the mutexattr parameter,
 * and should behave as if mutexattr is NULL (i.e., default attributes); also,
 * static initializers do not need to be implemented */
int  gtthread_mutex_init(gtthread_mutex_t *mutex);
int  gtthread_mutex_lock(gtthread_mutex_t *mutex);
int  gtthread_mutex_unlock(gtthread_mutex_t *mutex);

/* gtthread_mutex_destroy() and gtthread_mutex_trylock() do not need to be
 * implemented */

#endif // __GTTHREAD_H
