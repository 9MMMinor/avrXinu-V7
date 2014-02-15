/*
 *  ttyopen.c
 *  avr-Xinu
 *
 *  Created by Michael Minor on 3/12/10.
 *  Copyright 2010. All rights reserved.
 *
 */

#include <avr-Xinu.h>
#include <stdlib.h>		/* for calloc() */
#include <USART.h>
#include <tty.h>

extern SYSCALL wait(int);

/*
 *----------------------------------------------------------------
 * ttyopen - open tty device and return descriptor (for namespace)
 *----------------------------------------------------------------
 */

int ttyopen(struct devsw *devptr, char *name, char *mode)
{
	/* this routine is not normally used  to open tty devices,	*/
	/* but is provided so that automatic calls to open do not	*/
	/* fail.  It returns SYSERR unless called with a null name	*/
	
	if (*name == '\0')
		return (devptr->dvnum);
	else
		return (SYSERR);
}
