/* resched.c  -  resched */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <inttypes.h>

extern void panic();
extern int insert();
extern int getlast();
extern void ctxsw();
char * getSP(void);

/*------------------------------------------------------------------------
 * resched  --  reschedule processor to highest priority ready process
 *
 * Notes:	Upon entry, currpid gives current process id.
 *		Proctab[currpid].pstate gives correct NEXT state for
 *			current process if other than PRREADY.
 *------------------------------------------------------------------------
 */

int resched(void)
{
	register struct	pentry volatile *optr;	/* pointer to old process entry */
	register struct	pentry volatile *nptr;	/* pointer to new process entry */
	int newpid;

	preempt = QUANTUM;		/* reset preemption counter	*/

	/* no switch needed if current process priority higher than next */
	
	if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
             ( lastkey(rdytail) < optr->pprio) )	{
//		kprintf("resched: No Switch currpid=%d\n", currpid);
		return(OK);
	}
	
	/* force context switch */
	if (optr->pstate == PRCURR) {
		optr->pstate = PRREADY;
		insert(currpid,rdyhead,optr->pprio);
	}

	/* remove highest priority process at end of ready list */
	
	if ( (newpid = getlast(rdytail)) == EMPTY )
		return(EMPTY);
	
	nptr = &proctab[ ( currpid = newpid ) ];
	nptr->pstate = PRCURR;		/* mark it currently running	*/
//	kprintf("resched: Yes Switch currpid=%d\n", currpid);

	ctxsw(&optr->pregs[0],&nptr->pregs[0]);	/* switch context from old to new */

	return(OK);
}

/*
void dump_Stack(int pid)
{
    struct pentry *p;
    char *saddr;
    int j;

    p = &proctab[pid];
    if (pid == currpid)
	saddr = getSP()+8;
    else
	saddr = p->pSP;
    kprintf("\nProcess %d stack %p\n",pid,saddr);
    while ((unsigned)saddr < (unsigned)p->pbase)
    {
	j = *++saddr;
	kprintf("0X%02x\n",j&0xff);
    }
}
*/

char *getSP(void)
{
    char * mySP;

    asm volatile(
		 "in %A0, __SP_L__" "\n\t"
		 "in %B0, __SP_H__" "\n\t"
		 : "=d" (mySP)
		 :
		 );

    return (mySP+2);
}

