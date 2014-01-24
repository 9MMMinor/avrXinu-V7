/* putc.c - putc */
//
//  Xinu_putc.c
//  compatible with avr-libc
//
//  Created by Michael M Minor on 4/17/11.
//  Copyright 2011. All rights reserved.
//

#include <stdio.h>

#include <conf.h>
#include <kernel.h>
#include <io.h>

/*
 *------------------------------------------------------------------------
 *  putc  -  write a single character to a device
 *------------------------------------------------------------------------
 */

SYSCALL _putc(int descrp, char ch)
{
	struct	devsw	*devptr;
	
	if (isbaddev	(descrp) )
		return(SYSERR);
	devptr = &devtab[descrp];
	return(	(*devptr->dvputc)(devptr,ch) );
}

/*
 *------------------------------------------------------------------------
 *  Xinu_putc  -  write a single character to a stream
 *      Get the file descriptor from the stream and call
 *      Xinu device driver.
 *------------------------------------------------------------------------
 */

int Xinu_putc(char ch, FILE * stream)
{
	struct	devsw	*devptr;
    int descrp;
	
	if (isbaddev	(descrp = file_get_fdesc(stream)) )
		return(SYSERR);
	devptr = &devtab[descrp];
	return(	(*devptr->dvputc)(devptr,ch) );
}
