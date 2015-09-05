#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "gtthread.h"


/*Basic idea to prevent Deadlock: Let the philosopher on seat number 0,2,4 picked the left chopsticks first and 1,3 picked the right chopsticks first*/

typedef struct Seat				/*Data structure for main thread to pass information to the philosopher threads*/
{
	int seat_number;
	int chop1;			/*The first chopstick to pick*/
	int chop2;	         /*The second chopstick to pick*/
}Seat;

gtthread_mutex_t chopstick[5];   /*Five chopsticks*/
Seat seat[5];	/*Seat information for each philosoper*/
gtthread_t thread[5];

void *philosopher(void *arg){
	Seat *myseat = (Seat *) arg;		/*Get the information of my seat*/
	int random_num,i,j;
	printf("Philosopher #%d gets seated!\n", myseat->seat_number);
	while(1){
		
		/* Start thinking*/
		printf("Philosopher %d starts thinking!\n", myseat->seat_number);
        random_num = (rand()%50000);
        for(i = 0; i < random_num; i++)
            for(j = 0; j < 10000; j++)
                ;
		printf("Philosopher %d gets hungry!\n", myseat->seat_number);
		
		/* Acquire the first chopstick */
		gtthread_mutex_lock(&chopstick[myseat->chop1]);					
		if(myseat->chop1 == myseat->seat_number)
			printf("Philosopher %d acquires the left chopstick! \n",myseat->seat_number);
		else
			printf("Philosopher %d acquires the right chopstick! \n",myseat->seat_number);
		
		/* Acquire the second chopstick */
		gtthread_mutex_lock(&chopstick[myseat->chop2]);					/* Acquire the second chopstick */
		if(myseat->chop2 == myseat->seat_number)
			printf("Philosopher %d acquires the left chopstick! \n",myseat->seat_number);
		else
			printf("Philosopher %d acquires the right chopstick! \n",myseat->seat_number);
		
		/* Start eating */
		printf("Philosopher %d starts eating!\n",myseat->seat_number);
		random_num = (rand()%50000);
		for(i = 0; i < random_num; i++)
			for(j = 0; j < 10000; j++)
			;
	    
		/*Finish eating and release the chopstick*/
		
		/*Release the first chopstick*/
		gtthread_mutex_unlock(&chopstick[myseat->chop1]);
		if(myseat->chop1 == myseat->seat_number)
			printf("Philosopher %d releases the left chopstick! \n",myseat->seat_number);
		else
			printf("Philosopher %d releases the right chopstick! \n",myseat->seat_number);
		
		/*Release the second chopstick*/
		gtthread_mutex_unlock(&chopstick[myseat->chop2]);
		if(myseat->chop2 == myseat->seat_number)
			printf("Philosopher %d releases the left chopstick! \n",myseat->seat_number);
		else
			printf("Philosopher %d releases the right chopstick! \n",myseat->seat_number);	
	}
	 
}


int main(int argc, char **argv) {
	
	int i,left,right,checker;
	
	gtthread_init(100);
	/*Initialized the chopstick mutext*/
	for(i = 0; i < 5; i++){
		gtthread_mutex_init(&chopstick[i]);
	}
	
	/*Seed random number generator*/
	srand((unsigned int)time(NULL));  
	
	/*Initialized the seat information*/
	for(i = 0; i < 5; i++){
		seat[i].seat_number = i;
		left = i;				/*index of left chopstick*/
		right = (i + 1)%5;		/*index of right chopstick*/
		if(i%2 == 0){
			seat[i].chop1 = left;			/*for 0, 2, 4, pick left chopstick first*/
			seat[i].chop2 = right;
		}
		else{
			seat[i].chop1 = right;			/*for 1, 3, pick right chopstick firsts*/
			seat[i].chop2 = left;
		}	 
	}
	
	
	for(i = 0; i < 5; i++){		/*Initialized the 5 philosopher thread*/
		checker = gtthread_create(&thread[i], philosopher, &seat[i]);
        if(checker != 0)
            printf("Philosopher fails to get seated!\n");
	}
	
	gtthread_exit(NULL);
    return 0;
}