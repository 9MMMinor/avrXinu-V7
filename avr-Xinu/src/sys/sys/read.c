/* read.c - read */

#include <conf.h>
#include <kernel.h>
#include <io.h>

/*
 *------------------------------------------------------------------------
 *  read  -  read one or more bytes from a device
 *------------------------------------------------------------------------
 */

SYSCALL read(int descrp, unsigned char *buff, int count)
{
	struct	devsw	*devptr;

	if (isbaddev(descrp) )
		return(SYSERR);
	devptr = &devtab[descrp];
	return(	(*devptr->dvread)(devptr,buff,count) );
}
