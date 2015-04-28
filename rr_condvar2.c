/*
 * CS170: rr_condvar.c
 * this code is a modification of race3.c to use condition variables to ensure
 * round robin scheduling using condition variables
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

int MyTurn = 0;

typedef struct 
{
	pthread_mutex_t *lock;
	pthread_cond_t *wait;
	int id; 
	int size;
	int iterations;
	char *s;
	int nthreads;
} Thread_struct;

void *infloop(void *x)
{
	int i, j, k;
	Thread_struct *t;
 
	t = (Thread_struct *) x;

	for (i = 0; i < t->iterations; i++) 
	{
		/*
		 * do try this at home
		 */
		pthread_mutex_lock(t->lock);
		while((MyTurn % t->nthreads) != t->id)
    		{
			pthread_cond_signal(t->wait);
	    		pthread_cond_wait(t->wait,t->lock);
    		}
		for (j = 0; j < t->size-1; j++) 
		{
			t->s[j] = 'A'+t->id;
			for(k=0; k < 50000; k++);	/* delay loop */
    		}
		t->s[j] = '\0';
		printf("Thread %d: %s\n", t->id, t->s);
		MyTurn++;
		pthread_cond_signal(t->wait);
		pthread_mutex_unlock(t->lock);
	}

	return(NULL);
}

int
main(int argc, char **argv)
{
	pthread_mutex_t lock;
	pthread_cond_t wait;
	pthread_t *tid;
	pthread_attr_t *attr;
	Thread_struct *t;
	void *retval;
	int nthreads, size, iterations, i;
	char *s;

	if (argc != 4) 
	{
		fprintf(stderr, "usage: race nthreads stringsize iterations\n");
		exit(1);
	}

	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&wait, NULL);
	nthreads = atoi(argv[1]);
	size = atoi(argv[2]);
	iterations = atoi(argv[3]);

	tid = (pthread_t *) malloc(sizeof(pthread_t) * nthreads);
	attr = (pthread_attr_t *) malloc(sizeof(pthread_attr_t) * nthreads);
	t = (Thread_struct *) malloc(sizeof(Thread_struct) * nthreads);
	s = (char *) malloc(sizeof(char *) * size);

	for (i = 0; i < nthreads; i++) 
	{
		t[i].nthreads = nthreads;
		t[i].id = i;
		t[i].size = size;
		t[i].iterations = iterations;
		t[i].s = s;
		t[i].lock = &lock;
		t[i].wait = &wait;
		pthread_attr_init(&(attr[i]));
		pthread_attr_setscope(&(attr[i]), PTHREAD_SCOPE_SYSTEM);
		pthread_create(&(tid[i]), &(attr[i]), infloop, (void *)&(t[i]));
  	}
	for (i = 0; i < nthreads; i++) 
	{
		pthread_join(tid[i], &retval);
	}

	return(0);
}
