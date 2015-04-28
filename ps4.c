#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "printqsim.h"

/*
 * job queue structure
 */
typedef struct {
  Job **jobs;
  int head;
  int tail;
  int njobs;
  pthread_mutex_t *lock;
  pthread_cond_t *full;		/* condition variable controlling fullness */
  pthread_cond_t *empty;	/* condition variable controlling emptiness */
} Buffer;
  
void initialize_state(SimParameters *p)
{
  Buffer *b;
  pthread_attr_t attr;

  b = (Buffer *) malloc(sizeof(Buffer));
  b->jobs = (Job **) malloc(sizeof(Job *)*p->bufsize);
  b->head = 0;
  b->tail = 0;
  b->njobs = 0;
  b->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  /*
   * make space enough to hold condition variables
   */
  b->full = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
  b->empty = (pthread_cond_t *) malloc(sizeof(pthread_cond_t));
  /*
   * initialize the mutex and cond vars
   */
  pthread_mutex_init(b->lock, NULL);
  /*
   * if this is set to SCHED_FIFO the solution works
   */
  pthread_attr_setschedpolicy(&attr,SCHED_RR);
  pthread_cond_init(b->full, NULL);
  pthread_cond_init(b->empty, NULL);
  p->state = (void *)b;

  return;
}

void submit_job(Agent *s, Job *j)
{
	SimParameters *p;
  	Buffer *b;

	p = s->p;
	b = (Buffer *)p->state;

	/*
	 * lock this buffer so we can test under lock
	 */
	pthread_mutex_lock(b->lock);
	while(1)
	{
		/*
		 * if a new job will fit
		 */
		if (b->njobs < p->bufsize) 
		{
			/*
			 * enqueue it at the head
			 */
      			b->jobs[b->head] = j;
			/*
			 * bump the head pointer
			 */
      			b->head = (b->head + 1) % p->bufsize;
 			b->njobs++;
			/*
			 * if the queue was empty, signal a printer
			 * thread waiting for a job to arrive
			 */
      			if (b->njobs == 1) 
			{
				pthread_cond_signal(b->empty);
			}
			/*
			 * drop the lock -- we are leaving the critical
			 * section
			 */
      			pthread_mutex_unlock(b->lock);
			/*
			 * job successfully queued, bail out
			 */
      			return;
    		} 
		else /* the queue is full -- we must wait until it has space */ 
		{
      		printf("%4d: user %2d blocking because the queue is full\n", 
             		time(0)-p->starttime, s->id);
      			fflush(stdout);
			/*
			 * wait here -- printer thread will signal
			 * when there is space
			 */
      			pthread_cond_wait(b->full, b->lock);
			/*
			 * when we wake up here, we are holding the lock
			 * and we are the only thread in the critical section
			 */
		}
    	}
}


Job *get_print_job(Agent *s)
{
	SimParameters *p;
  	Buffer *b;
  	Job *j;

	p = s->p;
	b = (Buffer *)p->state;


	/*
	 * lock this buffer -- we are going to mess with it
	 */
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
			 * if the buffer was full before we took this
			 * job off the queue, we must signal any waiting
			 * user threads
			 */
			if (b->njobs == p->bufsize-1) 
			{
				pthread_cond_signal(b->full);
			}
			/*
			 * those threads won't run (if they are there)
			 * until we leave the critical section
			 *
			 * drop the lock to get out
			 */
      			pthread_mutex_unlock(b->lock);
      			return j;
		}
		else /* there are no jobs in the queue -- 
			wait until there are */
		{
      printf("%4d: prnt %2d blocking because the queue is empty\n", 
             		time(0)-p->starttime, s->id);
    			fflush(stdout);
			/*
			 * wait here until a user job signals that the
			 * queue is no longer empty
			 */
      			pthread_cond_wait(b->empty, b->lock);
			/*
			 * when we wake up, we know that a user has signaled
			 * us.  We are holding the lock and the only thread
			 * in the critical section
			 */
    		}
  	}
}
