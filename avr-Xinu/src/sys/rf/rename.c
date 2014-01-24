/* rename.c - rename */

#include <avr-Xinu.h>
#include <file.h>
#include <fserver.h>
#include <rfile.h>
#include <name.h>

/*------------------------------------------------------------------------
 *  rename  -  rename a file (key is optional)
 *------------------------------------------------------------------------
 */

SYSCALL rename(char * old, char * new)
{
	char	fullold[NAMLEN];
	char	fullnew[NAMLEN];
	struct	devsw	*devptr;
	int	dev, dev2;

	/* map names through namespace and restrict to single device */
	
//	printf("rename: dev=%d :%s:%s:\n", nammap(old,fullold), old, fullold);
//	printf("rename: dev2=%d :%s:%s:\n", nammap(new,fullnew), new, fullnew);
	if ( (dev = nammap(old, fullold)) == SYSERR ||
	     (dev2= nammap(new, fullnew)) == SYSERR ||
	      dev != dev2)
			return(SYSERR);
	devptr = &devtab[dev];
	return(	(*devptr->dvcntl)(devptr,FLRENAME,fullold,fullnew) );
}
