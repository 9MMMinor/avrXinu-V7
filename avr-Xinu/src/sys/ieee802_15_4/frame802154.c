//
//  frame802154.c
//  802_15_4_Mib
//
//  Created by Michael M Minor on 12/4/13.
//
//

#include <avr-Xinu.h>
#include "frame802154.h"

static int printable(char);

#define NOT_YET (frameReturn_t)SYSERR
#define BAD_FRAME_CREATE (frameReturn_t)SYSERR

frameReturn_t
frame802154_create(frame802154_t *p, frame_t *frame)
{
	uint8_t type = p->fcf.frameType;
	
	switch (type)	{
		case FRAME_TYPE_BEACON:
			return (NOT_YET);
		case FRAME_TYPE_DATA:
			p->payload = makeTXFrameHdr(p, frame->data);
			return ( (frameReturn_t)OK );
		case FRAME_TYPE_ACK:
			return (NOT_YET);
		case FRAME_TYPE_MAC_COMMAND:
			p->payload = makeMACCommandHdr(p, frame->data);
			return ( NOT_YET);
		default:
			break;
	}
	return (BAD_FRAME_CREATE);
}

octet_t *
makeMACCommandHdr(frame802154_t *p, octet_t *frameBuffer)
{
	return frameBuffer;
}

octet_t *
makeTXFrameHdr(frame802154_t *p, octet_t *frameBuffer)
{
	octet_t *q = frameBuffer;
	frameControlField_t fcf = p->fcf;
	
	q = copyOctets(q, HEADERVARPTR(p->fcf), 2);	/* fcf */
	q = copyOctets(q, HEADERVARPTR(p->seq), 1);	/* seq */
	if (fcf.frameDestinationAddressMode)	{
		q = copyOctets(q, HEADERVARPTR(p->dest_pid), 2);
	}
	switch (fcf.frameDestinationAddressMode)	{
		case FRAME_ADDRESS_MODE_SHORT:
			q = copyOctets(q, HEADERVARPTR(p->dest_addr[0]), 2);
			break;
		case FRAME_ADDRESS_MODE_EXTENDED:
			q = copyOctets(q, HEADERVARPTR(p->dest_addr[0]), 8);
			break;
		case FRAME_ADDRESS_MODE_NONE:
		default:
			break;
	}
	if(!fcf.framePanIDCompress) {
		q = copyOctets(q, HEADERVARPTR(p->src_pid), 2);
	}
	switch (fcf.frameSourceAddressMode)	{
		case FRAME_ADDRESS_MODE_SHORT:
			q = copyOctets(q, HEADERVARPTR(p->src_addr[0]), 2);
			break;
		case FRAME_ADDRESS_MODE_EXTENDED:
			q = copyOctets(q, HEADERVARPTR(p->src_addr[0]), 8);
			break;
		case FRAME_ADDRESS_MODE_NONE:
		default:
			break;
	}
	/* Aux security header */
	if	(fcf.frameSecurity) {
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
	
	return (q);		/* where the payload goes */
}

octet_t *
copyOctets(octet_t *to, octet_t *from, int nbytes)
{
	while (--nbytes >= 0)
		*to++ = *from++;
	return (to);
}

void frameHeaderDump(char *routine, frame802154_t *fptr, int len)
{
	frameControlField_t fcf = fptr->fcf;
	octet_t *p = (octet_t *)fptr;
	int i;
	
	kprintf("frame_Dump (%s):",routine);
	
	kprintf("\n FCF = <%02x%02x> ", *(p+1), *p);
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
		securityControlField_t scf = fptr->aux_hdr.scf;
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
}

void frameDump(char *routine, uint8_t *p, int len)
{
	uint8_t ch[16];
	int i,j;
	
	kprintf("DATA:\n");
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
