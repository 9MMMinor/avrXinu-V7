/* ready.c - ready */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>

int insert(int, int, int);
int resched(void);

/*
 *------------------------------------------------------------------------
 * ready  --  make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
 
int ready(int pid, int resch)
			/* id of process to make ready	*/
			/* reschedule afterward?	*/
{
	register struct	pentry volatile *pptr;

	if (isbadpid(pid))
		return(SYSERR);
	pptr = &proctab[pid];
	pptr->pstate = PRREADY;
	insert(pid,rdyhead,pptr->pprio);
	if (resch)
		resched();
	return(OK);
}
