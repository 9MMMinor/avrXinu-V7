/* network.h */

/* All includes needed for the network */

#include <stdint.h>
#include <ether.h>
#include <ip.h>
#include <icmp.h>
#include <udp.h>
#include <net.h>
#include <dgram.h>
#include <arp.h>
#include <fserver.h>
#include <rfile.h>
#include <domain.h>
#include <netutil.h>
#include <dhcp_client.h>

/* just so the sources will compile */
#ifndef ETHER
#define ETHER -1
#endif
#ifndef INTERNET
#define INTERNET -1
#endif

/* Declarations data conversion and checksum routines */

/*********************** Motorola 68K ********************************/
//	extern short cksum();	/* 1s comp of 16-bit 2s comp sum*/
//
/* for talking to the Vaxen */
//#define h2vax(x) (unsigned) ((unsigned) ((x)>>8) + (unsigned)((x)<<8))
//#define vax2h(x) (unsigned) ((unsigned) ((x)>>8) + (unsigned)((x)<<8))
//
/* The Sun uses network byte order already!! */
//#define hs2net(x) (x)
//#define net2hs(x) (x)
//#define hl2net(x) (x)
//#define net2hl(x) (x)
//
/* network macros */
//#define hi8(x)   (unsigned char)  (((long) (x) >> 16) & 0x00ff)
//#define low16(x) (unsigned short) ((long) (x) & 0xffff)


/********************* VAX/PDP-11 ************************************/
//extern	short		hs2net();	/* host to network short	*/
//extern	short		net2hs();	/* network to host short	*/
//extern	long		hl2net();	/* host-to-network long		*/
//extern	long		net2hl();	/* network to host long		*/
//extern	long		hl2vax();	/* pdp11-to-vax long		*/
//extern	long		vax2hl();	/* vax-to-pdp11 long		*/


/********************* Intel ****************************************/
/*#if	BYTE_ORDER == LITTLE_ENDIAN
 *#define hs2net(x) (unsigned) ((((x)>>8) &0xff) | (((x) & 0xff)<<8))
 *#define	net2hs(x) hs2net(x)
 *
 *#define hl2net(x)	(((((x)& 0xff)<<24) | ((x)>>24) & 0xff) | \
 *(((x) & 0xff0000)>>8) | (((x) & 0xff00)<<8))
 *#define net2hl(x) hl2net(x)
 *#endif
 *
 *#if	BYTE_ORDER == BIG_ENDIAN
 *#define hs2net(x) (x)
 *#define net2hs(x) (x)
 *#define hl2net(x) (x)
 *#define net2hl(x) (x)
 *#endif
 *
 * //network macros
 *#define hi8(x)   (unsigned char)  (((long) (x) >> 16) & 0x00ff)
 *#define low16(x) (unsigned short) ((long) (x) & 0xffff)
 *
 *#define	BYTE(x, y)	((x)[(y)]&0xff)	// get byte "y" from ptr "x"
 */

/******************* ATMEGA (avr) ************************************/
extern uint32_t hton32(uint32_t h);
#define	BYTE_ORDER LITTLE_ENDIAN
/* host to network short	*/
#define hs2net(x) (uint16_t) ((((x)>>8) &0xff) | (((x) & 0xff)<<8))
/* network to host short	*/
#define	net2hs(x) hs2net(x)
/* host-to-network long		*/
/*#define hl2net(x)	(((((x)& 0xff)<<24) | ((x)>>24) & 0xff) | \	*/
/*					(((x) & 0xff0000)>>8) | (((x) & 0xff00)<<8)) */
#define hl2net(x) hton32((x))
/* network to host long		*/
#define net2hl(x) hl2net(x)

/* network macros */
#define hi8(x)   (uint8_t)  (((uint32_t) (x) >> 16) & 0x00ff)
#define low16(x) (uint16_t) ((uint32_t) (x) & 0xffff)

#define	BYTE(x, y)	((x)[(y)]&0xff)	/* get byte "y" from ptr "x" */

/* general use declarations */
SYSCALL	getaddr(IPaddr address);
SYSCALL	getnet(IPaddr address);
SYSCALL mkpool(int bufsiz, int numbufs);
SYSCALL ip2name(IPaddr ip, char *nam);

/* internal use declarations */
int netnum(IPaddr netpart, IPaddr address);
int freebuf(int *buf);
int *getbuf(int poolid);
int icmp_in(struct epacket *packet, int icmpp, int lim);
uint16_t net_calc_checksum(uint16_t checksum, const uint8_t* data, uint16_t data_len, uint8_t skip);
//NO!!! #define cksum(buf,nwords) ~net_calc_checksum(0,(buf),(nwords<<1),0)
int route(IPaddr faddr, struct epacket *packet, int totlen);
int getpath(IPaddr faddr);
int udpnxtp(void);
int ipsend(IPaddr faddr, struct epacket *packet, int datalen);
void dot2ip(char * ip, int nad1, int nad2, int nad3, int nad4);
int nqalloc(void);
int udpsend(IPaddr faddr, int fport, int lport, struct epacket *packet, int datalen);
