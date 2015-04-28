/*
 * global simulation parameters
 */
typedef struct {
  int nusers;
  int nprinters;
  int arrtime;
  int maxpages;
  int bufsize;
  int nevents;
  int starttime;
  void *state;		/* dynamically initialized global state */
} SimParameters;

/*
 * each active simulation entity will have its own Agent structure
 */
typedef struct {
	SimParameters *p;	/* pointer to global sim parameters */
	void *v;		/* pointer to individual state record */
	int id;			/* integer id for this entity */
} Agent;

typedef struct {
  int jobsize;
  int userid;
  int jobid;
} Job;

extern void initialize_state(SimParameters *);

extern void submit_job(Agent *, Job *); 

extern Job *get_print_job(Agent *);

extern pthread_mutex_t print_lock;
extern int Ego();
