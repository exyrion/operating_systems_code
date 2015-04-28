#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "printqsim.h"

pthread_mutex_t print_lock;
SimParameters Parameters;

int Ego()
{
        int return_val;

        return_val = (int)pthread_self();

        return(return_val);
}


void *user_thread(void *arg)
{
	Agent *s;
	SimParameters *p;
	int i;
	int sleeptime;
	int jobsize;
	Job *j;

	/*
	 * get the Agent for this entity structure
	 */
	s = (Agent *) arg;

	/*
	 * from the agent struct, get the global simulation parameters
	 */
	p = s->p;

	/*
	 * a user submits nevents number of print jobs -- this is the main loop
	 *
	 * nevents is a global simulation parameter
	 */
	for (i = 0; i < p->nevents; i++) 
	{
		/*
		 * pick a random sleep time based on the parameter arrtive
		 */
		sleeptime = random()%(p->arrtime*2) + 1;
		printf("%4d: user %2d/%03d: Sleeping for %2d seconds\n", 
		time(0)-p->starttime, s->id, i, sleeptime);
		fflush(stdout);
		/*
		 * put this thread to sleep
		 */
		sleep(sleeptime);
		/*
		 * sleep done, let's make a job
		 */
		j = (Job *) malloc(sizeof(Job));
		/*
		 * random job size
		 */
		j->jobsize = (random()%p->maxpages) + 1;
		/*
		 * the job belongs to this thread (user)
		 */
		j->userid = s->id;
		/*
		 * we'll label the job with this iteration number
		 */
		j->jobid = i;
		printf("%4d: user %2d/%03d: Submitting a job with size %d\n", 
		time(0)-p->starttime, s->id, i, j->jobsize);
		fflush(stdout);
		/*
		 * call the submit job routine passing the queue structure and
		 * the job as arguments
		 */
		submit_job(s, j);
	}

	/*
	 * here, all of the jobs have been submitted -- exit
	 */
	printf("%4d: user %2d/%03d: Done\n", 
	  time(0)-p->starttime, s->id, i);
	fflush(stdout);
	return NULL;

}

/* Assume 4 seconds a page */

void *printer_thread(void *arg)
{
	Agent *s;
	SimParameters *p;
	int jobsize, userid, jobid;
	int i;
	Job *j;

	/*
	 * get he agent structure for this agent
	 */
	s = (Agent *)arg;
	/*
	 * get the simulation parameters for the entire simulation
	 */
	p = s->p;

	i = 0;
	/*
	 * printers run for ever
	 */
	while(1) 
	{
		printf("%4d: prnt %2d/%03d: ready to print\n",
			time(0)-p->starttime, s->id, i);
		fflush(stdout);
		/*
		 * get the next print job
		 */
		j = get_print_job(s);
		/*
		 * if there isn't one, we are finished
		 */
		if (j == NULL) 
		{
			printf("%4d: prnt %2d/%03d: Done\n",
				time(0)-p->starttime, s->id, i);
			fflush(stdout);
			pthread_exit(NULL);
		}
		/*
		 * otherwise, simulate printing a job by sleeping
		 * for 4 * the jobsize seconds
		 */
		printf("%4d: prnt %2d/%03d:",
			time(0)-p->starttime, s->id, i);
		printf(" Printing job %3d from user %2d size %3d\n", 
		j->jobid, j->userid, j->jobsize);
		fflush(stdout);
		sleep(4*j->jobsize);
		/*
		 * throw away the job structure since the job
		 * has been simulated
		 */
		free(j);
		i++;
	}
}

/*
 * simulation engine
 */
main(int argc, char **argv)
{
	Agent *s;
	pthread_t *user_tids;
	pthread_t *printer_tids;
	pthread_attr_t attr;
	int i;

	/*
	 * first, check to see if there are enough arguments
	 */
	if (argc != 7) {
	fprintf(stderr, 
	    "usage: printqsim nusers nprinters arrtime maxpages bufsize nevents\n");
	exit(1);
	}

	/*
	 * initialize a global mutex lock for the entire simulation
	 */
	pthread_mutex_init(&print_lock, NULL);

	/*
	 * parse the arguments and pit them in a global simulation
	 * structure variable called Parameters.  Its purpose is to
	 * hold the simulation parameters for the entire simulation
	 */
	Parameters.nusers = atoi(argv[1]);
	Parameters.nprinters = atoi(argv[2]);
	Parameters.arrtime = atoi(argv[3]);
	Parameters.maxpages = atoi(argv[4]);
	Parameters.bufsize = atoi(argv[5]);
	Parameters.nevents = atoi(argv[6]);
	Parameters.starttime = time(0);
	/*
	 * seed the random number generator
	 */
	srandom(Parameters.starttime);

	/*
	 * call the initialize routine
	 */
	initialize_state(&Parameters);

	/*
	 * there will be one pthread for each user and one pthread for
	 * each printer.  Need to make the space to hold the thread
	 * identifiers
	 */
	user_tids = (pthread_t *) malloc(sizeof(pthread_t)*Parameters.nusers);
	printer_tids = (pthread_t *) malloc(sizeof(pthread_t)*Parameters.nprinters);

	/*
	 * create the user threads
	 */
	for (i = 0; i < Parameters.nusers; i++) 
	{
		/*
		 * make an agent structure for this user
		 */
		s = (Agent *)malloc(sizeof(Agent));
		/*
		 * point this agent at the global simulation parameters
		 */
		s->p = &Parameters;

		/*
		 * give this simulation structure its own id
		 */
		s->id = i;
		/*
		 * make this thread pre-emptable
		 */
		pthread_attr_init(&attr);
		pthread_attr_setscope(&attr,PTHREAD_SCOPE_SYSTEM);
		/*
		 * launch this user thread passing in the simulation structure
		 * for this user as an argument
		 */
		pthread_create(&(user_tids[i]), 
				&attr, 
			       user_thread, 
			       (void *) s);
	}

	/*
	 * now create the printer threads
	 */
	for (i = 0; i < Parameters.nprinters; i++) 
	{
		/*
		 * make an agent structure for each printer
		 */
		s = (Agent *)malloc(sizeof(Agent));

		/*
		 * point the printer at the global parameters
		 */
		s->p = &Parameters;

		/*
		 * give this printer its own id
		 */
		s->id = i;
		/*
		 * make this thread pre-emptable
		 */
		pthread_attr_init(&attr);
		pthread_attr_setscope(&attr,PTHREAD_SCOPE_SYSTEM);
		/*
		 * launch this printer thread passing in the 
		 * simulation structure for this printer as an argument
		 */
		pthread_create(&(printer_tids[i]), &attr,
				printer_thread, (void *) s);
	}

	/*
	 * at this point, we have one thread running for each user
	 * and one running for each printer, each thread has a copy
	 * of the simulation parameters and its own simulation id -- there
	 * is nothing left for the main thread to do
	 */

	/*
	 * the main thread is done -- exit
	 */
	for(i=0; i < Parameters.nprinters; i++)
	{
		(void)pthread_join(printer_tids[i],NULL);
	}
	for(i=0; i < Parameters.nusers; i++)
	{
		(void)pthread_join(user_tids[i],NULL);
	}
	pthread_exit(NULL);
}
  
