/* rarp_in.c - rarp_in */

#include <avr-Xinu.h>
#include <network.h>

/*
 *------------------------------------------------------------------------
 *  rarp_in  -  handle RARP packet coming in from Ethernet network
 *------------------------------------------------------------------------
 */
 
int rarp_in(struct epacket *packet, int device)
{
	STATWORD ps;    
	int	pid;
	int	ret;
	struct	arppak	*apacptr;
	struct	etblk	*etptr;
	int rarp_reply;

	apacptr = (struct arppak *) packet->ep_data;
	rarp_reply = net2hs(apacptr->ar_op);
#ifdef DEBUG
	kprintf("rarp_in: rarp_reply = %d\n", rarp_reply);
#endif
	if (rarp_reply == AR_RRLY) {
		etptr = &eth[devtab[device].dvminor];
#ifdef DEBUG
		kprintf("%02X%02X%02X%02X%02X%02X\n",
				apacptr->ar_tha[0],
				apacptr->ar_tha[1],
				apacptr->ar_tha[2],
				apacptr->ar_tha[3],
				apacptr->ar_tha[4],
				apacptr->ar_tha[5]);
		kprintf("%02X%02X%02X%02X%02X%02X\n",
				etptr->etpaddr[0],
				etptr->etpaddr[1],
				etptr->etpaddr[2],
				etptr->etpaddr[3],
				etptr->etpaddr[4],
				etptr->etpaddr[5]);
#endif
		if ( blkequ(apacptr->ar_tha,etptr->etpaddr,EPADLEN) ) {
			blkcopy(Net.myaddr, apacptr->ar_tpa, IPLEN);
			netnum(Net.mynet, Net.myaddr);
			disable(ps);
			Net.mavalid = TRUE;
			pid = Arp.rarppid;
			if (!isbadpid(pid)) {
				Arp.rarppid = BADPID;
				send(pid, OK);
			}
			restore(ps);
		}
		ret = OK;
	} else
		ret = SYSERR;
	freebuf((int *)packet);
	return(ret);
}
