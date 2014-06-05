//
//  frame802154.c
//  802_15_4_Mib
//
//  Created by Michael M Minor on 12/4/13.
//
//

#include <avr-Xinu.h>
#include <avr/io.h>
#include <string.h>
#include "frame802154.h"
#include "mib.h"

static int printable(char);

#define NOT_YET (frame802154_t *)0
#define BAD_FRAME_CREATE (frame802154_t *)0

/**
 *	frame_create -- allocate a frame and initialize the radio header.
 *
 *	This is simply not very useful!
 */
frame802154_t *
frame802154_create(uint8_t type)
{
	frame802154_t *p;
	
	switch (type)	{
		case FRAME_TYPE_BEACON:
			return (NOT_YET);
		case FRAME_TYPE_DATA:
			p = (frame802154_t *)malloc(sizeof(frame802154_t));
			radio_createDataHdr(p, 0xffff, 0xffff, macShortAddress, macPANId);
			return (p);
		case FRAME_TYPE_ACK:
			return (NOT_YET);
		case FRAME_TYPE_MAC_COMMAND:
			makeMACCommandHdr(p);
			return (NOT_YET);
		default:
			break;
	}
	return (BAD_FRAME_CREATE);
}

/**
 *------------------------------------------------------------------------
 *	radio_createDataHdr -- create a FRAME_TYPE_DATA packet header
 *	return pointer to payload
 *------------------------------------------------------------------------
 */
octet_t	*
radio_createDataHdr (
				frame802154_t *pkt,			/* source packet buffer */
				ShortAddr_t dest_addr,		/* destination address address or address_BCAST*/
				PanId_t dest_panID,			/* destination RADIO protocol panID	*/
				ShortAddr_t src_addr,		/* source address		*/
				PanId_t src_panID			/* source RADIO protocol panID	*/
)
{
		
	/* Create Header in pkt */
	/* fill every field! */
	pkt->fcf.frameType = FRAME_TYPE_DATA;
	pkt->fcf.frameSecurity = 0;
	pkt->fcf.frameAckRequested = 0;			/* set in driver, if requested */
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
	
	return ( pkt->data );
}

/*------------------------------------------------------------------------
 * radio_send_data - send a radio packet
 *------------------------------------------------------------------------
 */
//radio_status_t radio_send_data (
//				  uint8_t *buff,			/* buffer of user data		*/
//				  uint8_t len				/* length of data in buffer	*/
//)
//{
//	int pktlen;
//	frame802154_t *p = frame802154_create(FRAME_TYPE_DATA);
	
//	memcpy(p->data, buff, (p->data_len = len));
//	pktlen = p->header_len + p->data_len + FTR_LEN;
	
//	write(RADIO, (unsigned char *)p, pktlen);
//	free(p);
//	return OK;
//}

frame802154_t *
makeMACCommandHdr(frame802154_t *p)
{
	return p;
}


/**
 *	packFrame() -- pack a frameBuffer from the prototype in p
 *
 */

void
packTXFrame(frame802154_t *p)
{
	octet_t *q = (octet_t *)&TRXFBST;
	frameControlField_t fcf = p->fcf;
	
	*q++ = p->header_len + p->data_len + FTR_LEN;	/* length of TX buffer */
	q = copyOctets(q, HEADERVARPTR(p->fcf), 2);		/* fcf */
	q = copyOctets(q, HEADERVARPTR(p->seq), 1);		/* seq */
	if (fcf.frameDestinationAddressMode != FRAME_ADDRESS_MODE_NONE)	{
		q = copyOctets(q, HEADERVARPTR(p->dest_pid), 2);
	}
	switch (fcf.frameDestinationAddressMode)	{
		case FRAME_ADDRESS_MODE_SHORT:
			q = copyOctets(q, HEADERVARPTR(p->dest_addr.caddr[0]), 2);
			break;
		case FRAME_ADDRESS_MODE_EXTENDED:
			q = copyOctets(q, HEADERVARPTR(p->dest_addr.caddr[0]), 8);
			break;
		case FRAME_ADDRESS_MODE_NONE:
		default:
			break;
	}
	if (fcf.framePanIDCompress == 0) {
		q = copyOctets(q, HEADERVARPTR(p->src_pid), 2);
	}
	switch (fcf.frameSourceAddressMode)	{
		case FRAME_ADDRESS_MODE_SHORT:
			q = copyOctets(q, HEADERVARPTR(p->src_addr.caddr[0]), 2);
			break;
		case FRAME_ADDRESS_MODE_EXTENDED:
			q = copyOctets(q, HEADERVARPTR(p->src_addr.caddr[0]), 8);
			break;
		case FRAME_ADDRESS_MODE_NONE:
		default:
			break;
	}
	/* Aux security header */
	if	(fcf.frameSecurity == 1) {
		securityControlField_t scf = p->aux_hdr.scf;
		q = copyOctets(q, HEADERVARPTR(scf), 1);
		q = copyOctets(q, HEADERVARPTR(p->aux_hdr.frameCounter), 4);
		switch (scf.keyIdMode) {
			case 1:
				q = copyOctets(q, p->aux_hdr.key, 1);
				break;
			case 2:
				q = copyOctets(q, p->aux_hdr.key, 5);
				break;
			case 3:
				q = copyOctets(q, p->aux_hdr.key, 9);
				break;
			case 0:
			default:
				break;
		}
	}
	blkcopy(q, p->data, p->data_len);				/* copy the payload */
}

/**
 *	copyOctets() -- copy nbytes from to
 *
 *	return pointer to next "to"
 */

octet_t *
copyOctets(octet_t *to, octet_t *from, int nbytes)
{
	while (--nbytes >= 0)
		*to++ = *from++;
	return (to);
}

/**
 *	copyRXOctets() -- copy nbytes from to
 *
 *	return pointer to next "from"
 */

octet_t *
copyRXOctets(octet_t *to, octet_t *from, int nbytes)
{
	while (--nbytes >= 0)
		*to++ = *from++;
	return (from);
}

/**
 *	unpackRXFrame() -- unpack the RX buffer into the fixed length frame802154_t *p
 *
 *	stores receive buffer info into the prototype add header_len and data_len
 */

void
unpackRXFrame(frame802154_t *p)
{
	octet_t *q = (octet_t *)&TRXFBST;
	
	q = copyRXOctets(HEADERVARPTR(p->fcf), q, 2);		/* fcf */
	q = copyRXOctets(HEADERVARPTR(p->seq), q, 1);		/* seq */
	if (p->fcf.frameDestinationAddressMode != FRAME_ADDRESS_MODE_NONE)	{
		q = copyRXOctets(HEADERVARPTR(p->dest_pid), q, 2);
	}
	switch (p->fcf.frameDestinationAddressMode)	{
		case FRAME_ADDRESS_MODE_SHORT:
			q = copyRXOctets(HEADERVARPTR(p->dest_addr.caddr[0]), q, 2);
			memset(HEADERVARPTR(p->dest_addr.caddr[2]), 0, 6);
			break;
		case FRAME_ADDRESS_MODE_EXTENDED:
			q = copyRXOctets(HEADERVARPTR(p->dest_addr.caddr[0]), q, 8);
			break;
		case FRAME_ADDRESS_MODE_NONE:
		default:
			break;
	}
	if (p->fcf.framePanIDCompress == 0) {
		q = copyRXOctets(HEADERVARPTR(p->src_pid), q, 2);
	}
	switch (p->fcf.frameSourceAddressMode)	{
		case FRAME_ADDRESS_MODE_SHORT:
			q = copyRXOctets(HEADERVARPTR(p->src_addr.caddr[0]), q, 2);
			memset(HEADERVARPTR(p->src_addr.caddr[2]), 0, 6);
			break;
		case FRAME_ADDRESS_MODE_EXTENDED:
			q = copyRXOctets(HEADERVARPTR(p->src_addr.caddr[0]), q, 8);
			break;
		case FRAME_ADDRESS_MODE_NONE:
		default:
			break;
	}
	/* Aux security header */
	if	(p->fcf.frameSecurity == 1) {
//		securityControlField_t scf = p->aux_hdr.scf;
		q = copyRXOctets(HEADERVARPTR(p->aux_hdr.scf), q, 1);
		q = copyRXOctets(HEADERVARPTR(p->aux_hdr.frameCounter), q, 4);
		switch (p->aux_hdr.scf.keyIdMode) {
			case 1:
				q = copyRXOctets(p->aux_hdr.key, q, 1);
				break;
			case 2:
				q = copyRXOctets(p->aux_hdr.key, q, 5);
				break;
			case 3:
				q = copyRXOctets(p->aux_hdr.key, q, 9);
				break;
			case 0:
			default:
				break;
		}
	}
	p->header_len = q - &TRXFBST;							/* header length	*/
	p->data_len = TST_RX_LENGTH - p->header_len - FTR_LEN;	/* data length		*/
	q = copyRXOctets(p->data, q, p->data_len);
	uint8_t hibyte = *q++;
	uint8_t lobyte = *q;
	p->crc = (hibyte<<8) | lobyte;
}

uint8_t
getFrameHdrLength(frame802154_t *p)
{
	uint8_t ret;
	
//	if (p->header_len)	{
//		return ( p->header_len );
//	}
	
	ret = 3;		/* fcf */
					/* seq */
	if (p->fcf.frameDestinationAddressMode != FRAME_ADDRESS_MODE_NONE)	{
		ret += 2;
	}
	switch (p->fcf.frameDestinationAddressMode)	{
		case FRAME_ADDRESS_MODE_SHORT:
			ret += 2;
			break;
		case FRAME_ADDRESS_MODE_EXTENDED:
			ret += 8;
			break;
		case FRAME_ADDRESS_MODE_NONE:
		default:
			break;
	}
	if(p->fcf.framePanIDCompress == 0) {
		ret += 2;
	}
	switch (p->fcf.frameSourceAddressMode)	{
		case FRAME_ADDRESS_MODE_SHORT:
			ret += 2;
			break;
		case FRAME_ADDRESS_MODE_EXTENDED:
			ret += 8;
			break;
		case FRAME_ADDRESS_MODE_NONE:
		default:
			break;
	}
	/* Aux security header */
	if	(p->fcf.frameSecurity == 1) {
		//		securityControlField_t scf = p->aux_hdr.scf;
		ret += 5;
		switch (p->aux_hdr.scf.keyIdMode) {
			case 1:
				ret += 1;
				break;
			case 2:
				ret += 5;
				break;
			case 3:
				ret += 9;
				break;
			case 0:
			default:
				break;
		}
	}
	p->header_len = ret;
	return (ret);
}


/**
 *	frameRAWHeaderDump() -- dump the header from the RAW RX frame with pointer, "fptr."
 *
 *	"routine" is a string identifying the dump.
 */

void frameRAWHeaderDump(char *routine, octet_t *fptr, int len)
{
	frameControlField_t fcf = ((frame802154_t *)fptr)->fcf;
	octet_t *p = fptr;
	int i;
	
	kprintf("frame_Dump (%s):", routine);
//	kprintf(" packet length = %d", fptr->header_len+fptr->data_len+2);
	
	kprintf(" FCF = <%02x%02x> ", *(p+1), *p);
	p += 2;
	kprintf("Seq = %02x\n", *p++);
	if (fcf.frameDestinationAddressMode)	{
		kprintf("Destination PAN Id = <%02x%02x> ", *(p+1), *p);
		p += 2;
	}
	switch (fcf.frameDestinationAddressMode)	{
		case FRAME_ADDRESS_MODE_SHORT:
			kprintf("Short Address = <%02x%02x>\n", *(p+1), *p);
			p += 2;
			break;
		case FRAME_ADDRESS_MODE_EXTENDED:
			kprintf("Extended Address = <%02x%02x%02x%02x%02x%02x%02x%02x>\n", *(p+7), *(p+6), *(p+5), *(p+4),
					*(p+3), *(p+2), *(p+1), *(p+0));
			p += 8;
			break;
		case FRAME_ADDRESS_MODE_NONE:
		default:
			kprintf("\n");
			break;
	}
	if (!fcf.framePanIDCompress) {
		kprintf("Source PAN Id = <%02x%02x> ", *(p+1), *p);
		p += 2;
	} else {
		kprintf("Source ");
	}
	switch (fcf.frameSourceAddressMode)	{
		case FRAME_ADDRESS_MODE_SHORT:
			kprintf("Short Address = <%02x%02x>\n", *(p+1), *(p+0));
			p += 2;
			break;
		case FRAME_ADDRESS_MODE_EXTENDED:
			kprintf("Extended Address = <%02x%02x%02x%02x%02x%02x%02x%02x>\n",  *(p+7), *(p+6), *(p+5), *(p+4),
					*(p+3), *(p+2), *(p+1), *(p+0));
			p += 8;
			break;
		case FRAME_ADDRESS_MODE_NONE:
		default:
			kprintf("\n");
			break;
	}
	/* Aux security header */
	if	(fcf.frameSecurity) {
		securityControlField_t scf = ((frame802154_t *)fptr)->aux_hdr.scf;
		kprintf("Security Control Field = <%02x> ", *p++);
		kprintf("Frame Counter = <%02x%02x%02x%02x>\n", *(p+3), *(p+2), *(p+1), *(p+0));
		p += 4;
		kprintf("Keys = < ");
		switch (scf.keyIdMode) {
			case 1:
				kprintf("%02x ", *p++);
				break;
			case 2:
				for (i=0 ; i<5 ; i++)
					kprintf("%02x ", *p++);
				break;
			case 3:
				for (i=0 ; i<9 ; i++)
					kprintf("%02x ", *p++);
				break;
			case 0:
			default:
				break;
		}
		kprintf(">\n");
	}
//	frameDump("data", fptr->data, fptr->data_len);
//	kprintf("CRC = <%04x>\n", fptr->crc);
}

/**
 *	frameHeaderDump() -- dump the header from the frame802154_t frame with pointer, "fptr."
 *
 *	"routine" is a string identifying the dump.
 */

void frameHeaderDump(char *routine, frame802154_t *fptr, int len)
{
	frameControlField_t fcf = fptr->fcf;
	octet_t *p;
	int i;
	
	kprintf("frame_Dump (%s):", routine);
	kprintf(" packet length = %d", fptr->header_len+fptr->data_len+2);
	
	kprintf(" FCF = <%04x> ", fptr->fcf);
	kprintf("Seq = %02x\n", fptr->seq);
	if (fcf.frameDestinationAddressMode)	{
		kprintf("Destination PAN Id = <%04x> ", fptr->dest_pid);
	}
	switch (fcf.frameDestinationAddressMode)	{
		case FRAME_ADDRESS_MODE_SHORT:
			kprintf("Short Address = <%04x>\n", fptr->dest_addr.saddr);
			break;
		case FRAME_ADDRESS_MODE_EXTENDED:
			p = &fptr->dest_addr.eaddr;
			kprintf("Extended Address = <%02x%02x%02x%02x%02x%02x%02x%02x>\n", *(p+7), *(p+6), *(p+5), *(p+4),
					*(p+3), *(p+2), *(p+1), *(p+0));
			break;
		case FRAME_ADDRESS_MODE_NONE:
		default:
			kprintf("\n");
			break;
	}
	if (!fcf.framePanIDCompress) {
		kprintf("Source PAN Id = <%04x> ", fptr->src_pid);
	} else {
		kprintf("Source ");
	}
	switch (fcf.frameSourceAddressMode)	{
		case FRAME_ADDRESS_MODE_SHORT:
			kprintf("Short Address = <%04x>\n", fptr->src_addr.saddr);
			break;
		case FRAME_ADDRESS_MODE_EXTENDED:
			p = &(fptr->src_addr.eaddr);
			kprintf("Extended Address = <%02x%02x%02x%02x%02x%02x%02x%02x>\n",  *(p+7), *(p+6), *(p+5), *(p+4),
					*(p+3), *(p+2), *(p+1), *(p+0));
			break;
		case FRAME_ADDRESS_MODE_NONE:
		default:
			kprintf("\n");
			break;
	}
	/* Aux security header */
	if	(fcf.frameSecurity) {
		securityControlField_t scf = fptr->aux_hdr.scf;
		kprintf("Security Control Field = <%02x> ", scf);
		kprintf("Frame Counter = <%04lx>\n", fptr->aux_hdr.frameCounter);
		kprintf("Keys = < ");
		p = fptr->aux_hdr.key;
		switch (scf.keyIdMode) {
			case 1:
				kprintf("%02x ", *p++);
				break;
			case 2:
				for (i=0 ; i<5 ; i++)
					kprintf("%02x ", *p++);
				break;
			case 3:
				for (i=0 ; i<9 ; i++)
					kprintf("%02x ", *p++);
				break;
			case 0:
			default:
				break;
		}
		kprintf(">\n");
	}
	frameDump("data", fptr->data, fptr->data_len);
	kprintf("CRC = <%04x>\n", fptr->crc);
}

void frameDump(char *routine, uint8_t *p, int len)
{
	uint8_t ch[16];
	int i,j;
	
	kprintf("%s:\n", routine);
	for (i=0 ; i<len ; i+=16)	{
		kprintf("%04X ", i);
		for (j=0; j<16; j++)	{
			ch[j] = ' ';
			if (i+j < len)	{
				ch[j] = *p++;
				kprintf("%02X ", ch[j]);
			}
			else
				kprintf("   ");
		}
		kprintf("\t/");
		for (j=0; j<16; j++)	{
			if ( printable(ch[j]) )
				kprintf("%c",ch[j]);
			else
				kprintf(".");
		}
		kprintf("/\n");
	}
}

static int printable(char ch)
{
	return( (ch >= ' ') && (ch <= '~') );
}
