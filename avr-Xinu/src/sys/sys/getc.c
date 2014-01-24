/* getc.c - getc */
//
//  Xinu_getc.c
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
 *  getc  -  get one character from a device
 *------------------------------------------------------------------------
 */
 
SYSCALL	_getc(int descrp)
{
	struct	devsw	*devptr;

	if (isbaddev(descrp) )
		return(SYSERR);
	devptr = &devtab[descrp];
        return( (*devptr->dvgetc)(devptr) );
}

/*
 *------------------------------------------------------------------------
 *  Xinu_getc  -  read a single character from a stream
 *      Get the file descriptor from the stream and call
 *      Xinu device driver.
 *------------------------------------------------------------------------
 */

int Xinu_getc(FILE * stream)
{
	struct	devsw	*devptr;
    int descrp;
	
	if (isbaddev	(descrp = file_get_fdesc(stream)) )
		return(SYSERR);
	devptr = &devtab[descrp];
	return(	(*devptr->dvgetc)(devptr) );
}
