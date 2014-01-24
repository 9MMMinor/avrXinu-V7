/* rfread.c - rfread */

#include <avr-Xinu.h>
#include <network.h>

/*------------------------------------------------------------------------
 *  rfread  --  read one or more bytes from a remote file
 *------------------------------------------------------------------------
 */

int rfread(struct devsw *devptr, char *buff, int len)
{
	return( rfio(devptr, FS_READ, buff, min(len,RDATLEN) ) );
}
