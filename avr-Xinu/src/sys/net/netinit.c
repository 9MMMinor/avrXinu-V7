/* netinit.c - netinit */

#include <avr-Xinu.h>
#include <sleep.h>
#include <network.h>

#ifndef GATEWAY
#define GATEWAY 192,168,1,1
#endif
#ifndef DHCP_REQUEST_IP
#define DHCP_REQUEST_IP 192,168,1,105
#endif


static char *local_name = "Xinu.local.";

/*
 *------------------------------------------------------------------------
 *  netinit  -  initialize network data structures
 *------------------------------------------------------------------------
 */

int netinit(void)
{
	struct	netq	*nqptr;
	int	i;

	/* Initialize pool of network buffers and rest of Net structure	*/

	if (clkruns == FALSE)
		panic("net: no clock");
	Net.netpool = mkpool(EMAXPAK, NETBUFS);
	for (i=0 ; i<NETQS ; i++) {
		nqptr = &Net.netqs[i];
		nqptr->valid = FALSE;
		nqptr->uport = -1;
		nqptr->xport = pcreate(NETQLEN);
	}
	Net.mnvalid = Net.mavalid = FALSE;
	dot2ip(Net.gateway, GATEWAY);
	dot2ip(Net.myaddr, DHCP_REQUEST_IP);
	netnum(Net.mynet, Net.myaddr);
	Net.nxtprt = ULPORT;
	Net.nmutex = screate(1);
	Net.npacket = Net.ndrop = Net.nover = Net.nmiss = Net.nerror = 0;
	blkcopy(Net.myname,local_name,strlen(local_name));
	return(OK);
}

struct netinfo Net;
