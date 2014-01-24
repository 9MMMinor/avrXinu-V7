/* x_net.c - x_net */

#include <avr-Xinu.h>
#include <network.h>

/*------------------------------------------------------------------------
 *  x_net  -  (command netstat) print network status information
 *------------------------------------------------------------------------
 */
COMMAND	x_net(int nargs, int *argv)
//int	stdin, stdout, stderr, nargs;
//char	*args[];
{
	struct	netq	*nqptr;
	char	str[80];
	int	i;

	sprintf(str,
		"bpool=%d, mutex/cnt=%d/%d, nxt prt=%d, addr %svalid\n",
		Net.netpool, Net.nmutex, scount(Net.nmutex), Net.nxtprt,
		Net.mavalid ? "" : "in");
	write(argv[1], (unsigned char *)str, strlen(str));
	sprintf(str,"Packets: recvd=%d, tossed=%d (%d for overrun)\n",
		Net.npacket, Net.ndrop, Net.nover);
	write(argv[1], (unsigned char *)str, strlen(str));
	sprintf(str,"         (%d missed: no buffer space)  (%d LANCE errors)\n",
		Net.nmiss, Net.nerror);
	write(argv[1], (unsigned char *)str, strlen(str));
	for (i=0; i<NETQS; i++) {
		nqptr = &Net.netqs[i];
		if (!nqptr->valid)
		  continue;
		sprintf(str,
			"%2d. uport=%4d, pid=%3d, xprt=%2d, size=%2d\n",
			i, nqptr->uport, nqptr->pid, nqptr->xport,
			pcount(nqptr->xport) );
		write(argv[1], (unsigned char *)str, strlen(str));
	}
	return(OK);
}
