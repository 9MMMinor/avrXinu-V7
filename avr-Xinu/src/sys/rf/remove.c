/* remove.c - remove */

#include <avr-Xinu.h>
#include <file.h>
#include <fserver.h>
#include <rfile.h>
#include <name.h>

/*------------------------------------------------------------------------
 *  remove  -  remove a file given its name (key is optional)
 *------------------------------------------------------------------------
 */

SYSCALL remove(char * name, int key)
{
	char	fullnam[NAMLEN];
	struct	devsw	*devptr;
	int	dev;

	if ( (dev=nammap(name, fullnam)) == SYSERR)
		return(SYSERR);
	devptr = &devtab[dev];
	return(	(*devptr->dvcntl)(devptr, FLREMOVE, fullnam, (void *)key) );
}
