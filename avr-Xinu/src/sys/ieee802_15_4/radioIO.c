//
//  radioIO.c - radio_init radio_in radio_register radio_recv radio_recvaddr
//				radio_send radio_release
//
//  802_15_4_Mib
//
//  Created by Michael Minor on 3/31/14.
//  Copyright (c) 2014.
//
//
//

#include <avr-Xinu.h>
#include "registermap.h"
#include "frame802154.h"
#include "mib.h"
#include "macSymbolCounter.h"
#include "radioIO.h"
#include "radio.h"

struct radioinfo Radio;
struct radioentry radiotab[RADIO_SLOTS];		/* table of RADIO endpts	*/
int timeEvent;									/* fast timer message		*/
frame802154_t *currpkt;		/* packet buffer being used now	*/
struct radioinfo Net;		/* local network interface	*/

/*
 *------------------------------------------------------------------------
 *  netInit  -  initialize radio (IEEE 802.15.4) network data structures
 *------------------------------------------------------------------------
 */

int netInit(void)
{
	/* Initialize pool of network buffers and rest of Radio structure	*/
	
	Net.radiopool = mkpool(sizeof(frame802154_t), RADIOBUFS);
	Net.nmutex = screate(1);
//	Net.fiport = pcreate(RADIO_QUEUE_LEN);		/* input */
//	Net.foport = pcreate(RADIO_QUEUE_LEN);		/* output */
	Net.npacket = Net.ndrop = Net.nover = Net.nmiss = Net.nerror = 0;
	
	/* initialize PAN ID */
	macPANId = 101;				/* arbitrary */
	
	/* initialize Personal Area Network ID and MAC Short Address Registers	*/
	/* for level 3 frame filtering											*/
	SHORT_ADDR_0 = lobyte(macShortAddress);
	SHORT_ADDR_1 = hibyte(macShortAddress);
	PAN_ID_0 = lobyte(macPANId);
	PAN_ID_1 = hibyte(macPANId);
	return(OK);
}

/*
 *------------------------------------------------------------------------
 * netin - continuously read the next incoming packet and handle it
 *------------------------------------------------------------------------
 */

PROCESS	netin(int argc, int *argv)
{
	int	retval;			/* return value from function	*/
	int userpid = 0;
	
	if (argc != 1)
		panic("netin: Bad argument\n");
	userpid = argv[0];
	
	/* initialze other parts of radio */
	macSymbolCounterInit();
	radioTimerEventInit();
	radio_init();				/* initializes low-level 802.15.4 PHY + MAC */
	mac_init();					/* Copy radio address to global variable */

	netInit();
	
	resume ( userpid );			/* = main */
	
	if (Net.radiopool == SYSERR) {
		kprintf("Cannot allocate network buffer pool");
		kill(getpid());
	}
	
		
	/* Indicate that IP address, mask, and router are not yet valid	*/
	
	Net.addressvalid = TRUE;
	
	/* Initialize radio buffer table */
	
	radio_buffer_init();
	
	currpkt = (frame802154_t *)getbuf(Net.radiopool);
	
	/* Do forever: read packets from the network and process */
	
	while (TRUE) {
	    retval = read(RADIO, (unsigned char *)currpkt, sizeof(frame802154_t));
	    if (retval == SYSERR) {
			panic("Radio read error");
	    }
		
	    /* Demultiplex on radio type */
//		frameHeaderDump("netin",currpkt,sizeof(frame802154_t));
		
	    switch (currpkt->fcf.frameType) {
			case FRAME_TYPE_BEACON:
				break;
			case FRAME_TYPE_DATA:
				radio_in();					/* handle data input */
				break;
			case FRAME_TYPE_MAC_COMMAND:
				break;
			default:
			case FRAME_TYPE_ACK:							/* ack frames intercepted in driver */
				Net.nerror++;
				Net.ndrop++;
				break;
		}
	}
}


/**
 *------------------------------------------------------------------------
 * radio_buffer_init - initialize RADIO endpoint table
 *------------------------------------------------------------------------
 */
void radio_buffer_init(void)
{
	uint8_t	i;			/* table index */
	
	for(i=0; i<RADIO_SLOTS; i++) {
		radiotab[i].rdstate = RADIO_FREE;
	}
	
	return;
}

/**
 *------------------------------------------------------------------------
 * radio_in - handle an incoming RADIO packet
 *------------------------------------------------------------------------
 */
void radio_in(void)					/* currpkt points to the packet	*/
{
	STATWORD ps;
	uint8_t	i;						/* index into radiotab		*/
	int j,k;
	struct radioentry *radiotr;		/* pointer to radiotab entry	*/
	
	for (i=0; i<RADIO_SLOTS; i++) {
		uint8_t e0, e1, e2, e3;
	    radiotr = &radiotab[i];
		j = (int)IRQ_STATUS&0xff;
		k = (int)IRQ_STATUS1&0xff;
		kprintf("IRQ status = %x %x\n", j, k);
		e0 = radiotr->rdstate != RADIO_FREE;
		e1 = ( (currpkt->fcf.framePanIDCompress == 0) && (currpkt->fcf.frameDestinationAddressMode != FRAME_ADDRESS_MODE_NONE) );
		e1 = e1 && ( (currpkt->dest_pid == macPANId) || (currpkt->dest_pid == 0xffff) );
		e2 = (currpkt->fcf.frameDestinationAddressMode == FRAME_ADDRESS_MODE_SHORT);
		e2 = e2 && ( (currpkt->dest_addr.saddr == macShortAddress) || (currpkt->dest_addr.saddr == 0xffff) );
		e3 = (currpkt->fcf.frameDestinationAddressMode == FRAME_ADDRESS_MODE_EXTENDED);
		e3 = e3 && (currpkt->dest_addr.eaddr == macExtAddress);
		kprintf("radio_in: %x %x %x %x\n",e0, e1, e2, e3);
	    if ( e0 && (IRQ_status1.AMI_Maf0 == 1) )	{
			IRQ_status1.AMI_Maf0 = 1;					/* clear bit */
			
			/* Level 3 hardware matches incoming packet */
			/* Frame Filtering is an evaluation whether or not a received frame is dedicated for this
			 * node. To accept a received frame and to generate an address match interrupt
			 * (TRX24_AMI) a filtering procedure as described in IEEE 802.15.4-2006 chapter 7.5.6.2.
			 * (Third level of filtering) is applied to the frame:
			 * — The Frame Type subfield shall not contain a reserved frame type.
			 * — The Frame Version subfield shall not contain a reserved value.
			 * — If a destination PAN identifier is included in the frame, it shall match macPANId or shall be the
			 *   broadcast PAN identifier (0xffff).
			 * — If a short destination address is included in the frame, it shall match either macShortAddress or the
			 *   broadcast address (0xffff). Otherwise, if an extended destination address is included in the frame, it
			 *   shall match aExtendedAddress.
			 * — If the frame type indicates that the frame is a beacon frame, the source PAN identifier shall match
			 *   macPANId unless macPANId is equal to 0xffff, in which case the beacon frame shall be accepted
			 *   regardless of the source PAN identifier.
			 * — If only source addressing fields are included in a data or MAC command frame, the frame shall be
			 *   accepted only if the device is the PAN coordinator and the source PAN identifier matches
			 *   macPANId.
			 */
			
			if (radiotr->rdcount < RADIO_QSIZ) {
				disable(ps);										/* ++++++ */
				radiotr->rdcount++;
				radiotr->rdqueue[radiotr->rdtail++] = currpkt;
				if (radiotr->rdtail >= RADIO_QSIZ) {
					radiotr->rdtail = 0;
				}
				currpkt = (frame802154_t *)getbuf(Net.radiopool);
				if (radiotr->rdstate == RADIO_RECV) {
					radiotr->rdstate = RADIO_USED;
					send(radiotr->rdpid, OK);
				}
				restore(ps);										/* ++++++ */
				return;
			}
	    }
	}
	
	/* no match - simply discard packet */
	
	return;
}

/**
 *-------------------------------------------------------------------------
 * radio_register - register a destination (address,panID) and source panID
 *		to receive incoming RADIO messages from the specified destination
 *		site sent to a specific source panID
 *-------------------------------------------------------------------------
 */
int radio_register	(
					ShortAddr_t dest_addr,	/* destination address address or zero	*/
					PanId_t dest_panID,		/* destination RADIO protocol panID	*/
					PanId_t src_panID		/* source RADIO protocol panID	*/
					)
{
	uint8_t	i;					/* index into radiotab		*/
	struct radioentry *radiotr;	/* pointer to radiotab entry	*/
	
	/* See if request already registered */
	
	for (i=0; i<RADIO_SLOTS; i++) {
		radiotr = &radiotab[i];
		if (radiotr->rdstate == RADIO_FREE) {
			continue;
		}
		if ((dest_panID == radiotr->rddest_panID) &&
			(src_panID == radiotr->rdsrc_panID) &&
			(dest_addr   == radiotr->rddest_addr  ) ) {
			
			/* Entry in table matches request */
			
			return SYSERR;
		}
	}
	
	/* Find a free slot and allocate it */
	
	for (i=0; i<RADIO_SLOTS; i++) {
		radiotr = &radiotab[i];
		if (radiotr->rdstate == RADIO_FREE) {
			radiotr->rdstate = RADIO_USED;
			radiotr->rddest_addr = dest_addr;
			radiotr->rddest_panID = dest_panID;
			radiotr->rdsrc_panID = src_panID;
			radiotr->rdcount = 0;
			radiotr->rdhead = radiotr->rdtail = 0;
			radiotr->rdpid = -1;
			return OK;
		}
	}
	
	return SYSERR;
}

/**
 *------------------------------------------------------------------------
 * radio_recv - receive a RADIO packet
 *------------------------------------------------------------------------
 */
int radio_recv	(
					ShortAddr_t dest_addr,	/* destination address address or 0xffff	*/
				  	PanId_t dest_panID,		/* destination RADIO protocol panID			*/
				  	PanId_t src_panID,		/* source RADIO protocol panID				*/
				  	octet_t *buff,			/* buffer to hold RADIO data				*/
				  	uint8_t len,			/* length of buffer							*/
				  	uint32_t timeout		/* read timeout in Symbol Times				*/
					)
{
	STATWORD ps;
	uint8_t	i;			/* index into radiotab		*/
	struct radioentry *radiotr;	/* pointer to radiotab entry	*/
	int	msg;			/* message from recvtime()	*/
	frame802154_t *pkt;		/* ptr to packet being read	*/
	uint8_t	msglen;			/* length of RADIO data in packet	*/
	octet_t *rdataptr;		/* pointer to RADIO data		*/
	
	disable(ps);
	for (i=0; i<RADIO_SLOTS; i++) {
		radiotr = &radiotab[i];
		if ( (dest_panID == radiotr->rddest_panID) &&
			 (src_panID == radiotr->rdsrc_panID) &&
			 (dest_addr == radiotr->rddest_addr) ) {
			
			/* Entry in table matches request */
//			kprintf("radio_recv: table %d match\n",i);
			
			break;
		}
	}
	
	if (i >= RADIO_SLOTS) {
		restore(ps);
		return SYSERR;
	}
	
	if (radiotr->rdcount == 0) {		/* No packet is waiting	*/
		radiotr->rdstate = RADIO_RECV;
		radiotr->rdpid = currpid;
		msg = recvclr();				/* remember: No Packet yet */
		msg = recvtim(1);				/* wait for packet with slow (1 TICK) timeout */
//		msg = recvOrTimeout(timeout);
		radiotr->rdstate = RADIO_USED;
		if (msg == TIMEOUT) {
			kprintf("recv: TIMEOUT\n");
			restore(ps);
			return TIMEOUT;
		}
		if (msg != OK) {
			kprintf("recv: NOT OK\n");
			restore(ps);
			return SYSERR;
		}
	}
	
	/* Packet has arrived -- dequeue it */
	
	pkt = radiotr->rdqueue[radiotr->rdhead++];
	if (radiotr->rdhead >= RADIO_QSIZ) {
		radiotr->rdhead = 0;
	}
	radiotr->rdcount--;
	
	/* Copy RADIO data from packet into caller's buffer */
//	frameHeaderDump("radio_recv", pkt, len);
	
	msglen = pkt->data_len;
	rdataptr = (octet_t *)pkt->data;
	for (i=0; i<msglen; i++) {
		if (i >= len) {
			break;
		}
		*buff++ = *rdataptr++;
	}
	freebuf((int *)pkt);
	restore(ps);
	return i;
}

/**
 *------------------------------------------------------------------------
 * radio_clear - clear previously recevied RADIO packets
 *					returns number of packets dumped
 *------------------------------------------------------------------------
 */
int radio_clear	 (
				 ShortAddr_t dest_addr,	/* destination address address or 0xffff	*/
				 PanId_t dest_panID,	/* destination RADIO protocol panID			*/
				 PanId_t src_panID		/* source RADIO protocol panID				*/
				 )
{
	STATWORD ps;
	int	i;			/* index into radiotab		*/
	struct radioentry *radiotr;	/* pointer to radiotab entry	*/
	frame802154_t *pkt;		/* ptr to packet being read	*/
	
	disable(ps);
	for (i=0; i<RADIO_SLOTS; i++) {
		radiotr = &radiotab[i];
		if ( (dest_panID == radiotr->rddest_panID) &&
			(src_panID == radiotr->rdsrc_panID) &&
			(dest_addr == radiotr->rddest_addr) ) {
			
			/* Entry in table matches request */
//			kprintf("radio_clear: table %d match\n",i);
			
			break;
		}
	}
	
	if (i >= RADIO_SLOTS) {
		restore(ps);
		return SYSERR;
	}
	
	for (i=0; radiotr->rdcount > 0; i++) {
		pkt = radiotr->rdqueue[radiotr->rdhead++];
		if (radiotr->rdhead >= RADIO_QSIZ) {
			radiotr->rdhead = 0;
		}
		freebuf((int *)pkt);
		radiotr->rdcount--;
	}
	
	restore(ps);
	return i;
}

/**
 *------------------------------------------------------------------------
 * radio_recvaddr - receive a RADIO packet and record the sender's address
 *------------------------------------------------------------------------
 */
int	radio_recvaddr	 (
					 ShortAddr_t *dest_addr,	/* ptr destination address */
					 PanId_t *dest_panID,		/* ptr destination RADIO protocol panID	*/
					 PanId_t src_panID,			/* source RADIO protocol panID	*/
					 octet_t *buff,				/* buffer to hold RADIO data	*/
					 uint8_t len,				/* length of buffer		*/
					 uint32_t timeout			/* read timeout in symbol times		*/
					 )
{
	STATWORD ps;				/* interrupt mask		*/
	uint8_t	i;					/* index into radiotab		*/
	struct radioentry *radiotr;	/* pointer to radiotab entry	*/
	int	msg;					/* message from recvtime()	*/
	frame802154_t *pkt;			/* ptr to packet being read	*/
	uint8_t	msglen;				/* length of RADIO data in packet	*/
	octet_t	*rdataptr;			/* pointer to RADIO data		*/
	
	disable(ps);
	for (i=0; i<RADIO_SLOTS; i++) {
		radiotr = &radiotab[i];
		if ( (radiotr->rddest_addr == BROADCAST_ADDR ) &&
			(src_panID == radiotr->rdsrc_panID) ) {
			
			/* Entry in table matches request */
			break;
		}
	}
	
	if (i >= RADIO_SLOTS) {
		restore(ps);
		return SYSERR;
	}
	
	if (radiotr->rdcount == 0) {	/* no packet is waiting */
		radiotr->rdstate = RADIO_RECV;
		radiotr->rdpid = currpid;
		msg = recvclr();
//		msg = recvtime(timeout);	/* wait for packet with slow timeout */
//		timeEvent = currpid;
//		tmset(timerPortID, &timeEvent, timeout, &setTimeOut); /* to expire in timeout Symbol Times */
		msg = recvOrTimeout(timeout);
		radiotr->rdstate = RADIO_USED;
		if (msg == TIMEOUT) {
			restore(ps);
			return TIMEOUT;
		}
		if (msg != OK) {
			restore(ps);
			return SYSERR;
		}
	}
	
	
	/* Packet has arrived -- dequeue it */
	pkt = radiotr->rdqueue[radiotr->rdhead++];
	if (radiotr->rdhead >= RADIO_QSIZ) {
		radiotr->rdhead = 0;
	}
	radiotr->rdcount--;
	
	/* Record sender's address address and RADIO panID number */
	
	*dest_addr = pkt->src_addr.saddr;
	*dest_panID = pkt->src_pid;
	
	/* Copy RADIO data from packet into caller's buffer */
	
	msglen = pkt->data_len;
	rdataptr = pkt->data;
	for (i=0; i<msglen; i++) {
		if (i >= len) {
			break;
		}
		*buff++ = *rdataptr++;
	}
	freebuf((int *)pkt);
	restore(ps);
	return i;
}

/**
 *------------------------------------------------------------------------
 * radio_send - send a RADIO packet
 *------------------------------------------------------------------------
 */
int radio_send (
				  ShortAddr_t dest_addr,		/* destination address address or address_BCAST*/
				  PanId_t dest_panID,			/* destination RADIO protocol panID	*/
				  ShortAddr_t src_addr,			/* source address		*/
				  PanId_t src_panID,			/* source RADIO protocol panID	*/
				  octet_t *buff,				/* buffer of RADIO data		*/
				  uint8_t len					/* length of data in buffer	*/
				  )
{
	frame802154_t *pkt;			/* source packet buffer		*/
	uint8_t	pktlen;				/* total packet length		*/
//	static	uint16 ident = 1;	/* datagram IDENT field		*/
	octet_t	*rdataptr;			/* pointer to RADIO data	*/
//	byte	radiobcast[] = {0xff,0xff,0xff,0xff,0xff,0xff};	/* LOOKING FOR BEACON??? */
	
	
	/* Create RADIO packet in pkt */
	pkt = (frame802154_t *)getbuf(Net.radiopool);
	/* fill every field! */
	pkt->fcf.frameType = FRAME_TYPE_DATA;
	pkt->fcf.frameSecurity = 0;
	pkt->fcf.frameAckRequested = 0;
	pkt->fcf.framePending = 0;
	pkt->fcf.frameVersion = FRAME_VERSION_2006;
	pkt->fcf.frameDestinationAddressMode = FRAME_ADDRESS_MODE_SHORT;
	pkt->fcf.framePanIDCompress = 0;
	pkt->fcf.frameSourceAddressMode = FRAME_ADDRESS_MODE_SHORT;
	pkt->seq = macBSN++;
	pkt->dest_pid = dest_panID;
	pkt->dest_addr.saddr = dest_addr;
	pkt->src_pid = src_panID;
	pkt->src_addr.saddr = src_addr;
	pkt->header_len = getFrameHdrLength(pkt);
	pkt->data_len = len;
	
	rdataptr = pkt->data;
	for (; len>0; len--) {
		*rdataptr++ = *buff++;
	}

	
	/* Compute packet length as RADIO data size + header size + footer size	*/
	
	pktlen = pkt->data_len + pkt->header_len + FTR_LEN;
//	frameHeaderDump("radio_send", &pkt, pktlen);
	write(RADIO, (unsigned char *)pkt, pktlen);
	freebuf((int *)pkt);
	return OK;
}

/**
 *------------------------------------------------------------------------
 * radio_release - release a previously-registered destination address, destination
 *			panID, and source panID (exact match required)
 *------------------------------------------------------------------------
 */
int radio_release	 (
					 ShortAddr_t dest_addr,	/* destination address address or zero	*/
					 PanId_t dest_panID,	/* destination RADIO protocol panID	*/
					 PanId_t src_panID		/* source RADIO protocol panID	*/
					 )
{
	uint8_t	i;					/* index into radiotab		*/
	struct radioentry *radiotr;	/* pointer to radiotab entry	*/
	frame802154_t *pkt;			/* ptr to packet being read	*/
	
	for (i=0; i<RADIO_SLOTS; i++) {
		radiotr = &radiotab[i];
		if (radiotr->rdstate != RADIO_USED) {
			continue;
		}
		if ((dest_panID == radiotr->rddest_panID) &&
			(src_panID == radiotr->rdsrc_panID) &&
			(dest_addr   == radiotr->rddest_addr  ) ) {
			
			/* Entry in table matches */
			
			kprintf("release: %d packets\n", radiotr->rdcount);
			while (radiotr->rdcount > 0) {
				pkt = radiotr->rdqueue[radiotr->rdhead++];
				if (radiotr->rdhead >= RADIO_QSIZ) {
					radiotr->rdhead = 0;
				}
				freebuf((int *)pkt);
				radiotr->rdcount--;
			}
			radiotr->rdstate = RADIO_FREE;
			return OK;
		}
	}
	return SYSERR;
}
