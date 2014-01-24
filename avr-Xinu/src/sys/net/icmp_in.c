/* icmp_in.c - icmp_in */

#include <avr-Xinu.h>
#include <network.h>

/*
 *------------------------------------------------------------------------
 *  icmp_in  -  handle ICMP packet coming in from the network
 *------------------------------------------------------------------------
 */
 
int icmp_in(struct epacket *packet, int icmpp, int lim)
{
	struct ip *ipptr;
	struct icmp *icmpptr;
	int len;

	ipptr = (struct ip *)packet->ep_data;
	icmpptr = (struct icmp *) ipptr->i_data;
	if (!Net.mavalid || icmpptr->ic_typ != ICRQECH) {
		freebuf((int *)packet);
	} else {
		icmpptr->ic_typ = (char) ICRPECH;
		blkcopy(ipptr->i_dest, ipptr->i_src, IPLEN);
		len = ntoh16(ipptr->i_paclen) - IPHLEN;
		if (isodd(len)) {
			ipptr->i_data[len++] = NULLCH;
		}
		icmpptr->ic_cksum = hton16(~net_calc_checksum(0, (uint8_t*)icmpptr, len, ICCKOFF));
		if (pcount(icmpp) < lim) {
			psend(icmpp, (int)packet);
		}
		else {
			freebuf((int *)packet);
		}
	}
	return(OK);
}
