/*
 * CS170: 
 * adder.c -- each thread adds the value and increment passed as a 
 * struct thread_arg and returns the sum as a struct thread_return
 */

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

#define THREADS 4

struct thread_arg
{
	int value;
	int increment;
};

struct thread_return
{
	int return_value;
};

int Ego()
{
	int return_val;

	return_val = (int)pthread_self();

	return(return_val);
}

void *AddIt(void *arg)
{
	struct thread_arg *my_arg;
	struct thread_return *my_return;

	my_arg = (struct thread_arg *)arg;

	my_return = 
		(struct thread_return *)malloc(sizeof(struct thread_return));
	if(my_return == NULL)
		exit(1);

	printf("Hi.  I'm thread %d and I'm adding %d to %d\n", 
		Ego(),
		my_arg->increment,
		my_arg->value);

	my_return->return_value = my_arg->value + my_arg->increment;

	free(my_arg);

	
	pthread_exit((void *)my_return);

	return(NULL);
}

int
main()
{
	int i;
	int *vals;
	pthread_t *tid_array;
	int err;
	struct thread_arg *arg;
	struct thread_return *r;

	vals = (int *)malloc(THREADS*sizeof(int));
	if(vals == NULL)
	{
		exit(1);
	}

	tid_array = (pthread_t *)malloc(THREADS*sizeof(pthread_t));
	if(tid_array == NULL)
	{
		exit(1);
	}

	for (i = 0; i < THREADS; i++) 
	{
		arg = (struct thread_arg *)malloc(sizeof(struct thread_arg));
		if(arg == NULL)
			exit(1);

		arg->value = i;
		arg->increment = 10;
		err = pthread_create(&(tid_array[i]),
				     NULL,
				     AddIt,
				     (void *)arg);

		if(err != 0)
		{
			fprintf(stderr,"thread %d ",i);
			perror("on create");
			exit(1);
		}
			
    	}

	printf("main thread %d doing no adding\n",
			Ego());

	for (i = 0; i < THREADS; i++) 
	{

		printf("I'm %d Trying to join with thread %d\n", 
			Ego(),(int)tid_array[i]);
    		pthread_join(tid_array[i], (void **)&r);
    		printf("%d Joined with thread %d, ret. value %d\n", 
			Ego(),(int)tid_array[i],
			r->return_value);
		free(r);
  	}

	free((void *)tid_array);

	return(0);
}
