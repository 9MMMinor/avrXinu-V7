/* kill.c - kill */

#include <avr-Xinu.h>
#include <sem.h>
#include <mem.h>
#include <io.h>

extern void xdone();
extern int resched();
extern int dequeue();
extern SYSCALL unsleep();

/*
 *------------------------------------------------------------------------
 * kill  --  kill a process, pid, and remove it from the system
 *				return (free) stack memory and stdin and stdout streams
 *------------------------------------------------------------------------
 */
 
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct pentry volatile *pptr;		/* points to proc. table for pid*/
	int	dev;

	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	for (dev = 0; dev < _NFILE && (pptr->fildes[dev] != (FILE *)FDFREE); dev++ )
		fclose(pptr->fildes[dev]);
	send(pptr->pnxtkin, pid);
	freestk(pptr->pbase, pptr->pstklen);
	
	switch (pptr->pstate) {

	case PRCURR:	pptr->pstate = PRFREE;	/* suicide */
					resched();
	                panic("kill() -- should never get here!!!!\n");
	                break;

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
					pptr->pstate = PRFREE;
					break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
					/* fall through	*/
	default:		pptr->pstate = PRFREE;
	}
	
	restore(ps);
	return(OK);
}
