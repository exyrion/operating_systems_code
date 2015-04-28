#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "printqsim.h"

/*
 * describes a buffer used to queue jobs between users and printers
 */
typedef struct {
  Job **jobs;		/* array of Job pointers used as a queue */
  int head;		/* head of the queue */		
  int tail;		/* tail of the queue */
  int njobs;		/* number of jobs in queue */
  pthread_mutex_t *lock; /* lock for the queue */
} Buffer;
  
void initialize_state(SimParameters *p)
{
  Buffer *b;

  b = (Buffer *) malloc(sizeof(Buffer));
  b->jobs = (Job **) malloc(sizeof(Job *)*p->bufsize);
  b->head = 0;
  b->tail = 0;
  b->njobs = 0;
  b->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(b->lock, NULL);

  /*
   * Okay -- the quuue we've just made will be used by all agents
   *
   * point the simulation parameters structure at this state
   */
  p->state = (void *) b;
}

void submit_job(Agent *s, Job *j)
{
  Buffer *b;
  SimParameters *p;

  /*
   * get the global sim parameters from the agent
   */
  p = s->p;

  /*
   * get the buffer queue from the sim parameters
   */
  b = (Buffer *) p->state;

	while(1) 
	{

		/*
		 * lock this queue
		 */
		pthread_mutex_lock(b->lock);
		/*
		 * if there is space to add another job
		 */
		if (b->njobs < p->bufsize) 
		{
			/*
			 * add it at the head
			 */
			b->jobs[b->head] = j;
			b->head = (b->head + 1) % p->bufsize;
			/*
			 * bump the count
			 */
			b->njobs++;
			/*
			 * drop the lock
			 */
			pthread_mutex_unlock(b->lock);
			return;
		} 
		else	/* the queue is full -- for now, kill this thread */
		{
			/*
			 * drop the lock so we don't die holding
			 * it
			 */
			pthread_mutex_unlock(b->lock);
			printf("%4d: user %2d -- the queue is full -- exiting\n",
				time(0)-p->starttime, s->id);
			fflush(stdout);
			/*
			 * see ya
			 */
			pthread_exit(NULL);
		}
	}
	return;
}


Job *get_print_job(Agent *s)
{
	/*
	 * do nothing for now
	 */
  return NULL;
}
