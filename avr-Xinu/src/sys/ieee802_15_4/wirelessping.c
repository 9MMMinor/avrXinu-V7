//
//  wirelessping.c
//  802_15_4_Mib
//
//  Created by Michael Minor on 3/9/14.
//  Copyright (c) 2014.
//
//	This program is free software: you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//

#include <avr-Xinu.h>
#include <avr/io.h>
#include "frameIO.h"

extern struct radioinfo Radio;
extern Bool	macPromiscuousMode;

/*
 *--------------------------------------------------------------------------
 *  ping  -  This ping utility sends an ieee 802.14.5 packet to elicit an
 *  ACK from a local responder.  The packet may optionally request a reply
 *	ping. Pings have a PING header, followed by a payload which consists of
 *  a Flag byte and up to 8 bytes of data. A ping is sent every TIME seconds.
 *--------------------------------------------------------------------------
 */

#include "wirelessping.h"

int
main(void)
{
	frame802154_t *frame;
	pingPacket_t *ping;
	int loop = 0;

#ifdef MASTER
	frame802154_t *reply;
	uint8_t seq = 0;
	int echo_reply = 0;
	
	/* make a ping packet */
	ping = (pingPacket_t *)getmem(sizeof(pingPacket_t));
	ping->hdr = PINGAPP;
	ping->pcf.replyRequested = 1;
	ping->pcf.includeTimeVal = 1;
	ping->pcf.res = 0;
	memset(ping->data, 0, 8);
	
	while (TRUE) {
		
		loop++;
//		printf("Ping %d\n", loop);
		frame = (frame802154_t *)getbuf(Radio.radiopool);
		/* fill every field! */
		frame->fcf.frameType = FRAME_TYPE_DATA;
		frame->fcf.frameSecurity = 0;
		frame->fcf.frameAckRequested = 1;
		frame->fcf.framePending = 0;
		frame->fcf.frameVersion = FRAME_VERSION_2006;
		frame->fcf.frameDestinationAddressMode = FRAME_ADDRESS_MODE_SHORT;
		frame->fcf.framePanIDCompress = 0;
		frame->fcf.frameSourceAddressMode = FRAME_ADDRESS_MODE_SHORT;
		frame->seq = seq++;
		frame->dest_pid = 0xab;
		frame->dest_addr[0] = 0xbe;
		frame->dest_addr[1] = 0xba;
		frame->src_addr[0] = 0xa0;
		frame->src_addr[1] = 0xa1;
		frame->src_pid = 0x1234;
		frame->header_len = getFrameHdrLength(frame);
		if (ping->pcf.includeTimeVal == 1)	{
			memcpy(ping->data, (char *)&SCCNTLL, 4); /* get the symbol counter register */
		}
		
		memcpy(frame->data, ping, (frame->data_len = sizeof(pingPacket_t)));
		psend(Radio.foport, (int)frame);		/* writes & freebufs frame */
		
		if (ping->pcf.replyRequested)	{
			reply = (frame802154_t *)preceive(Radio.fiport);
			freebuf((int *)reply);
			echo_reply++;
		}
		
		if (ping->pcf.replyRequested && loop%10 == 0)	{
			printf("pings sent %d, echoed pings received %d\n", loop, echo_reply);
		}
		sleep(TIME);
	}
#endif
	
#ifdef SLAVE
	uint16_t temp_pid;
	uint8_t temp_addr[8];
	
	kprintf("Slave is listening\n");
//	macPromiscuousMode = TRUE;
	while (TRUE) {

		printf("Got ping %d\n", loop++);
		frame = (frame802154_t *)preceive(Radio.fiport);	/* blocks */
		ping = (pingPacket_t *)frame->data;
		if (ping->pcf.replyRequested == 1)	{
			sleep10(5);							/* slow ping response */
			temp_pid = frame->dest_pid;
			frame->dest_pid = frame->src_pid;
			frame->src_pid = temp_pid;
			memcpy(temp_addr, frame->dest_addr, 8);
			memcpy(frame->dest_addr, frame->src_addr, 8);
			memcpy(frame->src_addr, temp_addr, 8);
//			frameHeaderDump("Slave Echo", frame, 40);
			psend(Radio.foport, (int)frame);		/* ECHO: writes & freebufs frame */
		}
		else
			freebuf((int *)frame);
		
	}
#endif

	return(OK);
}
