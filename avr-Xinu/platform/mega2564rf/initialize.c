/* initialize.c - nulluser, sysinit */

#include <stdio.h>

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <sleep.h>    
#include <mem.h>
#include <USART.h>
#include <tty.h>
#include <q.h>
#include <io.h>
#include <mark.h>
#include <ports.h>
#include <network.h>
#include <spi.h>
#include <external_interrupt.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include <stdlib.h>                     /* FOR malloc() */
#include <avr/pgmspace.h>

#define FOSC F_CPU					// default fuse setting CKDIV8=0
#define BAUD 9600L
#define STK500_UBRR (FOSC)/(BAUD*8L)-1L		// U2X0 set for doubling


SYSCALL create(int (*procaddr)(), int ssize, int priority, char *name, int nargs, ...);
SYSCALL poolinit(void);
SYSCALL pinit(int maxmsgs);
void naminit(void);
SYSCALL namopen(struct devsw *devptr, char *filenam, char *mode);
void _mkinit(void);
extern int main();		/* address of user's main prog	*/
extern void USART_Init();	/* initialize for kprintf polled output */

extern char __bss_end;
extern char __data_end;
extern char __heap_start;
extern char __heap_end;
extern void _etext();

extern SYSCALL resume();
extern int newqueue();
extern int init();
extern void clkinit();
extern void dump_Stack();
extern char *getSP(void);

/* memory locations */

char *__malloc_heap_start = &__heap_start;
char *__malloc_heap_end = &__heap_end;

#include <confisr.c>

/* Declarations of major kernel variables */

struct pentry proctab[NPROC];	/* process table						*/
int nextproc;					/* next process slot to use in create	*/
struct sentry semaph[NSEM];		/* semaphore table						*/
int nextsem;					/* next sempahore slot to use in screate*/
struct qent q[NQENT];			/* q table (see queue.c)				*/
int nextqueue;					/* next slot in q structure to use		*/
//#ifdef	NDEVS
//struct intmap intmap[NDEVS];		/* interrupt dispatch table		*/
//#endif
struct mblock memlist;			/* list of free memory blocks		*/
#ifdef	Ntty
struct tty tty[Ntty];			/* USART buffers and mode control	*/
#endif

/* active system status */

int numproc;	/* number of live user processes	*/
int currpid;	/* id of currently running process	*/
unsigned int reboot;		/* MCUSR after boot	*/

int rdyhead, rdytail;		/* head/tail of ready list (q indicies)	*/
char vers[]  = VERSION;		/* Xinu version printed at startup	*/

LOCAL sysinit();

#include <conf.c>

void nulluser(void) __attribute__ ((naked)) __attribute__ ((section (".init8")));


/**************************************************************************/
/***				NOTE:												***/
/***																	***/
/***   This is where the system begins after the C environment has		***/
/***   been established.  Interrupts are initially DISABLED, and		***/
/***   must eventually be enabled explicitly.  This routine turns		***/
/***   itself into the null process after initialization.  Because		***/
/***   the null process must always remain ready to run, it cannot		***/
/***   execute code that might cause it to be suspended, wait for a		***/
/***   semaphore, or put to sleep, or exit.  In particular, it must		***/
/***   not do I/O unless it uses kprintf for polled output.				***/
/***																	***/
/**************************************************************************/

/* babysit CPU when no one home */

void nulluser(void)
{
    int userpid;
	int ubrr = STK500_UBRR;
	unsigned int flash = (unsigned int)&_etext;
    
	reboot = (unsigned int)MCUSR;

    PORTB = 0x00;
    DDRB = 0x01;			/* make the LED pin an output */    
    USART_Init(0, ubrr);	/* initialize USART transmitter for kprintf */

    kprintf_P(PSTR("\n\nXinu Version %s"), vers);	
	kprintf_P(PSTR("   MCUCSR=%x\n"), reboot);

    MCUSR = 0;
    sysinit();
    kprintf_P(PSTR("%u bytes Xinu code, SP=%p\n"), flash<<1, getSP());
    kprintf_P(PSTR("clock %sabled\n\n"),clkruns==1?"en":"dis");

    enable();		/* enable interrupts */
    
	/* create a process to execute the user's avr-main program, "Xinu_main" */
    userpid = create(main,INITSTK,INITPRIO,INITNAME,0);
//	kprintf("main: created %d\n", userpid);

#ifdef	NETDAEMON
	/* start the network input daemon process */
	resume(
	  create(NETIN, NETISTK, NETIPRI, NETINAM, 1, userpid)
	);
#else
	PORTB = 0;
    resume( userpid );
#endif

    while ( 1 )	{
//		SMCR |= (1<<SE);	/* enable sleep idle mode */
//		pause();
//		SMCR &= ~(1<<SE);
//		PORTB ^= 1;
	}
}

/*------------------------------------------------------------------------
 *  sysinit  --  initialize all Xinu data structeres and devices
 *------------------------------------------------------------------------
 */
LOCAL	sysinit()
{

	int i,j,len;
	struct pentry *pptr;	 /* null process entry */
	struct sentry *sptr;
	struct mblock *volatile mptr;

	numproc = 0;			/* initialize system variables */
	nextproc = NPROC-1;
	nextsem = NSEM-1;
	nextqueue = NPROC;		/* q[0..NPROC-1] are processes */

	memlist.mnext = mptr =		/* initialize free memory list */
	    (struct mblock *volatile) roundmb(__malloc_heap_start);
	mptr->mnext = (struct mblock *)NULL;
	mptr->mlen = len = (int) truncmb(RAMEND - NULLSTK - (unsigned)&__bss_end);
	__malloc_heap_start = (char *)mptr;
	__malloc_heap_end = __malloc_heap_start + len;
	kprintf_P(PSTR("Heap: %p of length %d\n"), mptr, len);
	
	for (i=0 ; i<NPROC ; i++)	/* initialize process table */
		proctab[i].pstate = PRFREE;

	/* initialize null process entry */
	pptr = &proctab[NULLPROC];
	pptr->pstate = PRCURR;
	for (j=0; j<6; j++)
		pptr->pname[j] = "nullp"[j];

	pptr->plimit = (unsigned char *)(RAMEND + 1) - NULLSTK;
	pptr->pbase = (unsigned char *) RAMEND;
	*pptr->pbase = (unsigned char)MAGIC; 	/* clobbers return, but proc 0 doesn't return */
	pptr->paddr = (int *) main;
	pptr->pargs = 0;
	pptr->pprio = 0;
	pptr->pregs[SSP_L] = lobyte((unsigned int)pptr->plimit);	/* for error checking */
	pptr->pregs[SSP_H] = hibyte((unsigned int)pptr->plimit);	/* for error checking */
	currpid = NULLPROC;

	for (i=0 ; i<NSEM ; i++) {	/* initialize semaphores */
		(sptr = &semaph[i])->sstate = SFREE;
		sptr->sqtail = 1 + (sptr->sqhead = newqueue());
	}

	rdytail = 1 + (rdyhead=newqueue());	/* initialize ready list */

	
#ifdef	MEMMARK
	kprintf("Memory marking\n");
	_mkinit();			/* initialize memory marking */
#else
	kprintf("Pool init\n");
	poolinit();			/* explicitly */
	pinit(MAXMSGS);
#endif

#ifdef	RTCLOCK
	kprintf("init RTC\n");
	clkinit();			/* initialize r.t.clock	*/
#endif

#ifdef NDEVS
	for ( i=0 ; i<NDEVS ; i++ ) {
		if (i>0) kprintf("init dev %d\n", i);
	    init(i);
	}
#endif

#ifdef	NNETS
//	kprintf("net init\n");
	netinit();
#endif

	return (OK);
}
