/* udpdump.c - udpdump */

#include <avr-Xinu.h>
#include <network.h>

void hexdump(uint8_t * buf, int dlen);
#define	EPO_UDP		4

/*
 *------------------------------------------------------------------------
 * udpdump - pretty-print a UDP packet to the console
 *------------------------------------------------------------------------
 */
 
void udpdump(struct udp *pudp, int dlen, int order)
{
	int	u_sport, u_dport, u_udplen, u_ucksum;

	if (order & EPO_UDP) {
		u_sport = pudp->u_sport;
		u_dport = pudp->u_dport;
		u_udplen = pudp->u_udplen;
		u_ucksum = pudp->u_ucksum;
	} else {
		u_sport = hs2net(pudp->u_sport);
		u_dport = hs2net(pudp->u_dport);
		u_udplen = hs2net(pudp->u_udplen);
		u_ucksum = hs2net(pudp->u_ucksum);
	}
	kprintf("UDP:\tsrc %5u dst %5u len %d cksum %04x\n",
		u_sport, u_dport, u_udplen, u_ucksum);
	hexdump((uint8_t *)pudp+UHLEN, dlen - UHLEN);
}
