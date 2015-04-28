#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "printqsim.h"

void initialize_state(SimParameters *p)
{
  pthread_mutex_lock(&print_lock);
  fprintf(stdout,"thread-%d, initialize_v: dummy version called\n",
			Ego());
  /*
   * no dynamically initialized global state in this example
   */
  p->state = NULL;
  pthread_mutex_unlock(&print_lock);

}

void submit_job(Agent *s, Job *j)
{
  pthread_mutex_lock(&print_lock);
  fprintf(stdout,"thread-%d, submit_job: dummy version called\n",
			Ego());
  fprintf(stdout,"thread-%d\treturning\n",
			Ego());
  fflush(stdout);
  pthread_mutex_unlock(&print_lock);
  return;
}


Job *get_print_job(Agent *s)
{
  pthread_mutex_lock(&print_lock);
  fprintf(stdout,"thread-%d, get_print_job: dummy version called\n",
			Ego());
  fprintf(stdout,"thread-%d\treturning\n",
			Ego());
  fflush(stdout);
  pthread_mutex_unlock(&print_lock);
  return NULL;
}
