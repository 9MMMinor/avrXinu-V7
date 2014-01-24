/* access.c - access */

#include <avr-Xinu.h>
#include <file.h>
#include <fserver.h>
#include <rfile.h>
#include <name.h>

/*------------------------------------------------------------------------
 *  access  -  determine accessability given file name and desired mode
 *------------------------------------------------------------------------
 */

SYSCALL access(char *name, int mode)
{
	char	fullnam[NAMLEN];
	struct	devsw	*devptr;
	int	dev;

	if ( (dev=nammap(name, fullnam)) == SYSERR)
		return(SYSERR);
	devptr = &devtab[dev];
	return(	(*devptr->dvcntl)(devptr, FLACCESS, fullnam, (void *)mode) );
}
