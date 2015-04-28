/*
 * hw.c -- hello world with posix threads
 *
 */

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

void *printme(void *arg)
{
	printf("Hello world\n");
	return NULL;
}

int
main()
{
	pthread_t tcb;
	void *status;
	int err;

	err = pthread_create(&tcb, NULL, printme, NULL);

	if (err != 0) 
	{
		perror("pthread_create");
    		exit(1);
  	}

	err = pthread_join(tcb, &status);
	if (err != 0) 
	{ 
		perror("pthread_join"); 
		exit(1); 
	}

	return(0);

}
