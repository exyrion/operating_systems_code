/*
 * CS170: preempt2.c
 * adapted from code by Jim Plank
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/*
 * fewer threads than processors
 */
#define THREADS 3



void *infloop(void *x)
{
	double f;
	int i, j, *id;
 
	id = (int *) x;

	i = 0;
	while(1)
	{
    		printf("thread %d.  i = %10d\n", *id, i);
    		f = 1;
    		for (j = 0; j < 50000; j++) 
		{
      			f = f*.99;
    		}
		i++;
  	}
	pthread_exit(NULL);
}

int
main()
{
	pthread_t t[THREADS];
	pthread_attr_t attr[THREADS];
	int i[THREADS];
	void *retval;
	int j;

	
	for(j=0; j < THREADS; j++) {
		i[j] = j;
		i[j] = j;
		pthread_create(&(t[j]), NULL, infloop, (void *)&(i[j]));
	}

	for(j=0; j < THREADS; j++) {
		pthread_join(t[j], &retval);
	}

	return(0);
}
