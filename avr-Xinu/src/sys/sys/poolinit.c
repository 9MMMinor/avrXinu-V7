/* poolinit.c - poolinit */

#include <conf.h>
#include <kernel.h>
#include <mark.h>
#include <bufpool.h>

struct	bpool	bptab[NBPOOLS];
int	nbpools;
#ifdef	MEMMARK
MARKER	bpmark;				/* self initializing mark	*/
#endif

/*------------------------------------------------------------------------
 *  poolinit  --  initialize the buffer pool routines
 *------------------------------------------------------------------------
 */

SYSCALL
poolinit(void)
{
#ifdef	MEMMARK
	STATWORD ps;    

	int	status;

	disable(ps);
	if ( (status=mark(bpmark)) == OK) {
		nbpools = 0;
	}
	restore(ps);
	return( (status==OK) ? OK : SYSERR );
#else
	nbpools = 0;
	return(OK);
#endif
}






