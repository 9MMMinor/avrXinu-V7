/* arp_in.c - arp_in */

#include <avr-Xinu.h>
#include <network.h>


/*
 *------------------------------------------------------------------------
 *  arp_in  -  handle ARP packet coming in from Ethernet network
 *------------------------------------------------------------------------
 */

int arp_in(struct epacket *packet, int device)
{
	STATWORD ps;    
	int	pid;
	short	arop;
	struct	arppak	*apacptr;
	struct	arpent	*atabptr;
	struct	etblk	*etptr;	
	
	etptr = (struct etblk *) devtab[device].dvioblk;
	apacptr = (struct arppak *) packet->ep_data;
	atabptr = &Arp.arptab[arpfind(apacptr->ar_spa)];
	if (atabptr->arp_state != AR_RSLVD) {
		blkcopy((uint8_t *)atabptr->arp_Ead, (uint8_t *)apacptr->ar_sha, EPADLEN);
		atabptr->arp_dev = device;
		atabptr->arp_state = AR_RSLVD;
#ifdef DEBUG		
	        kprintf("arp_in: resolved addr for %d.%d.%d.%d\n",
			apacptr->ar_spa[0]&0xff,
			apacptr->ar_spa[1]&0xff,
			apacptr->ar_spa[2]&0xff,
			apacptr->ar_spa[3]&0xff);
#endif		
	}
	arop = net2hs(apacptr->ar_op);
	switch (arop) {

	case AR_REQ:	/* request - answer if for me */
		if (! blkequ(Net.myaddr, apacptr->ar_tpa, IPLEN)) {
			freebuf((int *)packet);
			return(OK);
		}
		apacptr->ar_op = hs2net(AR_RPLY);
		blkcopy((uint8_t *)apacptr->ar_tpa, (uint8_t *)apacptr->ar_spa, IPLEN);
		blkcopy((uint8_t *)apacptr->ar_tha, (uint8_t *)packet->ep_hdr.e_src, EPADLEN);
		blkcopy((uint8_t *)packet->ep_hdr.e_dest, (uint8_t *)apacptr->ar_tha, EPADLEN);
		blkcopy((uint8_t *)apacptr->ar_sha, (uint8_t *)etptr->etpaddr, EPADLEN);
		blkcopy((uint8_t *)apacptr->ar_spa, (uint8_t *)Net.myaddr, IPLEN);
		write(device, (uint8_t *)packet, EMINPAK);
		return(OK);

	case AR_RPLY:	/* reply - awaken requestor if any */
		disable(ps);
		pid = Arp.arppid;
		if (!isbadpid(pid)
		    && blkequ(Arp.arpwant, apacptr->ar_spa, IPLEN)) {
			Arp.arppid = BADPID;
			send(pid, OK);
		}
		freebuf((int *)packet);
		restore(ps);
		return(OK);

	default:
		Net.ndrop++;
		freebuf((int *)packet);
		return(SYSERR);
	}
}
