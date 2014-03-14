//
//!  \file macSymbolCounter.c
//!  \defgroup radio-15-4
//
//
//
//  Created by Michael M Minor on 10/17/13.
//
//

/** \mainpage macSymbolCounter
 *  \section intro_section INTRODUCTION
 * The following Timer is modeled after The TCP Timer Process [Comer 1991].
 * Timed events are entered into a linked list in
 * absolute time order. When a time expires, the interrupt service resumes
 * the radioTimer process, which runs at high priority. The timer process sends
 * the message that the event contains (tq_msg) to the port that the message
 * specifies (tq_port) and removes the event from the head of the list. The queue
 * can contain at most MAXTIMERMESSAGES unique events (port & msg). Before an
 * event is placed in the time queue, any old events (with the same port and
 * message) are removed. It is still possible to have more than one event trigger at the
 * same compare time. The timer ensures that only a single interrupt occurs. Time critical events
 * may specify a INTPROC to be called from interrupt service. In addition, user processes
 * could use the message, which is type (void *), to pass callback routines through
 * the port.
 */

#include <avr-Xinu.h>
#include <proc.h>
#include <sleep.h>
#include <sem.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <mem.h>
#include <mark.h>
#include <ports.h>

#include "macSymbolCounter.h"


struct timestamp_timeval radioTime;
PROCESS radioTimer(void);

void macSymbolCounterInit(void)
{
	STATWORD ps;
//
//	12/21/13 -- timer2 stopped?? after I soldered 47 ohm termination on RFP, RFN pins. (Connection??)
//		select system clock (SCCKSEL=0)
//
	SCCR0 |= ( (0<<SCCKSEL)	|	/* select 32kHz clock source if 1 */
				(1<<SCEN)	|	/* Symbol Counter Enable */
				(1<<SCTSE) );	/* Symbol Counter Automatic Timestamping Enable */
	SCIRQM |= (1<<IRQMOF);		/* enables the SCNT_OVFL interrupt */

	disable(ps);
	SCCNTHH = 0;
	SCCNTHL = 0;
	SCCNTLH = 0;
	SCCNTLL = 0;
	radioTime.sysSeconds = clktime;
	radioTime.timestamp = 0;
	restore(ps);
}

/*
 *********************************************************
 * Symbol Counter Overflow Interrupt
 *	62500 (sec)-1
 *	2^32 / 62500 = 68719 seconds
 *	or			      19 hours 5 minutes and 19 seconds
 *********************************************************
 */

ISR(SCNT_OVFL_vect)
{
	radioTime.sysSeconds = clktime;
}

/**
 * All 32 bit registers support atomic read or write operation. That means reading or writing
 * the least significant xxxLL byte (the register name ends in LL) updates or captures the
 * complete 32 bit value.
 */

uint32_t macSymbolCounterRead(void)
{
	uint32_t r32 = (uint32_t)SCCNTLL;
	
	return ( ((uint32_t)SCCNTHH<<24) |
			 ((uint32_t)SCCNTHL<<16) |
			 ((uint32_t)SCCNTLH<<8)	 |
			 r32		);
}

void macSymbolCounterWrite(uint32_t value)
{
	
	SCCNTHH = (value>>24)&0xFF;
	SCCNTHL = (value>>16)&0xFF;
	SCCNTLH = (value>>8)&0xFF;
	SCCNTLL = (value)&0xFF;
}

void macCompareUnit1Write(uint32_t value)
{
	
	SCOCR1HH = (value>>24)&0xFF;
	SCOCR1HL = (value>>16)&0xFF;
	SCOCR1LH = (value>>8)&0xFF;
	SCOCR1LL = (value)&0xFF;
}

uint32_t macCompareUnit1Read(void)
{
	uint32_t r32 = (uint32_t)SCOCR1LL;
	
	return ( ((uint32_t)SCOCR1HH<<24) |
			((uint32_t)SCOCR1HL<<16) |
			((uint32_t)SCOCR1LH<<8)	 |
			r32		);
}


int	tqMutex;			/* radioTimer mutual exclusion semaphore */
int tqEvent;			/* radioTimer event signal semaphore	 */
int tqPid;				/* Pid of eventTimer	*/
struct tqent *tqHead;	/* timer queue list head */
int timerPortID;		/* a message port for fast timer events */

/**
 *------------------------------------------------------------------------
 *  radioTimerEventInit -  radio event timer initialize
 *------------------------------------------------------------------------
 */

void
radioTimerEventInit(void)
{
	
	tqHead = NULLQ;
	tqMutex = screate(1);		/* mutual exclusion semaphore	*/
	tqEvent = screate(0);		/* event signal */
	SCCR0 &= ~(1<<SCCMP1);		/* absolute compare (SCOCR1 == SCCNT) */
	/* create a port with maximum counts of messages */
	timerPortID = pcreate(MAXTIMERMESSAGES);
	tqPid = create(radioTimer, 600, 105, "radioTimer", 0);
	resume(tqPid);
}

/**
 *------------------------------------------------------------------------
 *  radioTimer -- a process to manage fast timer events.
 *------------------------------------------------------------------------
 */

PROCESS
radioTimer(void)
{
	uint32_t now;			// times from system clock
	struct tqent *tq;		// temporary time list queue ptr
	
	while (TRUE) {
		wait(tqEvent);		//   wait for ISR to signal
		
		wait(tqMutex);
		now = macSymbolCounterRead();

		while ( tqHead  && tqHead->tq_compare <= now)	{
			psend(tqHead->tq_port, (int)tqHead->tq_msg);
			tq = tqHead;
			tqHead = tqHead->tq_next;
			freemem(tq, sizeof(struct tqent));
		}
		if (tqHead)	{
			macCompareUnit1Write(tqHead->tq_compare);
			SCIRQM |= (1<<IRQMCP1);		/* set interrupt enable */
		}
		else	{
			SCIRQM &= ~(1<<IRQMCP1);	/* disable interrupt */
		}
		signal(tqMutex);
	}
}

/**
 ****************************************************************************
 * Compare Unit 1 Interrupt service routine
 *
 *	NOTE: the status bit (in SCIRQS) is cleared automatically
 ****************************************************************************
 */

ISR(SCNT_CMP1_vect)
{
	struct tqent *tq;
	uint32_t now;
	
	now = macSymbolCounterRead();
	for( tq = tqHead; tq && tq->tq_compare <= now; tq = tq->tq_next)	{
		if ( tq->tq_callThru )	{
			(*tq->tq_callThru)(tq->tq_msg);		/* callback routine */
		}
	}
	signal(tqEvent);
}

/**
 *------------------------------------------------------------------------
 *  tmclear -  clear the indicated timer.
 *------------------------------------------------------------------------
 *	A timer is matched if the port and message match.
 *	Returns the time spent before removal or SYSERR if none is removed.
 */

uint32_t
tmclear(int port, void *msg)
{
	struct tqent *prev, *tq;
	uint32_t timespent;
	
	wait(tqMutex);
	prev = NULLQ;
	for (tq = tqHead; tq; tq = tq->tq_next) {
		if (tq->tq_port == port && tq->tq_msg == msg) {
			timespent = macSymbolCounterRead() - tq->tq_time;
			if (prev)	{
				prev->tq_next = tq->tq_next;
			} else if ( (tqHead = tq->tq_next) )	{
				macCompareUnit1Write(tqHead->tq_compare);
			} else	{
				SCIRQM &= ~(1<<IRQMCP1);	/* disable interrupt */
			}
			freemem(tq, sizeof(struct tqent));
			signal(tqMutex);
			return timespent;
		}
		prev = tq;
	}
	signal(tqMutex);
	return SYSERR;
}

/**
 *------------------------------------------------------------------------
 *  tmleft -  return the time left for this timer.
 *------------------------------------------------------------------------
 */

uint32_t
tmleft(int port, void *msg)
{
	struct tqent *tq;
	uint32_t timeleft;
	
	if (tqHead == NULLQ)
		return 0;
	wait(tqMutex);
	for (tq = tqHead; tq; tq = tq->tq_next) {
		if (tq->tq_port == port && tq->tq_msg == msg) {
			timeleft = tq->tq_compare-macSymbolCounterRead();
			signal(tqMutex);
			return timeleft;
		}
	}
	signal(tqMutex);
	return 0;
}

/**
 *------------------------------------------------------------------------
 *  tmset -  set a fast timer.
 *------------------------------------------------------------------------
 *	Enter a time event into a linked list in time order. The time argument
 *	is the relative time (in symbol time units) from the execution of tmset
 *	until a compare interrupt is set to occur. When the interrupt
 *	occurs, \em msg is passed to the Xinu port, \em port, and the INTPROC,
 *	\em callThru, is called with \em msg as an argument.
 */

int
tmset(uint8_t port, void *msg, uint32_t time, INTPROC (*callThru)(void *))
{
	struct tqent *tqPrevious, *tqNew, *tq;
	uint32_t now;

	if (time == 0)	{				/* do psend NOW */
		if ( callThru )	{
			(*callThru)(msg);		/* callback routine */
		}
		psend(port, (int)msg);
		return OK;
	}
	/* get a new time structure and fill it */
	tqNew = (struct tqent *)getmem(sizeof(struct tqent));	/* TODO: could use bpool and preallocate */
	tqNew->tq_time = now = macSymbolCounterRead();
	tqNew->tq_compare = now + time;
	tqNew->tq_port = port;
	tqNew->tq_msg = msg;
	tqNew->tq_callThru = callThru;
	tqNew->tq_next = NULLQ;
	
	/* clear duplicates */
	(void) tmclear(port, msg);
	
	wait(tqMutex);
	if (tqHead == NULLQ) {
		tqHead = tqNew;
		macCompareUnit1Write(tqNew->tq_compare);
		SCIRQS |= (1<<IRQSCP1);		/* clear interrupt status bit */
		SCIRQM |= (1<<IRQMCP1);		/* set interrupt enable */
		signal(tqMutex);
		return OK;
	}
	/* search the list for our spot */
	/* if the new item becomes first on the list - rewrite Compare register */
	
	for (tqPrevious=NULLQ, tq=tqHead; tq; tq=tq->tq_next) {
		if (tqNew->tq_compare < tq->tq_compare)	{
			break;					/* goes in front of tq */
		}
		tqPrevious = tq;
	}
	tqNew->tq_next = tq;
	if (tqPrevious)					/* link in tqNew */
		tqPrevious->tq_next = tqNew;
	else	{
		tqHead = tqNew;
		macCompareUnit1Write(tqNew->tq_compare);
	}
	signal(tqMutex);
	return OK;
}

/**
 *------------------------------------------------------------------------
 *  radioKick -  make sure we send a packet soon
 *------------------------------------------------------------------------
 */

int
radioKick(void)
{
	return OK;
}

/**
 *------------------------------------------------------------------------
 *  pauseMicroSeconds -  set a fast timer and wait for it
 *------------------------------------------------------------------------
 */

void pauseMicroSeconds(void *message, uint32_t usec)
{
	uint32_t time = usec/16;	/* convert to symbol times */
	
	tmset(timerPortID, message, time, (void *)0);
	preceive(timerPortID);		/* blocks until time event */
}

/**
 *------------------------------------------------------------------------
 *  pauseSymbolTimes -  set a fast timer and wait for it
 *------------------------------------------------------------------------
 */

void pauseSymbolTimes(void *message, uint32_t stime)
{
	
	tmset(timerPortID, message, stime, (void *)0);
	preceive(timerPortID);		/* blocks until time event */
}


/**
 *------------------------------------------------------------------------
 *  tqdump -  dump a timer queue (for debugging)
 *------------------------------------------------------------------------
 */

void
tqdump(void)
{
	struct tqent *tq;
	
	if ( (tq = tqHead) == NULLQ )	{
		kprintf("tq <>\n");
		return;
	}
	for ( ; tq; tq=tq->tq_next) {
		kprintf("tq <D %ld,T %ld, (0x%0x)>\n", tq->tq_compare,
				tq->tq_time, tq->tq_msg);
	}
}

/**
 *------------------------------------------------------------------------
 *  tqwrite - print the timer queue
 *------------------------------------------------------------------------
 */

void
tqwrite(void)
{
	struct tqent *tq;
	char str[80];
	
	if (tqHead == 0) {
		printf("no entries\n");
		return;
	}
	printf("compare\ttime\tport\tmsg\n");
	for (tq=tqHead; tq; tq=tq->tq_next) {
		sprintf(str, "%-8ld\t%-5ld\t%-4d\t0x%-3x\n",
				tq->tq_compare, tq->tq_time, tq->tq_port, tq->tq_msg);
		printf("%s", str);
	}
}
