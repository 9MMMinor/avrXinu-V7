/* rfgetc.c - rfgetc */

#include <avr-Xinu.h>
#include <network.h>

/*------------------------------------------------------------------------
 *  rfgetc  --  get a character from a remote file
 *------------------------------------------------------------------------
 */

int
rfgetc(struct devsw *devptr)
{
	char ch;
	int	retcode;

	if ( (retcode=read(devptr->dvnum, (unsigned char *)&ch, 1)) == 1)
		return(ch);
	else if (retcode != EOF)
		retcode = SYSERR;
	return(retcode);
}
