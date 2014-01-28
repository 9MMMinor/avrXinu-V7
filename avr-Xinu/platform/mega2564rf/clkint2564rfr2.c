/*
 *  clkint.c
 *  Avr_Xinu
 *
 *  Created by Michael Minor on 12/28/13.
 *  Copyright 2013. All rights reserved.
 *
 */

//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <conf.h>
#include <kernel.h>
#include <sleep.h>
#include <avr/io.h>
#include <avr/interrupt.h>

extern void wakeup(void);
extern int resched(void);

static int prescaler = 10;
static int rtcScaler = 10;

/*
 *--------------------------------------------------------------------------
 * Clock interrupt routine -- system clock driven TIMER3 in CTC mode
 * clkint() -- premptive scheduling and time-of-day clock routine
 *--------------------------------------------------------------------------
 */

/*
 *--------------------------------------------------------------------------
 * Compare Unit 1 Interrupt service routine
 *
 *	NOTE: the status bit (in SCIRQS) is cleared automatically
 *--------------------------------------------------------------------------
 */

ISR(SCNT_CMP1_vect)						/* Symbol Counter Compare vector */
{

	/* 0.1-second clock */
	uint32_t set = macCompareUnit1Read() + 6250;
	macCompareUnit1Write(set);
	if ( --rtcScaler == 0 )	{
		rtcScaler = 10;
		clktime++;
		PORTB ^= 1;
	}
	if (defclk)	{
		clkdiff++;
		return;
	}
	if ( slnempty && (--*sltop <= 0) )	{
		wakeup();
	{
    if (--preempt <= 0)	{
		resched();
	}
}
