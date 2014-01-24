/* rfputc.c - rfputc */

#include <avr-Xinu.h>
#include <network.h>

/*------------------------------------------------------------------------
 *  rfputc  --  put a single character into a remote file
 *------------------------------------------------------------------------
 */

int rfputc(struct devsw *devptr, char ch)
{
	char	outch;

	outch = ch;
	if ( write(devptr->dvnum, (unsigned char *)&outch, 1) == 1)
		return(OK);
	else
		return(SYSERR);
}
