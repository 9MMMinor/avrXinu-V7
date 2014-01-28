/*
 *  clkint.c
 *  avr-Xinu
 *
 *  Created by Michael Minor on 12/4/08.
 *  Copyright 2008. All rights reserved.
 *
 */
#include <conf.h>
#include <kernel.h>
#include <sleep.h>
#include <avr/io.h>
#include <avr/interrupt.h>

extern void wakeup(void);
extern int resched(void);

/*--------------------------------------------------------------------------
 * Clock interrupt routine -- watch crystal 32.768kHz driven TIMER2 overflow
 * clkint() -- premptive scheduling and time-of-day clock routine
 *--------------------------------------------------------------------------
 */

ISR(TIMER2_OVF_vect)		/* overflow interrupt vector */
{
    /* the timer overflow interrupt automatically clears the TOV2 Flag */
    /* 16 Hz clock (32768/8)/256 */

    if (--countTick <= 0)
		{
		countTick = TICK;
		clktime++;               /*update time of day*/
		}
    if (defclk)
		{
		clkdiff++;
		return;
		}
    if ( slnempty && (--*sltop <= 0) )
		wakeup();
    if (--preempt <= 0)
		resched();
}


