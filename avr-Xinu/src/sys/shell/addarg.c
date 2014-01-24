/* addarg.c - addarg */

#include <avr-Xinu.h>
#include <shell.h>

/*
 *------------------------------------------------------------------------
 *  addarg  -  copy arguments to area reserved in proctab
 *			pid -- process to receive arguments 
 *			nargs -- number of arguments to copy 
 *			len -- size of arg. area (in bytes)
 *------------------------------------------------------------------------
 */
 
/* mmm	avr-gcc passes most arguments in registers. Not wanting to get into
		the details, and gcc's strict use of variable length argument passing,
		for avr-Xinu, create() creates new threads with just two arguments:
			newproc(int argc, int *argv).
		The arguments are stored in the newly created proctab (see proc.h).
		The only problem with this is the need to limit the number of arguments
		to MAXARG.
		Rather than copying arguments and pointers onto the process stack, addarg()
		stores the argument pointers in proctab.parg[narg] and does not copy
		argument strings at all.
 * mmm */
 
int addarg(int pid, int nargs) /* , int len) */
{
	struct pentry *pptr;
	int i;
//	char	**fromarg;
//	long	*toarg;
//	char	*to;

	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate != PRSUSP)
		return(SYSERR);
//	toarg = (long *) ( ((unsigned)pptr->pbase) - (unsigned)len );
//	to = (char *) (toarg + (nargs + 2));
//	*toarg++ = (long) (toarg + 1);
//	for (fromarg=Shl.shtok ; nargs > 0 ; nargs--) {
	for (i=0 ; i < nargs; i++) {
//		*toarg++ = to;
//		strcpy(to, *fromarg++);
//		to += strlen(to) + 1;
		pptr->parg[i+pptr->pargs] = (int)Shl.shtok[i]; /******** change int parg[] TO void *parg[] in proc.h ******/
	}
//	*toarg = 0;
	
	pptr->pargs += nargs;
	pptr->parg[pptr->pargs] = 0;
	/* machine/compiler dependent pass arguments to created process */
	pptr->pregs[24] = lobyte((unsigned)pptr->pargs);	/*r24*/
	pptr->pregs[25] = hibyte((unsigned)pptr->pargs);
	
	return(OK);
}
