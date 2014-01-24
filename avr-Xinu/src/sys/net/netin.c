/* netin.c */

#include <avr-Xinu.h>
#include <network.h>
#include <mark.h>
#include <bufpool.h>

void arpinit(void);
int netinit(void);
int dhcpInit(void);
int arp_in(struct epacket *packet, int device);
int rarp_in(struct epacket *packet, int device);
int ip_in(struct epacket *packet, int input2output_port, int lim);
void packet_Dump(char * routine, struct epacket *epkptr, int len);

extern const unsigned char mac[];

extern void panic(char * msg);
extern void ether_init(unsigned const char * MAC);

/*
 *------------------------------------------------------------------------
 *  netin - initialize net, start output side, and become input daemon
 *			resume userpid
 *------------------------------------------------------------------------
 */

//PROCESS	netin(int userpid)
PROCESS netin(int argc, int *argv)
{
	struct epacket *packet;
	int	input2output_port;		/* formerly icmpp */
	int packet_len;
	int userpid = 0;
	
	if (argc == 1)
		userpid = argv[0];
	else
		panic("netin: Bad argument\n");

	ether_init(mac);
	arpinit();
	netinit();
	dhcpInit();
	input2output_port = pcreate(NETFQ);
	resume( create(NETOUT, NETOSTK, NETIPRI-1, NETONAM,
			2, userpid, input2output_port) );
			
	
			
	for (packet = (struct epacket *)getbuf(Net.netpool) ; TRUE ;)
		{
		Net.npacket++;
		if ( (packet_len = read(ETHER, (void *)packet, sizeof(*packet))) == SYSERR )
			{
#ifdef DEBUG		    
			kprintf("bad return from read\n");
#endif			
			Net.ndrop++;
			continue;
			}
		switch ((unsigned short) net2hs(packet->ep_hdr.e_ptype))
			{
		    case EP_ARP:
#ifdef DEBUG		  
		        kprintf("ARP packet of len %d\n", packet_len);
#endif			
				arp_in(packet, ETHER);
				packet = (struct epacket *)getbuf(Net.netpool);
				break;

		    case EP_RARP:
#ifdef DEBUG
				kprintf("RARP packet of len %d\n", packet_len);
				packet_Dump("netin",packet,packet_len);
#endif			
				rarp_in(packet, ETHER);
				packet = (struct epacket *)getbuf(Net.netpool);
				break;

		    case EP_IP:
#ifdef DEBUG
				kprintf("IP packet of len %d\n", packet_len);
#endif			
				ip_in(packet, input2output_port, NETFQ);
				packet = (struct epacket *)getbuf(Net.netpool);
				break;

		    default: /* just drop packet */
#ifdef DEBUG
				kprintf("unknown packet type 0x%x and len %d\n",
					(unsigned short) net2hs(packet->ep_hdr.e_ptype),
					packet_len);
#endif				
				Net.ndrop++;
			}
		}
	return (OK);
}
