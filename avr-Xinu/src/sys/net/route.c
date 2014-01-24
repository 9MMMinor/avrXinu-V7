/* route.c - route */

#include <avr-Xinu.h>
#include <network.h>

static Bool isLocalMulticast(IPaddr destnet);
#ifdef DEBUG
static void kprint_IP(IPaddr);
#endif

/*
 *------------------------------------------------------------------------
 *  route  -  route a datagram to a given IP address
 *------------------------------------------------------------------------
 */

int route(IPaddr faddr, struct epacket *packet, int totlen)
{
	int	result;
	int	dev;
	struct	arpent	*arpptr;
	IPaddr	mynet, destnet;
	int x;

	/* If IP address is broadcast address for my network, then use	*/
	/* physical broadcast address.  Otherwise, establish a path to	*/
	/* the destination directly or through a gateway				*/

	getnet(mynet);
	netnum(destnet, faddr);
#ifdef DEBUG
	kprintf("route: mynet=");
	kprint_IP(mynet);
	kprintf(" destnet=");
	kprint_IP(destnet);
	kprintf(" dest=");
	kprint_IP(faddr);
	kprintf("\n");
#endif	
	wait(Net.nmutex);
	/* NOTE: This code uses host 0 as broadcast like 4.2bsd UNIX */
	if ( blkequ(mynet, faddr, IPLEN) || isLocalMulticast(destnet) )
		{
		dev = ETHER;
		blkcopy(packet->ep_hdr.e_dest, EBCAST, EPADLEN);
		}
	else
		{
		if (!blkequ(destnet, mynet, IPLEN))
			{
			faddr = Net.gateway;
//			kprintf(" gateway dest=");
//			kprint_IP(faddr);
//			kprintf("\n");
			}
		arpptr = &Arp.arptab[ (x=getpath(faddr)) ];
//		kprintf("route: getpath=%d\n",x);
		if (arpptr->arp_state != AR_RSLVD)
			{
			arpptr->arp_state = AR_RGATE;
			arpptr = &Arp.arptab[ (x=getpath(Net.gateway)) ];
//			kprintf("route: gateway getpath=%d\n",x);
			if (arpptr->arp_state != AR_RSLVD)
				{
				panic("route - Cannot reach gateway");
				freebuf((int *)packet);
				signal(Net.nmutex);
				return(SYSERR);
				}
			}
		dev = arpptr->arp_dev;
		blkcopy(packet->ep_hdr.e_dest, arpptr->arp_Ead, EPADLEN);
		}
	result = write(dev, (uint8_t *)packet, totlen);
	signal(Net.nmutex);
	return(result);
}

static Bool isLocalMulticast(IPaddr destnet)
{
	IPaddr localsubnet = {224,0,0,0};
	
	return ( (Bool)blkequ(localsubnet,destnet,IPLEN) );
}

#ifdef DEBUG
static void kprint_IP(IPaddr addr)
{

	kprintf("%d.%d.%d.%d", (int)addr[0]&0xFF, (int)addr[1]&0xFF,
			(int)addr[2]&0xFF, (int)addr[3]&0xFF);
}
#endif
