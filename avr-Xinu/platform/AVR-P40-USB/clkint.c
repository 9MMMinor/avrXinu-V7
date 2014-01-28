/*
 *  clkint.c
 *  Avr_Xinu
 *
 *  Created by Michael Minor on 8/23/10.
 *  Copyright 2010. All rights reserved.
 *
 */
#include <conf.h>
#include <kernel.h>
#include <sleep.h>
#include <avr/io.h>
#include <avr/interrupt.h>

extern void wakeup(void);
extern int resched(void);

static int prescaler = 10;

/*
 *--------------------------------------------------------------------------
 * Clock interrupt routine -- system clock driven TIMER3 in CTC mode
 * clkint() -- premptive scheduling and time-of-day clock routine
 *--------------------------------------------------------------------------
 */

ISR(TIMER3_COMPA_vect)		/* Timer3 Compare Match A vector */
{
    /* executing the ISR automatically clears the OCF3A Flag */
    /* 100 Hz clock 8000000/64/(1+1249) */
	/* OCR3A set to 1249 for CTC mode */

	ctr100++;				/* Version 8 timer/counter */
	
	if (--prescaler > 0)
		return;
	prescaler = 10;
	
	if (--countTick <= 0)	{
		countTick = TICK;
		clktime++;			/*update time of day*/
	}
    if (defclk)	{
		clkdiff++;
		return;
	}
    if ( slnempty && (--*sltop <= 0) )	{
		wakeup();
	}
    if (--preempt <= 0)	{
		resched();
	}
}
