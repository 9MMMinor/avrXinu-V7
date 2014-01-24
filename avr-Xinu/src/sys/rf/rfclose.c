/* rfclose.c - rfclose */

#include <avr-Xinu.h>
#include <network.h>

/*
 *------------------------------------------------------------------------
 *  rfclose  --  close a remote file by deallocating pseudo device
 *------------------------------------------------------------------------
 */
 
int rfclose(struct devsw *devptr)
{
	struct rfblk *rfptr;
	long junk = 0L;		/* argument to rfmkpac; not really used	*/

	rfptr = (struct rfblk *)devptr->dvioblk;
	if (rfptr->rf_state == RFREE)
		return(SYSERR);
	wait(rfptr->rf_mutex);
	rfmkpac(FS_CLOSE,rfptr->rf_name,&junk,(char *)&junk,0);
	rfptr->rf_state = RFREE;
	signal(rfptr->rf_mutex);
	return(OK);
}
