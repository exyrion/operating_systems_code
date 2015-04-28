#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "printqsim.h"

typedef struct {
  Job **jobs;
  int head;
  int tail;
  int njobs;
  pthread_mutex_t *lock;
  pthread_cond_t *full;
  pthread_cond_t *empty;
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
  b->full = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
  b->empty = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
  pthread_mutex_init(b->lock, NULL);
  pthread_cond_init(b->full, NULL);
  pthread_cond_init(b->empty, NULL);
  p->state = (void *) b;
}

void submit_job(Agent *s, Job *j)
{
	SimParameters *p;
  	Buffer *b;

	/*
	 * get the sim parameters from the agent
	 */
	p = s->p;
	/*
	 * get the queue from the sim parameters
	 */
  	b = (Buffer *) p->state;

	pthread_mutex_lock(b->lock);
	while(1)
	{
		/*
		 * if the job will fit
		 */
		if (b->njobs < p->bufsize)
		{
			/*
			 * insert it at the head
			 */
			b->jobs[b->head] = j;
			b->head = (b->head + 1) % p->bufsize;
			b->njobs++;
			/*
			 * signal anyone who is waiting
			 */
			pthread_cond_signal(b->empty);
			/*
			 * leave the critical section
			 */
			pthread_mutex_unlock(b->lock);
			return;
		} 
		else 	/* otherwise, wait until there is space and
			   we are signaled to proceed */
		{
      printf("%4d: user %2d blocking because the queue is full\n", 
             		time(0)-p->starttime, s->id);
      			fflush(stdout);
      			pthread_cond_wait(b->full, b->lock);
			/*
			 * when I wake up here, I have the lock and I'm
			 * back in the critical section
			 */
    		}
  	}
	return;
}


Job *get_print_job(Agent *s)
{
	SimParameters *p;
	Buffer *b;
  	Job *j;

	/*
	 * get the sim parameters
	 */
	p = s->p;
	/*
	 * get the buffer from the parameters
	 */
	b = (Buffer *)p->state;

	pthread_mutex_lock(b->lock);
	while(1) 
	{
		/*
		 * if there are jobs in the queue
		 */
		if (b->njobs > 0)
		{
			/*
			 * get the one at the tail
			 */
			j = b->jobs[b->tail];
			b->tail = (b->tail + 1) % p->bufsize;
			b->njobs--;
			/*
			 * signal any threads waiting because the queue
			 * is full
			 */
			pthread_cond_signal(b->full);
			/*
			 * leave the critical section
			 */
			pthread_mutex_unlock(b->lock);
			return j;
		} 
		else	/* otherwise wait until there are jobs available */ 
		{
	      printf("%4d: prnt %2d blocking because the queue is empty\n", 
				time(0)-p->starttime, s->id);
			fflush(stdout);
			pthread_cond_wait(b->empty, b->lock);
			/*
			 * when I'm here, I've been signaled because there
			 * are jobs in the queue.  Go try and get one
			 */
		}
  	}

	return;
}
