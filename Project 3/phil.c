#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#define THREADS 5

//cond array and chopsticks
enum {THINKING, HUNGRY, EATING} state[THREADS];
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int think_eat[THREADS][2];


void pickup(int i){ 	
	pthread_mutex_lock(&lock); 
	
	//initially hungry      
	state[i] = HUNGRY;
	
	//waits for chopsticks
	while (((state[(i + 4) % THREADS] == EATING) || (state[(i + 1) % THREADS] == EATING)) && (state[i] == HUNGRY)) { 	
	    	pthread_cond_wait(&cond, &lock);
    	}
    	
    	state[i] = EATING;
	pthread_mutex_unlock(&lock);
}     

void putdown (int i) { 	       
	pthread_mutex_lock(&lock);      
	
	// putdown so thinking 
    	state[i] = THINKING;   
    	
    	pthread_cond_signal(&cond);         
	pthread_mutex_unlock(&lock);
} 

void *philosopher(int* id){
	int thinktime = think_eat[*id][0];
	int think = 0;
	int eattime = think_eat[*id][1];
	int eat = 0;

	while (1) {
    		think = 0;
    		while (think < thinktime){
    			sleep(1);
    			think += 1;
    		}
		
		pickup(*id);
		
		eat = 0;
    		
    		printf("philosopher %d started eating now.\n", *id);
    		
    		while (eat < eattime){
    			sleep(1);
    			eat += 1;
    		}
    		
    		printf("philosopher %d finished eating now.\n", *id);
		
		putdown(*id);
	}
}

int main(){
	pthread_t tids[THREADS];
	int t_args[THREADS];	

	for(int i = 0; i < THREADS; i++) {
        	state[i] = THINKING;
		t_args[i] = i;

		srand(rand());
		
		think_eat[i][0] = (rand() % 10 + 1);
		think_eat[i][1] = (rand() % 5 + 1);
		printf("Philosopher no %d has thinking time %d and eating time %d\n", i, think_eat[i][0], think_eat[i][1]);
	}
	
	for(int i = 0; i < 5; i++) {
		int ret = pthread_create(&(tids[i]), NULL, philosopher, &t_args[i]);
		if (ret != 0) {
			printf("thread create failed \n");
			exit(1);
		} 
	}

    	for(int i = 0; i < 5; ++i){
    		pthread_join(*(tids + i), NULL);
    	}
	
	
	return 0; 
}

