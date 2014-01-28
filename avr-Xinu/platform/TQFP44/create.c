/* create.c - create, newpid */

#include <stdio.h>    
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>

LOCAL   newpid();
extern int open(int, char *, char *);
extern SYSCALL freemem(void *block, unsigned size);
extern FILE *fdopen(int fdes, char *mode);
extern FILE kprint_out;

/*
 *------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
 
SYSCALL create(int (*procaddr)(), int ssize, int priority, char *name, int nargs, ...)
{
	STATWORD ps;    
	int pid;				/* stores new process id	*/
	struct pentry *pptr;	/* pointer to proc. table entry */
	int i;
	unsigned char *saddr;			/* stack address		*/
	int INITRET();
	va_list ap;

	disable(ps);
	ssize = (int) roundmb(ssize);
	if ( (saddr = (unsigned char *)getstk(ssize)) == (unsigned char *)SYSERR ) {
	    restore(ps);
	    return (SYSERR);
	}
	if ( ssize < MINSTK || (pid=newpid()) == SYSERR || priority < 1 ) {
	    freestk((unsigned)saddr, (unsigned)ssize);
	    restore(ps);
	    return(SYSERR);
	}
	numproc++;
	pptr = &proctab[pid];
	
	/* stderr set to &kprint_out (see kprintf.c and initialize.c) */
	pptr->fildes[0] = fdopen(CONSOLE, "r");	/* stdin set to console */
	pptr->fildes[1] = fdopen(CONSOLE, "w");	/* stdout set to console */
	pptr->fildes[2] = &kprint_out;			/* stderr set to &kprint_out */

	for (i=2; i < _NFILE; i++)			/* others set to unused */
		pptr->fildes[i] = (FILE *)FDFREE;

	pptr->pstate = PRSUSP;
	for (i=0 ; i<PNMLEN && (int)(pptr->pname[i]=name[i])!=0 ; i++)
		;
	pptr->pprio = priority;
	pptr->pbase = saddr;
	pptr->pstklen = ssize;
	pptr->psem = 0;
	pptr->phasmsg = FALSE;
	pptr->plimit = pptr->pbase - ssize + sizeof (WORD);	
	*saddr-- = (char)MAGIC;		/* Bottom of stack */
	pptr->pargs = nargs;
	for (i=0 ; i<PNREGS ; i++)
		pptr->pregs[i] = INITREG;	
	pptr->paddr = (int *)procaddr;
	pptr->pregs[SPC_L] = lobyte((unsigned) procaddr);
	pptr->pregs[SPC_H] = hibyte((unsigned) procaddr);
	pptr->pregs[SSREG] = INITPS;
	pptr->pnxtkin = BADPID;
	pptr->pdevs[0] = pptr->pdevs[1] = BADDEV;
	
	va_start(ap,nargs);
	for (i=0 ; i < nargs; i++)
		{
	    pptr->parg[i] = va_arg(ap, unsigned int);
		}
	va_end(ap);
	pptr->parg[nargs] = 0;
	
	/* machine/compiler dependent pass arguments to created process */
	pptr->pregs[24] = lobyte((unsigned)pptr->pargs);	/*r24*/
	pptr->pregs[25] = hibyte((unsigned)pptr->pargs);
	pptr->pregs[22] = lobyte((unsigned)&pptr->parg[0]);	/*r22*/
	pptr->pregs[23] = hibyte((unsigned)&pptr->parg[0]);

	*saddr-- = lobyte((unsigned)INITRET);	/* push on initial return address*/
	*saddr-- = hibyte((unsigned)INITRET);
//	*saddr-- = 0;	/* 256Kb memory device */
	*saddr-- = lobyte((unsigned)procaddr);	/* push on procedure address	*/
	*saddr-- = hibyte((unsigned)procaddr);
//	*saddr-- = 0;	/* 256Kb memory device */
	pptr->pregs[SSP_L] = lobyte((unsigned) saddr);
	pptr->pregs[SSP_H] = hibyte((unsigned) saddr);

	restore(ps);
	return(pid);
}

/*
 *------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
 
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
