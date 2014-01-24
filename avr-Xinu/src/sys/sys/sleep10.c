/* sleep10.c - sleep10 */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <sleep.h>

extern int resched();
extern int insertd();

/*
 *------------------------------------------------------------------------
 * sleep10  --  delay the caller for a specified number of clock ticks n
 * sleep10(TICK); is guaranteed to put the process to sleep for 1 second
 *------------------------------------------------------------------------
 */

SYSCALL	sleep10(int n)
{
	STATWORD ps;
    
	if (n < 0  || clkruns==0)
		return(SYSERR);
	disable(ps);
	if (n == 0) {		/* sleep10(0) -> end time slice */
		;
	} else {
		insertd(currpid,clockq,n);
		slnempty = TRUE;
		sltop = (int *volatile) &q[q[clockq].qnext].qkey;
		proctab[currpid].pstate = PRSLEEP;
	}
	resched();
	restore(ps);
	return(OK);
}
