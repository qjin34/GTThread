
#include "gtthread.h"


static gtthread_t *thread_table[MAX_THREAD]; 		/*Hashtable like table to quickly locate thread with index*/
	
/*Define static global variable that are not accessible from the outside*/
int current_state;            /*Very important, used to differentiate between library mode and application mode*/
static long sched_period;      /*In microseconds*/
static Queue readyQ; 				   /* Queue for thread that are ready*/
static Queue waitQ;				   /* Queue for thread that are waiting for other thread*/
static gtthread_t Main; 				/* The context of the main thread*/
static struct itimerval tick;				/*Used for reset the clock*/
static struct itimerval extra;               /*Used for giving thread in library mode more time to execute*/
static struct sigaction action;

static int threadIDcount = 0;

void schedule();
void ret_catcher(void*(*start_routine(void*)), void* para);
void print_threadTable();


void schedule(){
	
	if(current_state == LIB){		/*if in the library mode, give it extra time to finish and return; no thread switching */
		setitimer(ITIMER_VIRTUAL, &extra, NULL);
		return;
    }
	
	if(readyQ.length == 0){			/*When there is no thread left*/
		exit(0);
	}
	
	if(readyQ.length == 1){					/* When there is only one thread (or none?), do nothing */
		setitimer(ITIMER_VIRTUAL, &tick, NULL);		/* Timer one more time !!!!*/
		return;
	}
	
	gtthread_t *current = readyQ.head;		/* Use the next thread in the queue as head , and place the current thread to end of the queue*/
	readyQ.head = readyQ.head->next;
	readyQ.tail->next = current;
	readyQ.tail = readyQ.tail->next;    
	readyQ.tail->next = NULL;
	setitimer(ITIMER_VIRTUAL, &tick, NULL);		/* Timer one more time !!!!*/
	swapcontext(&(current->context), &(readyQ.head->context));			/* Swap the head with the next next node*/
}



void gtthread_init(long period){
	
	current_state = LIB;    /*In user mode*/
	
	int i;
	
	/*Initialize the thread table*/
	for(i = 0; i < MAX_THREAD; i++){
		thread_table[i] = NULL;
	}
	
	
	/*Get the context of the main thread*/
	getcontext(&(Main.context));
	thread_table[threadIDcount] = &Main;     /*Place the main thread to the thread table*/
	Main.next = NULL;
	Main.ID = threadIDcount;
    threadIDcount++;
	Main.exit_value = NULL;
	Main.status = READY;
	Main.waiting_thread = NULL; 


	/*initialize the queue and put the main thread into the queue*/
	sched_period = period;
	readyQ.length = 1;
	readyQ.head = &Main;
	readyQ.tail = &Main;
	waitQ.length = 0;
	waitQ.head = NULL;
	waitQ.tail = NULL;
	
	/*(Perhaps use another kind of timer instead?)Set the timer for the round robin scheduler*/
	
	
	action.sa_handler = schedule;  
	action.sa_flags = 0;
	
	sigemptyset(&action.sa_mask);

	memset(&tick, 0, sizeof(tick));
	tick.it_value.tv_sec = 0;  
	tick.it_value.tv_usec = period;  
	tick.it_interval.tv_sec = 0;
	tick.it_interval.tv_usec = 0;	/*One time timer */
	
	extra.it_value.tv_sec = 0; 
	extra.it_value.tv_usec = 20;   /* Give the thread in the system mode 5% more time to execute */
	extra.it_interval.tv_sec = 0;
	extra.it_interval.tv_usec = 0;      /* One time timer too */
   	
	sigaction(SIGVTALRM, &action, NULL);
	setitimer(ITIMER_VIRTUAL, &tick, NULL);

	current_state = APP;    /*Go back to APP mode*/
}

void ret_catcher(void*(*start_routine(void*)), void* para)					/*Used to get the return value of a thread*/
{
	void* retval;
	retval = start_routine(para);
	gtthread_exit(retval);
}

int gtthread_create(gtthread_t *thread, void *(*start_routine)(void *),void *arg){
	
	current_state = LIB;    /*In lib mode*/
	getcontext(&(thread->context));  
	
	thread_table[threadIDcount] = thread;           /*Register the thread on the thread table*/
	/*Set a newly allocated stack to the context*/
	thread->context.uc_link = NULL;
	thread->context.uc_stack.ss_sp = malloc(SIGSTKSZ);
	thread->context.uc_stack.ss_size = SIGSTKSZ;
	thread->context.uc_stack.ss_flags = 0;
	thread->ID = threadIDcount;
	threadIDcount++;
	thread->exit_value = NULL;
	thread->status = READY;
	thread->waiting_thread = NULL;     /* Must initialized as NULL!!!*/
	
	if(thread->context.uc_stack.ss_sp == 0){     /*When the memory allocation failed*/
		return -1;
	}
	
	/*append the the function to the thread*/
	makecontext(&thread->context,(void *)ret_catcher, 2, start_routine, arg);
	
	
	/*Add the node to the ready queue*/
	if(readyQ.length == 0){		  /*When the ready queue is empty*/
		if(readyQ.head != NULL || readyQ.tail != NULL)   /*error processing*/
			return -1;  
		readyQ.head = readyQ.tail = thread;
		readyQ.length++;
	}
	else{						/*When not empty, put the newly created thread to the end of the queue*/
		readyQ.tail->next = thread;
		readyQ.tail = readyQ.tail->next;
		readyQ.tail->next = NULL;
		readyQ.length++;
	}
	
	current_state = APP;    /*go back to app mode*/
	return 0;    /*successfully return*/
}

void print_threadTable()
{
	int i=0;
	while(thread_table[i] != NULL){
		printf("Thread %d in table\t", i);
		i++;
	}
	printf("\n");
	
}

int gtthread_join(gtthread_t thread, void **status){
	current_state = LIB;
	gtthread_t *real_thread = thread_table[thread.ID];
	/*If the thread is already over*/  
	if(real_thread->status == FINISHED || real_thread->status == CANCELED){			
		if(status != NULL){
				*status = real_thread->exit_value;		/*(check if this is OK) give pointer to the exit value*/
			}
		current_state = APP;
		return 0; 							/* Successfully join!*/
	}
	
	gtthread_t *current = readyQ.head;		
	readyQ.head = readyQ.head->next;
	readyQ.length--;    
	waitQ.length++;		
	
	real_thread->waiting_thread = current;
	current->status = WAIT;              /*change the status to wait*/
	current->next = NULL;
	
	setitimer(ITIMER_VIRTUAL, &tick, NULL);		/* Timer one more time !!!!*/
	current_state = APP;  
	swapcontext(&(current->context), &(readyQ.head->context));			/* Swap the current context with the next context in the queue*/	
	current_state = LIB;
	if(status != NULL){
		*status = real_thread->exit_value;
	}
	current_state = APP; 
	return 0;		
}

gtthread_t gtthread_self(void){
	return *readyQ.head;
}



void gtthread_exit(void *retval){     /*no parameter at first */
	current_state = LIB;
	
	if(readyQ.length == 1 && waitQ.length == 0){   /* it's the final thread */
	  		exit(0);	
	}
	
	readyQ.head->exit_value = retval;            /*put the return pointer to the retval*/
	readyQ.head->status = FINISHED;              /*Mark the thread as finsihed*/
	
	if(readyQ.head->waiting_thread != NULL && (readyQ.head->waiting_thread->status) != CANCELED){	/*When there is another thread that is waiting for this thread*/
		readyQ.tail->next = readyQ.head->waiting_thread;    /*Put the waiting thread back to the tail of readyQ*/
		readyQ.tail = readyQ.tail->next;
		readyQ.tail->status = READY;
		readyQ.length++;						/* Number of ready thread increases */
		waitQ.length--;   						/* Number of wait thread decreases */
	}
	
	readyQ.head = readyQ.head -> next;
	readyQ.length--;
	setitimer(ITIMER_VIRTUAL, &tick, NULL);	  /* Give the next thread a new time slice */
	current_state = APP;
	setcontext(&(readyQ.head->context));
		//}
	
}


int  gtthread_equal(gtthread_t t1, gtthread_t t2){
	if(t1.ID == t2.ID)
		return 1;
	else
		return 0;	
}

int gtthread_yield(void){
	schedule();
	return 0;
}

int  gtthread_cancel(gtthread_t thread){
	current_state = LIB;
	gtthread_t *p;
	gtthread_t *real_thread = thread_table[thread.ID];			/*Point to the thread immediately*/
	
	if(thread.ID == readyQ.head->ID){
		current_state = APP;
		return -1;						/*Error: Cannot cancel itself*/
	}
	
	
	if(real_thread->status == READY){						/* When the thread to be canceled is in the ready queue */
		real_thread->status = CANCELED;		
		readyQ.length--;
		p = readyQ.head;
		while(p->next != real_thread){			/*identify the node before the canceled thread*/
			p = p->next;
		}
		if(real_thread == readyQ.tail){			/*When the canceled thread is on the tail of readyQ*/
			readyQ.tail = p;
			readyQ.tail -> next = NULL;
		}
		else{							/*When the canceled thread is not on the tail of readyQ */
			p -> next = real_thread->next;
		}		
	}
	else if(real_thread->status == WAIT){	/*when the thread to be canceled is in the wait state, change it to canceled state and decreate the waitQ.length*/
		real_thread->status = CANCELED;
		waitQ.length--;
	}
	/*when the thread to be canceled is already finished or canceled, do nothing*/
	
	current_state = APP;
	return 0;
}









