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
unsigned char countTick;	/* counts in TICKs of a second			*/
unsigned int clmutex;		/* mutual exclusion for time-of-day		*/
volatile long clktime;		/* current time in seconds since 1/1/70	*/
volatile unsigned long ctr100;		/* counts in 100ths of second 0-INF	*/
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

/* Output Compare Register setting to give 100 interrupts per second	*/
/* OCR_F_CLK = (F_CLK/6400) - 1											*/

#define OCR_3686400   575
#define OCR_7372800  1151
#define OCR_8000000  1249
#define OCR_11059200 1727
#define OCR_14745600 2303
#define OCR_16000000 2499

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
	ctr100 = 0L;			/* timer (relative)			*/
	slnempty = FALSE;		/* initially, no process asleep	*/
	clkdiff = 0;			/* zero deferred ticks		*/
	defclk = FALSE;				/* clock is not deferred	*/
	clkruns = 1;
	clockq = newqueue();

	OCR3AH = hibyte(OCR_11059200);	/* 0-OCR_F_CLK => divide by OCR_F_CLK+1 */
	OCR3AL = lobyte(OCR_11059200);	/* write high byte first!	*/
	  
    TCNT3L = 0x00;
	TCNT3H = 0x00;
	TCCR3A = 0x00;			/* normal port operation, WGM31=0 WGM30=0 */
	TCCR3B = (0<<CS32) | (1<<CS31) | (1<<CS30);	/* (clock source/64) */
	TCCR3B |= (1 << WGM32);	/* normal mode 4: CTC w/OCR3A */
	TCCR3C = 0x00;

	TIMSK3 |= (1<<OCF3A);	/* Clear overflow flag bit */
    TIMSK3 |= (1<<OCIE3A);	/* Output Compare A Match Interrupt Enable */
	
	/* Initialize timer 2 (RTC) */
	TIMSK2 &= ~((1<<TOIE2)|(1<<OCIE2A)|(1<<OCIE2B));     //Disable TC2 interrupt
    ASSR |= (1<<AS2);           //set Timer/Counter2 to be asynchronous from the CPU clock
	//with a second external clock(32.768kHz)driving it.
    TCNT2 = 0x00;
	TCCR2A = 0x00;				//normal mode 0
    TCCR2B = 0x05;              //prescale the timer to be clock source / 128
//
//
//	while(ASSR&0x1F)
		//	while(ASSR&( (1<<TCR2BUB)|(1<<OCR2BUB)|(1<<TCN2UB) ))
//		;	//Wait until TC0 is updated
//	TIFR2 |= (1<<TOV2);			//Clear overflow flag bit
// 	TIMSK2 |= (1<<TOIE2);       //set 8-bit Timer/Counter0 Overflow Interrupt Enable
		
}
#endif
