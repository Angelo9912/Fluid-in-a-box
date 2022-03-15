//----------------------------------------------------------------------
//------------------------PTASK LIBRARY---------------------------------
//-------------------PROJECT: SAND IN A BOX-----------------------------
//---------------TINFENA MATTIA & MASSARA ANGELO------------------------
//----------------------------------------------------------------------

//----------------------------------------------------------------------
// This library is developed on top of pthread library to simplify the 
// management of periodic tasks
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//--------------------------LIBRARIES-----------------------------------
//----------------------------------------------------------------------

#include <time.h>
#include <pthread.h>
#include <sched.h>
#include "ptask.h"

//----------------------------------------------------------------------
//---------------------GLOBAL COSTANTS----------------------------------
//----------------------------------------------------------------------

#define _GNU_SOURCE
#define NT			100		// Max number of task that is possible crate
#define NM			10		// Max number of semaphores that is possible create
#define STK_SIZE	40960	// Set the stack size to 4MB 

//----------------------------------------------------------------------
//---------------------GLOBAL VARIABLES---------------------------------
//----------------------------------------------------------------------

struct task_par		tp[NT];		// array of task parameters
pthread_t			tid[NT];	// array of task identifier
pthread_mutex_t		sem[NM];	// array of semaphores

int					ceiling[NM];
//---------------------------------
//           ceiling
// Is equal to the priority of the 
// highest-priority task that is 
// expected to used the mutex
//---------------------------------


//----------------------------------------------------------------------
//-----------------TIME MANAGEMENT FUNCTIONS----------------------------
//----------------------------------------------------------------------

static void time_copy (struct timespec *td, struct timespec ts)
{
	// Copy a timespec from a sorce time to a recipient time 
	// td recipient time, ts source time
	
	td->tv_sec = ts.tv_sec;
	td->tv_nsec = ts.tv_nsec;
}

static void time_add_ms (struct timespec *t, int ms)
{
	// Add a quantity of ms to a timespec

	t->tv_sec += ms/1000;				// we added ms to s
	t->tv_nsec += (ms%1000)*1000000;	// we added the rest to ns
	
	if(t->tv_nsec > 1000000000)
	{
		t->tv_nsec -= 1000000000;
		t->tv_sec += 1;
	}
}

static int time_cmp(struct timespec t1, struct timespec t2)
{
	//--------------------------------
	// Compare two timespec and:
	// Return 1 if ti > t2, 
	// Return -1 if t2 > t1, 
	// Return 0 if they are equal
	//--------------------------------
	
	if(t1.tv_sec > t2.tv_sec) return 1;
	if(t1.tv_sec < t2.tv_sec) return -1;
	if(t1.tv_nsec > t2.tv_nsec) return 1;
	if(t1.tv_nsec < t2.tv_nsec) return -1;
	return 0;
} 

void set_activation(int i)
{
	// Active a task

	struct timespec t;

	clock_gettime(CLOCK_MONOTONIC, &t);
	time_copy(&(tp[i].at), t);
	time_copy(&(tp[i].dl), t);
	time_add_ms(&(tp[i].at), tp[i].period);
	time_add_ms(&(tp[i].dl), tp[i].deadline);
}

int deadline_miss(int i)
{
	// Check if the task i have missed the deadline

	struct timespec now;

	clock_gettime(CLOCK_MONOTONIC, &now);
	
	if (time_cmp(now, tp[i].dl) > 0) 
	{
		tp[i].dmiss++;
		return 1;
	}
	return 0;
}

void wait_for_activation(int i)
{
	// Makes the task wait for the next activation

	clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &(tp[i].at), NULL);
	
	time_add_ms(&(tp[i].at), tp[i].period);
	time_add_ms(&(tp[i].dl), tp[i].period);

}	

//----------------------------------------------------------------------
//---------------------TASK MANAGEMENT FUNCTIONS------------------------
//----------------------------------------------------------------------


int task_create(void*(*task) (void*), int i, int period, int drel, int prio)
{
	// Create a task

	pthread_attr_t myatt;		// Standard attributes
	struct	sched_param mypar;	// Standard parameters
	int tret;
	
	if (i > NT) return -1;
	
	tp[i].arg = i;				// Task index
	tp[i].period = period;		// Task period
	tp[i].deadline = drel;		// Task relative deadline
	tp[i].priority = prio;		// Task priority from 1 to 99 
	tp[i].dmiss = 0;
	
	pthread_attr_init(&myatt);
	pthread_attr_setstacksize(&myatt, STK_SIZE);
	pthread_attr_setinheritsched(&myatt, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&myatt, SCHED_FIFO);
	mypar.sched_priority = tp[i].priority;
	pthread_attr_setschedparam(&myatt, &mypar);
	
	tret = pthread_create(&tid[i], &myatt, task, (void*)(&tp[i]));
	
	return tret;
}

int get_task_index(void* arg)
{
	// Returns the index of the task

	struct task_par *tpar;

	tpar = (struct task_par *)arg;
	return tpar->arg;
}

void wait_for_task_end (int i)
{
	// End the task

	pthread_join(tid[i],NULL);
}

//----------------------------------------------------------------------
//----------------SEMAPHORES MANAGEMENT FUNCTIONS-----------------------
//----------------------------------------------------------------------

int semaphore_init (int i, int func)
{
	// Create a semaphore

	pthread_mutexattr_t matt;
	pthread_mutexattr_init(&matt);
	
	if (func == STAND);
	else if (func == INHERIT)
		{
			pthread_mutexattr_setprotocol(&matt, PTHREAD_PRIO_INHERIT);
		}
	else if (func == CEILING)
		{
			pthread_mutexattr_setprotocol(&matt, PTHREAD_PRIO_PROTECT);
			pthread_mutexattr_setprioceiling(&matt, ceiling[i]);		
		}
	else return 1; //Invalid parameter, impossible setting the mutex 
	
	pthread_mutex_init(&sem[i], &matt);
	pthread_mutexattr_destroy (&matt);
	
	return 0;
	
}



