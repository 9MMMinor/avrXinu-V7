/* namopen.c - namopen */

#include <avr-Xinu.h>
#include <name.h>

/*
 *------------------------------------------------------------------------
 *  namopen  -  open an object (e.g., remote file) based on the name
 *------------------------------------------------------------------------
 */
 
SYSCALL namopen(struct devsw *devptr, char *filenam, char *mode)
{
	int	dev;
	char newname[NAMLEN];

	if ( (dev=nammap(filenam, newname)) == SYSERR)
		return(SYSERR);
	return( (int)open(dev, newname, mode) );
}
