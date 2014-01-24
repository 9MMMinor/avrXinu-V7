//
//  proc.h - isbadpid(), isgoodpid()
//  avr-Xinu
//
//  Modified by Michael Minor on 9/25/13.
//
//

/* process table declarations and defined constants			*/

#ifndef Xinu_proc_h
#define Xinu_proc_h



#ifndef	NPROC				/* set the number of processes	*/
#define	NPROC		30		/*  allowed if not already done	*/
#endif

#ifndef	_NFILE
#define _NFILE		20		/* # of files allowed */
#endif

#define	FDFREE		-1		/* free file descriptor */

/* process state constants */

#define	PRCURR		'\001'		/* process is currently running	*/
#define	PRFREE		'\002'		/* process slot is free			*/
#define	PRREADY		'\003'		/* process is on ready queue	*/
#define	PRRECV		'\004'		/* process waiting for message	*/
#define	PRSLEEP		'\005'		/* process is sleeping			*/
#define	PRSUSP		'\006'		/* process is suspended			*/
#define	PRWAIT		'\007'		/* process is on semaphore queue*/
#define	PRTRECV		'\010'		/* process is timing a receive	*/

/* miscellaneous process definitions */

#define	PNREGS		38		/* size of saved register area	*/
#define SSP_L		32		/* saved SP (low)               */
#define SSP_H		33		/* saved SP (high)				*/
#define SPC_L		34		/* saved PC (low)               */
#define SPC_H		35		/* saved PC (high)              */
#define SPC_E		36		/* saved PC (extended)          */
#define SSREG		37		/* saved Status Register        */
#define	PNMLEN		16		/* length of process "name"		*/
#define MAXARG		6		/* max args create()ed			*/

#define	NULLPROC	0		/* id of the null process; it	*/
/*  is always eligible to run	*/
#define	BADPID		-1		/* used when invalid pid needed	*/

#define	isbadpid(x)	( (x)<=0 || NPROC<=(x) )
#define isgoodpid(x) ( 0<(x) && (x)<NPROC )

/* process table entry */

struct	pentry	{
	int pstate;					/* process state: PRCURR, etc.	*/
	int pprio;					/* process priority				*/
	unsigned char pregs[PNREGS];/* saved context (see ctxsw)	*/
	int psem;					/* semaphore if process waiting	*/
	int pmsg;					/* message sent to this process	*/
	int phasmsg;				/* nonzero iff pmsg is valid	*/
	unsigned char *pbase;		/* base of run time stack		*/
	int pstklen;				/* stack length					*/
	unsigned char *plimit;		/* lowest extent of stack		*/
	char pname[PNMLEN];			/* process name					*/
	int pargs;					/* initial number of arguments	*/
	int parg[MAXARG];			/* arguments					*/
	int *paddr;					/* initial code address			*/
	int pnxtkin;				/* next-of-kin notified of death*/
	int pdevs[2];				/* devices to close upon exit	*/
	void *fildes[_NFILE];		/* pointers to FILE, fildes[0]==stdin	*/
};


extern struct pentry proctab[];
extern int numproc;		/* currently active processes	*/
extern int nextproc;		/* search point for free slot	*/
extern int currpid;		/* currently executing process	*/

#endif
