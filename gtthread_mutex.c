#include "gtthread.h"
extern int current_state;

int  gtthread_mutex_init(gtthread_mutex_t *mutex){				/*Initial the mutex as unlocked first*/
	current_state = LIB;
	mutex->state = UNLOCKED;	/*Unlocked at first*/
	mutex->ownerID = -1;      	/*No one own the mutex at first*/
	current_state = APP;
	return 0;
}

int  gtthread_mutex_lock(gtthread_mutex_t *mutex){
	while(1){
		current_state = LIB;
		if(mutex -> state == UNLOCKED){
			mutex -> state = LOCKED;
			mutex -> ownerID = gtthread_self().ID;
			break;
			}	
		current_state = APP; 
		gtthread_yield();     /*Give up the CPU when the state is locked*/
	   }
	current_state = APP;
	return 0;	
}


int  gtthread_mutex_unlock(gtthread_mutex_t *mutex){
	current_state = LIB;
	if(mutex -> ownerID != gtthread_self().ID || mutex -> state == UNLOCKED){ 
		current_state = APP;					    /*(1)cannot unlocked the thread that doesn't belong to itself*/
		return -1;								    /*(2)cannot unlocked a already unlocked mutex*/
	}
		
	mutex -> ownerID = -1;
	mutex -> state = UNLOCKED;
	current_state = APP;
	return 0;
}