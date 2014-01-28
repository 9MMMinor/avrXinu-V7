/* clkinit.c - clkinit */

#include <conf.h>
#include <kernel.h>
#include <q.h>
#include <sleep.h>   
#include <avr/io.h>
#include <avr/interrupt.h>

extern int screate();
extern int newqueue();
    
/* real-time clock variables and sleeping process queue pointers	*/
    
#ifdef	RTCLOCK
unsigned char countTick;	/* counts in TICKs of a second 			*/
unsigned int clmutex;		/* mutual exclusion for time-of-day		*/
volatile long clktime;		/* current time in seconds since 1/1/70	*/
Bool defclk;				/* non-zero, then deferring clock count */
unsigned int clkdiff;		/* deferred clock ticks					*/
int slnempty;				/* FALSE if the sleep queue is empty	*/
int *sltop;	      		 	/* address of key part of top entry in	*/
							/* the sleep queue if slnonempty==TRUE	*/
int clockq;					/* head of queue of sleeping processes  */
int preempt;				/* preemption counter.	Current process */
							/* is preempted when it reaches zero;	*/
#ifdef	RTCLOCK
							/* set in resched; counts in ticks		*/
int	clkruns;				/* set TRUE iff clock exists by setclkr	*/
#else
int	clkruns = FALSE;		/* no clock configured; be sure sleep	*/
#endif						/*   doesn't wait forever				*/

/*
 *------------------------------------------------------------------------
 * clkinit - initialize the clock and sleep queue (called at startup)
 *------------------------------------------------------------------------
 */
void clkinit()
{

	preempt = QUANTUM;		/* initial time quantum		*/
	countTick = TICK;	 	/* TICKs of a sec. counter	*/
	clmutex = screate(1);	/* semaphore for tod clock	*/
	clktime = 0L;			/* initially a low number	*/
	slnempty = FALSE;		/* initially, no process asleep	*/
	clkdiff = 0;			/* zero deferred ticks		*/
	defclk = 0;				/* clock is not deferred	*/
	clkruns = 1;
	clockq = newqueue();

	/* Initialize timer */
	TIMSK2 &= ~((1<<TOIE2)|(1<<OCIE2A)|(1<<OCIE2B));     //Disable TC2 interrupt
    ASSR |= (1<<AS2);           //set Timer/Counter2 to be asynchronous from the CPU clock 
								//with a second external clock(32.768kHz)driving it.  
    TCNT2 = 0x00;
	TCCR2A = 0x00;				//normal mode 0
    TCCR2B = 0x02;              //prescale the timer to be clock source / 8 to make it
								//exactly 1/16 second for every overflow to occur
	while(ASSR&0x1F)
//	while(ASSR&( (1<<TCR2BUB)|(1<<OCR2BUB)|(1<<TCN2UB) ))
		;	//Wait until TC0 is updated
	TIFR2 |= (1<<TOV2);			//Clear overflow flag bit
    TIMSK2 |= (1<<TOIE2);       //set 8-bit Timer/Counter0 Overflow Interrupt Enable                           
		
}
#endif
