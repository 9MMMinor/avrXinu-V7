/* mkarp.c - mkarp */

#include <conf.h>
#include <kernel.h>
#include <network.h>

extern void packet_Dump(char *, struct epacket *, int);
extern int *getbuf(int poolid);
extern int blkcopy(uint8_t *to, uint8_t *from, int16_t nbytes);

/*
 *------------------------------------------------------------------------
 *  mkarp  -  allocate and fill in an ARP or RARP packet
 *------------------------------------------------------------------------
 */
 
struct	epacket	*mkarp(uint16_t typ, uint16_t op, IPaddr spaddr, IPaddr tpaddr)
{
	register struct arppak *apacptr;
	struct epacket *packet;

	packet = (struct epacket *) getbuf(Net.netpool);
	if ( ((int)packet) == SYSERR) {
		kprintf("mkarp: Bad Packet\n");
		return((struct epacket *)SYSERR);
	}
	blkcopy((uint8_t *)packet->ep_hdr.e_dest, (uint8_t *)EBCAST, AR_HLEN);
	packet->ep_hdr.e_ptype = hs2net(typ);
	apacptr = (struct arppak *) packet->ep_data;
	apacptr->ar_hrd = hs2net(AR_HRD);
	apacptr->ar_prot = hs2net(AR_PROT);
	apacptr->ar_hlen = AR_HLEN;
	apacptr->ar_plen = AR_PLEN;
	apacptr->ar_op = hs2net(op);
	blkcopy((uint8_t *)apacptr->ar_sha, (uint8_t *)eth[0].etpaddr, AR_HLEN);
	blkcopy((uint8_t *)apacptr->ar_spa, (uint8_t *)spaddr, AR_PLEN);
	blkcopy((uint8_t *)apacptr->ar_tha, (uint8_t *)eth[0].etpaddr, AR_HLEN);
	blkcopy((uint8_t *)apacptr->ar_tpa, (uint8_t *)tpaddr, AR_PLEN);
	return(packet);
}
