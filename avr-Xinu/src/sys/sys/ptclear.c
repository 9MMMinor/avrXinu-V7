/* ptclear.c - _ptclear */

#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <ports.h>

extern SYSCALL sreset(int, int);
extern SYSCALL sdelete(int);

/*
 *------------------------------------------------------------------------
 *  _ptclear  --  used by pdelete and preset to clear a port
 *------------------------------------------------------------------------
 */

void _ptclear(struct pt *ptptr, int newstate, int (*dispose)())
{
	struct	ptnode	*p;

	/* put port in limbo until done freeing processes */
	ptptr->ptstate = PTLIMBO;
	ptptr->ptseq++;
	if ( (p=ptptr->pthead) != (struct ptnode *)NULLPTR ) {
		for(; p != (struct ptnode *) NULLPTR ; p=p->ptnext)
			(*dispose)( p->ptmsg );
		(ptptr->pttail)->ptnext = ptfree;
		ptfree = ptptr->pthead;
	}
	if (newstate == PTALLOC) {
		ptptr->pttail = ptptr->pthead = (struct ptnode *) NULLPTR;
		sreset(ptptr->ptssem, ptptr->ptmaxcnt);
		sreset(ptptr->ptrsem, 0);
	} else {
		sdelete(ptptr->ptssem);
		sdelete(ptptr->ptrsem);
	}
	ptptr->ptstate = newstate;
}
