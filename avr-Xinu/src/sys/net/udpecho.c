/* udpecho.c - udpecho */

#include <avr-Xinu.h>
#include <network.h>

#ifndef INTERNET
#define INTERNET -1
#endif

#define	MAXECHO	600		/* maximum size of echoed datagram	*/
static	char	buff[MAXECHO];	/* here because the stack may be small	*/

/*------------------------------------------------------------------------
 *  udpecho  -  UDP echo server process (runs forever in background)
 *------------------------------------------------------------------------
 */

PROCESS
udpecho(void)
{
	int	dev, len;

	if ( (dev=(int)open(INTERNET, ANYFPORT, (void *)UECHO)) == SYSERR) {
		printf("udpecho: open fails\n");
		return(SYSERR);
	}
	while ( TRUE ) {
		len = read(dev, (uint8_t *)buff, MAXECHO);
		write(dev, (uint8_t *)buff, len);
	}
}
