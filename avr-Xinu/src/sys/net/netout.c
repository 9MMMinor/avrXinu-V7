/* netout.c - netout */

#include <avr-Xinu.h>
#include <network.h>

#define  MNAMLEN  24		/* maximum size of this machine's name	*/

SYSCALL	DHCP_getaddr(IPaddr address);
SYSCALL	gettime(long *timvar);
SYSCALL	getname(char *nam);
SYSCALL resume(int pid);
SYSCALL	preceive(int portid);
PROCESS set_Utime();
void panic(char * msg);
int ipsend(IPaddr faddr, struct epacket *packet, int datalen);

extern unsigned char mac[];

/*
 *------------------------------------------------------------------------
 *  netout  -  start network by finding address and forward IP packets
 *------------------------------------------------------------------------
 */
 
PROCESS netout(int argc, int *argv) 
//PROCESS	netout(int userpid, int icmpp)
{
	struct epacket *packet;
	struct ip *ipptr;
	long tim;
	int len;
	char nam[MNAMLEN];
	IPaddr addr;
	int userpid, input2output_port;
	
	if (argc != 2)
		panic("netout: Bad arguments\n");
	userpid = argv[0];
	input2output_port = argv[1];

	DHCP_getaddr(addr);
	gettime(&tim);
//	resume( create(set_Utime, 300, 20, "set_Time", 0) );
	getname(nam);

//#ifdef PRINTADDRESS	
	kprintf("Phys. address:  %02x:%02x:%02x:%02x:%02x:%02x\n",
		eth[0].etpaddr[0]&0xff,
		eth[0].etpaddr[1]&0xff,
		eth[0].etpaddr[2]&0xff,
		eth[0].etpaddr[3]&0xff,
		eth[0].etpaddr[4]&0xff,
		eth[0].etpaddr[5]&0xff);
	
	kprintf("IP address %d.%d.%d.%d\n",
		(unsigned short) Net.myaddr[0]&0xff,
		(unsigned short) Net.myaddr[1]&0xff,
		(unsigned short) Net.myaddr[2]&0xff,
		(unsigned short) Net.myaddr[3]&0xff);
	
	kprintf("Domain name: %s\n", nam);
//#endif

	resume(userpid);
	while (TRUE) {
		packet = (struct epacket *)preceive(input2output_port);
		ipptr = (struct ip *)packet->ep_data;
		switch (ipptr->i_proto)	{
		
		case IPRO_ICMP:
			blkcopy((uint8_t *)addr, (uint8_t *)ipptr->i_dest, IPLEN);
			len = net2hs(ipptr->i_paclen) - IPHLEN;
			ipsend(addr, packet, len);
			break;
		case IPRO_UDP:
			kprintf("Netout: UDP\n");
//			dhcp_client_send();
			break;
		}
	}
	/* NOTREACHED */
	return (0);
}
