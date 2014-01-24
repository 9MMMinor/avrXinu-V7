/*
 *  interrupt_enable.c
 *  AVR-Xinu
 *
 *  Created by Michael Minor on Sat Apr 04 2009.
 *  Copyright (c) 2009 __MyCompanyName__. All rights reserved.
 *
 */
#include <avr/io.h>

void disable(char *x)
{
    *x = SREG;	/*save status register*/
    asm("cli");	/*and disable interrupts*/
}

void restore(char *x)
{
    SREG = *x;	/*restore the status register, possibly reenabling interrupts*/
}


