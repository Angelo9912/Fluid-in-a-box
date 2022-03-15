//----------------------------------------------------------------------
//-----------------PTASK LIBRARY HEADER FILE----------------------------
//-------------------PROJECT: SAND IN A BOX-----------------------------
//--------------TINFENA MATTIA & MASSARA ANGELO-------------------------
//----------------------------------------------------------------------

#ifndef PTASK_H
#define PTASK_H

//----------------------------------------------------------------------
//----------------------NEEDED LIBRARIES--------------------------------
//----------------------------------------------------------------------

#include <pthread.h>

//----------------------------------------------------------------------
//---------------------GLOBAL COSTANTS----------------------------------
//----------------------------------------------------------------------

#define	STAND		0		// Classical semphore
#define INHERIT		1		// Priority inheritance protocol
#define CEILING		2		// Highest locker priority protocol

//----------------------------------------------------------------------
//--------------------STRUCTURES DECLARATION----------------------------
//----------------------------------------------------------------------

struct task_par
{
	int		arg;				// task index NOTE: the index MUST be different for each task
	long	wcet;				// worst case execution time in milliseconds
	int		period;				// in milliseconds
	int		deadline;			// relative in ms
	int		priority;			// [1,99]
	int		dmiss;				// no. of misses
	struct	timespec at;		// absolute activation time
	struct	timespec dl;		// absolute deadline
};

//----------------------------------------------------------------------
//------------GLOBAL EXTERNAL VARIABLES DECLARATION---------------------
//----------------------------------------------------------------------

extern struct task_par	tp[];
extern pthread_t		tid[];
extern int				ceiling[];
extern pthread_mutex_t	sem[];

//----------------------------------------------------------------------
//--------------TIME MANAGEMENT FUNCTIONS DECLARATION-------------------
//----------------------------------------------------------------------

extern void set_activation(int i);
extern int deadline_miss(int i);
extern void wait_for_activation(int i);

//----------------------------------------------------------------------
//-------------TASK MANAGEMENT FUNCTIONS DECLARATION--------------------
//----------------------------------------------------------------------

extern int task_create(void*(*task) (void*), int i, int period, int drel, int prio);
extern int get_task_index(void* arg);
extern void wait_for_task_end (int i);

//----------------------------------------------------------------------
//------------SEMAPHORES MANAGEMENT FUNCTIONS DECLARATION---------------
//----------------------------------------------------------------------

extern int semaphore_init (int i, int func);

#endif

