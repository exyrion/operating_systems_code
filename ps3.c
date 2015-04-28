#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "printqsim.h"

/*
 * queue of jobs
 */
typedef struct {
  Job **jobs;		/* array of pointers to jobs */
  int head;		/* head of queue */
  int tail;		/* tail of queue */
  int njobs;		/* number of jobs */
  pthread_mutex_t *lock;	/* lock for this queue */
} Buffer;
  
void initialize_state(SimParameters *p)
{
  Buffer *b;

  /*
   * make space for this queue
   */
  b = (Buffer *) malloc(sizeof(Buffer));
  /*
   * make space to hold an array of pointers to jobs
   */
  b->jobs = (Job **) malloc(sizeof(Job *)*p->bufsize);
  b->head = 0;
  b->tail = 0;
  b->njobs = 0;
  b->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(b->lock, NULL);

  /*
   * point the global sim parameter structure to this queue structure
   */
  p->state = (void *) b;
}

void submit_job(Agent *s, Job *j)
{
	SimParameters *p;
  	Buffer *b;


	/*
	 * get the global sim parameters
	 */
	p = s->p;
	/*
	 * get the globally defined queue structure
	 */
  	b = (Buffer *)p->state;

	while(1) 
	{
		/*
		 * lock this queue
		 */
		pthread_mutex_lock(b->lock);
		/*
		 * if there is space in the que for another job
		 */
		if (b->njobs < p->bufsize) 
		{
			/*
			 * add it at the head
			 */
			b->jobs[b->head] = j;
			b->head = (b->head + 1) % p->bufsize;
			b->njobs++;
			/*
			 * drop the lock and bail
			 */
			pthread_mutex_unlock(b->lock);
			return;
		} 
		else /* otherwise, sleep for a second and try again */
		{
			/*
			 * !!! drop the lock so we don't go to sleep holding
			 * it
			 */
			pthread_mutex_unlock(b->lock);
			printf("%4d: user %2d sleeping because the queue is full\n",
			time(0)-p->starttime, s->id);
			fflush(stdout);
			sleep(1);
		}
	}
}


Job *get_print_job(Agent *s)
{
	SimParameters *p;
  	Buffer *b;
  	Job *j;

	/*
	 * get the sim parameters from the agent
	 */
	p = s->p;

	/*
	 * get the job queue buffer from the global sim parameters
	 */
	b = (Buffer *)p->state;

	/*
	 * do forever
	 */
	while(1)
	{
		/*
		 * lock the queue
		 */
		pthread_mutex_lock(b->lock);
		/*
		 * if there are jobs waiting in the queue
		 */
		if (b->njobs > 0) 
		{
			/*
			 * get the job pointer at the tail of the lost
			 */
			j = b->jobs[b->tail];
			/*
			 * bump the tail around
			 */
      			b->tail = (b->tail + 1) % p->bufsize;
			/*
			 * decrement the number of jobs in the queue
			 */
			b->njobs--;
			/*
			 * we've got the job safely off the queue, drop the
			 * lock
			 */
			pthread_mutex_unlock(b->lock);
			/*
			 * return the job
			 */
      			return j;
    		} 
		else /* otherwise, the queue is empty */
		{
			/*
			 * drop the lock so we don't sleep with it
			 */
      			pthread_mutex_unlock(b->lock);
      		printf("%4d: prnt %2d sleeping because the queue is empty\n",
             		time(0) - p->starttime, s->id);
      			fflush(stdout);
			/*
			 * sleep for a second -- maybe there will be more jobs
			 * when we wake up
			 */
      			sleep(1);
    		}
		/*
		 * loop back and try again
		 */
  	}
  
	return;
}
