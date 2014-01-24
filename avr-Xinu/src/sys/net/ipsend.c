/* ipsend.c - ipsend */

#include <avr-Xinu.h>
#include <network.h>

/*
 *------------------------------------------------------------------------
 *  ipsend  -  fill in IP header and send datagram to specified address
 *------------------------------------------------------------------------
 */
 
int ipsend(IPaddr faddr, struct epacket *packet, int datalen)
{
	register struct	ip *ipptr;

	packet->ep_hdr.e_ptype = HTON16(EP_IP);
	ipptr = (struct ip *) packet->ep_data;
	ipptr->i_verlen = IVERLEN;
	ipptr->i_svctyp = ISVCTYP;
	ipptr->i_paclen = hton16( datalen+IPHLEN );
	ipptr->i_id = hton16(ipackid++);
	ipptr->i_fragoff = HTON16(IFRAGOFF);
	ipptr->i_tim2liv = ITIM2LIV;
	getaddr(ipptr->i_src);
	blkcopy(ipptr->i_dest, faddr, IPLEN);
	ipptr->i_cksum = hton16(~net_calc_checksum(0, (uint8_t*)ipptr, IPHLEN, IPCKOFF));
	return( route(faddr, packet, EHLEN+IPHLEN+datalen) );
}
int	ipackid = 1;