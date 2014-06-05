//
//  radioIO.c - radio_init radio_in radio_register radio_recv_data radio_recv_frame
//				radio_recvaddr radio_send radio_release
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
	Net.npacket = Net.ndata = Net.nbeacon = Net.ncmd = 0;
	Net.ndrop = Net.nover = Net.nerror = 0;
	
	/* initialize PAN ID */
	macPromiscuousMode = FALSE;
	macPANId = 101;				/* arbitrary */
	
	/* initialize Personal Area Network ID and MAC Short Address Registers	*/
	/* for level 3 frame filtering */
	radio_set_short_address(macShortAddress);
	radio_set_pan_id(macPANId);
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
		Net.npacket++;
	    /* Demultiplex on radio type */
//		frameHeaderDump("netin",currpkt,sizeof(frame802154_t));
		
	    switch (currpkt->fcf.frameType) {
			case FRAME_TYPE_BEACON:
				Net.nbeacon++;
				break;
			case FRAME_TYPE_DATA:
				Net.ndata++;
				radio_in();					/* handle data input */
				break;
			case FRAME_TYPE_MAC_COMMAND:
				Net.ncmd++;
				break;
			default:
			case FRAME_TYPE_ACK:			/* ack frames intercepted in driver */
				Net.nerror++;
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
//	int j,k;
	struct radioentry *radiotr;		/* pointer to radiotab entry	*/
	
	for (i=0; i<RADIO_SLOTS; i++) {
//		uint8_t e0, e1, e2, e3;
	    radiotr = &radiotab[i];
//		j = (int)IRQ_STATUS&0xff;
//		k = (int)IRQ_STATUS1&0xff;
//		kprintf("IRQ status = %x %x\n", j, k);
//		e0 = radiotr->rdstate != RADIO_FREE;
//		e1 = ( (currpkt->fcf.framePanIDCompress == 0) && (currpkt->fcf.frameDestinationAddressMode != FRAME_ADDRESS_MODE_NONE) );
//		e1 = e1 && ( (currpkt->dest_pid == macPANId) || (currpkt->dest_pid == 0xffff) );
//		e2 = (currpkt->fcf.frameDestinationAddressMode == FRAME_ADDRESS_MODE_SHORT);
//		e2 = e2 && ( (currpkt->dest_addr.saddr == macShortAddress) || (currpkt->dest_addr.saddr == 0xffff) );
//		e3 = (currpkt->fcf.frameDestinationAddressMode == FRAME_ADDRESS_MODE_EXTENDED);
//		e3 = e3 && (currpkt->dest_addr.eaddr == macExtAddress);
//		kprintf("radio_in: %x %x %x %x\n",e0, e1, e2, e3);
	    if ( (radiotr->rdstate != RADIO_FREE) && (IRQ_status1.AMI_Maf0 == 1) )	{
			IRQ_status1.AMI_Maf0 = 1;					/* clear bit */
			
			/* For the first level of filtering, the MAC sublayer shall discard all received frames
			 * that do not contain a correct value in their FCS field in the MFR (see 7.2.1.9).
			 */
			
			/* The second level of filtering shall be dependent on whether the MAC sublayer is
			 * currently operating in promiscuous mode. In promiscuous mode, the MAC sublayer
			 * shall pass all frames received after the first filter directly to the upper layers
			 * without applying any more filtering or processing. The MAC sublayer shall be in
			 * promiscuous mode if macPromiscuousMode is set to TRUE.
			 */
			
			/* Level 3 hardware matches incoming packet ((rfr2-register)) */
			/* Frame Filtering is an evaluation whether or not a received frame is dedicated for this
			 * node. To accept a received frame and to generate an address match interrupt
			 * (TRX24_AMI) a filtering procedure as described in IEEE 802.15.4-2006 chapter 7.5.6.2.
			 * (Third level of filtering) is applied to the frame:
			 * — The Frame Type subfield shall not contain a reserved frame type.
			 * — The Frame Version subfield shall not contain a reserved value.
			 * — If a destination PAN identifier is included in the frame, it shall match macPANId ((PAN_ID_0, PAN_ID_1))
			 *   or shall be the broadcast PAN identifier (0xffff).
			 * — If a short destination address is included in the frame, it shall match either macShortAddress
			 *   ((SHORT_ADDR_0, SHORT_ADDR_1)) or the broadcast address (0xffff).
			 *   Otherwise, if an extended destination address is included in the frame, it shall match macExtAddress.
			 *   ((IEEE_ADDR_0 ... IEEE_ADDR_7))
			 * — If the frame type indicates that the frame is a beacon frame, the source PAN identifier shall match
			 *   macPANId ((PAN_ID_0, PAN_ID_1)) unless macPANId is equal to 0xffff, in which case the beacon frame shall
			 *   be accepted regardless of the source PAN identifier.
			 * — If only source addressing fields are included in a data or MAC command frame, the frame shall be
			 *   accepted only if the device is the PAN coordinator ((CSMA_seed1.AACK_I_AM_Coord=1)) and the source PAN
			 *   identifier matches macPANId ((PAN_ID_0, PAN_ID_1)).
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
			else	{
				Net.nover++;
			}
	    }
	}
	
	/* no match - simply discard packet */
	Net.ndrop++;
	return;
}

/**
 *------------------------------------------------------------------------
 *  radio_alloc  -  register a destination (address,panID) and source panID
 *		to receive incoming RADIO messages from the specified destination
 *		site sent to a specific source panID.
 *		Allocate a free slot and return descriptor or SYSERR if one can not
 *		be allocated.  Functions like a call to open for the Datagram
 *		Pseudo-Device.  Radio_alloc must be called before a call to
 *		radio_read().
 *------------------------------------------------------------------------
 */

int
radio_alloc(
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
			
			return (i);
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
			radiotr->rdtimeout = FALSE;
			return (i);
		}
	}
	return (SYSERR);
}


/**
 *-------------------------------------------------------------------------
 * radio_register - register a destination (address,panID) and source panID
 *		to receive incoming RADIO messages from the specified destination
 *		site sent to a specific source panID
 *-------------------------------------------------------------------------
 */
int
radio_register(
			  ShortAddr_t dest_addr,	// destination address address or zero
			  PanId_t dest_panID,		// destination RADIO protocol panID
			  PanId_t src_panID			// source RADIO protocol panID
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

int
radio_getSlot(
			  ShortAddr_t dest_addr,	// destination address address or zero
			  PanId_t dest_panID,		// destination RADIO protocol panID
			  PanId_t src_panID			// source RADIO protocol panID
			)
{
	int i;
	struct radioentry *radiotr;		/* pointer to radiotab entry	*/
	
	for (i=0; i<RADIO_SLOTS; i++) {
		radiotr = &radiotab[i];
		if (radiotr->rdstate == RADIO_FREE) {
			continue;
		}
		if ((dest_panID == radiotr->rddest_panID) &&
			(src_panID == radiotr->rdsrc_panID) &&
			(dest_addr   == radiotr->rddest_addr  ) ) {
			
			/* Entry in table matches request */
			
			return i;
		}
	}
	return SYSERR;
}

/**
 *------------------------------------------------------------------------
 *  radio_read_data  -  read one data packet from a radio I/O slot
 *
 *	This is a prototype for a Xinu pseudo driver for a specific protocol
 *	much like the Datagram Device driver.
 *	n = read(RADIODATA0, octet_t *buff, int len);
 *	returns the actual number of bytes in the data packet or error code
 *	to the buffer, buff, of length, len.
 *------------------------------------------------------------------------
 */

int
radio_read_data(uint8_t slot, octet_t *buff, int len)
{
	struct radioentry *radioptr;	/* pointer to radiotab entry	*/
	frame802154_t *frame;			/* ptr to frame being read	*/
	int i, datalen;
	octet_t *rdataptr;
	
	radioptr = &radiotab[slot];
	if (radioptr->rdstate == RADIO_FREE)	{
		radioptr->rdstatus = RDNOSLOT;
		return (SYSERR);
	}
	
	if (radioptr->rdcount == 0) {	/* queue is empty */
		radioptr->rdstate = RADIO_RECV;
		radioptr->rdpid = getpid();
		recvclr();					/* remember: No Packet yet */
		if (radioptr->rdtimeout)	{
			if (recvOrTimeout(RDTIMEOUT_TIME) == TIMEOUT)	{
				radioptr->rdstatus = RDTIMEOUT;
				return (TIMEOUT);
			}
		} else if (receive() != OK)	{
			radioptr->rdstatus = RDBAD;
			return (SYSERR);
		}
	}
	/* Frame has arrived -- dequeue it */
	frame = radioptr->rdqueue[radioptr->rdhead++];
	if (radioptr->rdhead >= RADIO_QSIZ) {
		radioptr->rdhead = 0;
	}
	radioptr->rdcount--;
	radioptr->rdstatus = RDSUCCESS;
	
	/* Copy RADIO data from packet into caller's buffer */
	//	frameHeaderDump("radio_recv", pkt, len);
	
	datalen = frame->data_len;
	rdataptr = (octet_t *)frame->data;
	for (i=0; i<datalen; i++) {
		if (i >= len) {
			break;
		}
		*buff++ = *rdataptr++;
	}
	freebuf((int *)frame);
	
	return(len);
}

/**
 *------------------------------------------------------------------------
 * radio_recv_frame - receive RADIO frame.
 *------------------------------------------------------------------------
 */
frame802154_t *
radio_recv_frame	(
					ShortAddr_t dest_addr,	/* destination address address or 0xffff	*/
					PanId_t dest_panID,		/* destination RADIO protocol panID			*/
					PanId_t src_panID,		/* source RADIO protocol panID				*/
					uint32_t timeout		/* read timeout in Symbol Times				*/
					)
{
	STATWORD ps;
	uint8_t	i;			/* index into radiotab		*/
	struct radioentry *radiotr;	/* pointer to radiotab entry	*/
	int	msg;			/* message from recvtime()	*/
	frame802154_t *pkt;		/* ptr to packet being read	*/
	
	disable(ps);
	for (i=0; i<RADIO_SLOTS; i++) {
		radiotr = &radiotab[i];
		if ( (dest_panID == radiotr->rddest_panID) &&
			(src_panID == radiotr->rdsrc_panID) &&
			(dest_addr == radiotr->rddest_addr) ) {
			
			/* Entry in table matches request */
//			kprintf("radio_recv_frame: table %d match\n",i);
			
			break;
		}
	}
	
	if (i >= RADIO_SLOTS) {
		restore(ps);
		radiotr->rdstatus = RDNOSLOT;
		return NOFRAME;
	}
	
	if (radiotr->rdcount == 0) {		/* No packet is waiting	*/
		radiotr->rdstate = RADIO_RECV;
		radiotr->rdpid = currpid;
		msg = recvclr();				/* remember: No Packet yet */
		if (timeout)	{
//			kprintf("recvOr..\n");
			msg = recvOrTimeout(timeout);
		}
		else	{
//			kprintf("receive...\n");
			msg = receive();
		}
		if (msg == TIMEOUT) {
//			kprintf("recv: TIMEOUT\n");
			restore(ps);
			radiotr->rdstatus = RDTIMEOUT;
			return NOFRAME;
		}
		if (msg != OK) {
//			kprintf("recv: NOT OK\n");
			restore(ps);
			radiotr->rdstatus = RDBAD;
			return NOFRAME;
		}
	}
	
	/* Packet has arrived -- dequeue it */
	
	pkt = radiotr->rdqueue[radiotr->rdhead++];
	if (radiotr->rdhead >= RADIO_QSIZ) {
		radiotr->rdhead = 0;
	}
	radiotr->rdcount--;
	radiotr->rdstatus = RDSUCCESS;
	restore(ps);
	return (pkt);
}

/**
 *------------------------------------------------------------------------
 * radio_read_frame - read RADIO frame.
 *------------------------------------------------------------------------
 */
frame802154_t *
radio_read_frame(int slot)
{
	STATWORD ps;
	struct radioentry *radiotr;	/* pointer to radiotab entry	*/
	int	msg;			/* message from recvtime()	*/
	frame802154_t *pkt;		/* ptr to packet being read	*/
	
	disable(ps);
	radiotr = &radiotab[slot];
	
	if (radiotr->rdcount == 0) {		/* No packet is waiting	*/
		radiotr->rdstate = RADIO_RECV;
		radiotr->rdpid = currpid;
		msg = recvclr();				/* remember: No Packet yet */
		if (radiotr->rdtimeout)	{
			kprintf("recvOr..\n");
			msg = recvOrTimeout(RDTIMEOUT_TIME);
		}
		else	{
			kprintf("receive...\n");
			msg = receive();
		}
		if (msg == TIMEOUT) {
			kprintf("recv: TIMEOUT\n");
			radiotr->rdstatus = RDTIMEOUT;
			restore(ps);
			return NOFRAME;
		}
		if (msg != OK) {
			kprintf("recv: NOT OK\n");
			radiotr->rdstatus = RDBAD;
			restore(ps);
			return NOFRAME;
		}
	}
	
	/* Packet has arrived -- dequeue it */
	
	pkt = radiotr->rdqueue[radiotr->rdhead++];
	if (radiotr->rdhead >= RADIO_QSIZ) {
		radiotr->rdhead = 0;
	}
	radiotr->rdcount--;
	radiotr->rdstatus = RDSUCCESS;
	restore(ps);
	return (pkt);
}


/**
 *------------------------------------------------------------------------
 * radio_recv_data - receive RADIO data.
 *------------------------------------------------------------------------
 */
int
radio_recv_data	(
				ShortAddr_t dest_addr,	// destination address address or 0xffff
				PanId_t dest_panID,		// destination RADIO protocol panID
				PanId_t src_panID,		// source RADIO protocol panID
				octet_t *buff,			// buffer to hold RADIO data
				uint8_t len,			// length of buffer
				uint32_t timeout		// read timeout in Symbol Times
				)
{
	uint8_t	i;				/* index into radiotab		*/
	frame802154_t *pkt;		/* ptr to packet being read	*/
	uint8_t	msglen;			/* length of RADIO data in packet	*/
	octet_t *rdataptr;		/* pointer to RADIO data		*/
		
	pkt = radio_recv_frame(dest_addr, dest_panID, src_panID, timeout);
	if (pkt == NOFRAME)		{
		/* have to find slot to get error code */
		i = radio_getSlot(dest_addr, dest_panID, src_panID);
		if (i == SYSERR)
			return RDBAD;
		return (&radiotab[i])->rdstatus;
	}
	
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
	return OK;
}

/**
 *------------------------------------------------------------------------
 * radio_clear - clear previously received RADIO packets
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
 * radio_send_data - create frame and send RADIO packet data.
 *------------------------------------------------------------------------
 */
int radio_send_data (
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
	octet_t	*rdataptr;			/* pointer to RADIO data	*/
	int ret;
	
	/* Create RADIO packet in pkt */
	pkt = (frame802154_t *)getbuf(Net.radiopool);
	radio_createDataHdr(pkt, dest_addr, dest_panID, src_addr, src_panID);
	pkt->data_len = len;
	rdataptr = pkt->data;
	for (; len>0; len--) {
		*rdataptr++ = *buff++;
	}
	
	/* Compute packet length as RADIO data size + header size + footer size	*/
	
	pktlen = pkt->data_len + pkt->header_len + FTR_LEN;
//	frameHeaderDump("radio_send", pkt, pktlen);
	ret = write(RADIO, (unsigned char *)pkt, pktlen);
	freebuf((int *)pkt);
	if (ret == SYSERR) return SYSERR;
	return OK;
}

/**
 *-------------------------------------------------------------------------
 * radio_send_frame - send a RADIO frame
 *-------------------------------------------------------------------------
 */
int
radio_send_frame(
				ShortAddr_t dest_addr,	// destination address address
				PanId_t dest_panID,		// destination RADIO protocol panID
				ShortAddr_t src_addr,	// source address
				PanId_t src_panID,		// source RADIO protocol panID
				frame802154_t *frame	// radio frame to send
				)
{
	uint8_t	framelen;			/* total frame length		*/
	
	/* Compute packet length as RADIO data size + header size + footer size	*/
	
	framelen = frame->data_len + frame->header_len + FTR_LEN;
	//	frameHeaderDump("radio_send", &frame, framelen);
	write(RADIO, (unsigned char *)frame, framelen);
	freebuf((int *)frame);
	return OK;
}

/**
 *------------------------------------------------------------------------
 * radio_release - release a previously-registered destination address, destination
 *			panID, and source panID (exact match required)
 *------------------------------------------------------------------------
 */
int
radio_release(
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
