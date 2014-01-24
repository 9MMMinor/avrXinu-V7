/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>

extern void insert();
extern int dequeue();
extern void resched();

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	int oldprio;
	struct pentry *const pptr = &proctab[pid]; /* mmm */

	disable(ps);
	if (isbadpid(pid) || newprio<=0 || pptr->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	oldprio = pptr->pprio;
	pptr->pprio = newprio;
	switch (pptr->pstate) {
	case PRREADY:
		insert( dequeue(pid), rdyhead, newprio);
	case PRCURR:
		resched();
	default:
		break;
	}
	restore(ps);
	return(oldprio);
}
