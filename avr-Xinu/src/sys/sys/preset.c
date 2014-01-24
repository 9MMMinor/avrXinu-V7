/* preset.c - preset */

#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <ports.h>

extern void _ptclear();

/*
 *------------------------------------------------------------------------
 *  preset  --  reset a port, freeing waiting processes and messages
 *------------------------------------------------------------------------
 */
 
SYSCALL	preset(int portid, int (*dispose)())
{
	STATWORD ps;    
	struct	pt *ptptr;

	disable(ps);
	if ( isbadport(portid) ||
#ifdef	MEMMARK
	     unmarked(ptmark) ||
#endif
	     (ptptr= &ports[portid])->ptstate != PTALLOC ) {
		restore(ps);
		return(SYSERR);
	}
	_ptclear(ptptr, PTALLOC, dispose);
	restore(ps);
	return(OK);
}
