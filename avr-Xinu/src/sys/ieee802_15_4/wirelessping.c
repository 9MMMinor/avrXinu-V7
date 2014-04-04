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
#include <mark.h>
#include <ports.h>
#include <avr/io.h>
#include "mib.h"
#include "frameIO.h"
#include "macSymbolCounter.h"

extern struct radioinfo Radio;
extern Bool	macPromiscuousMode;
extern uint32_t macAckWaitDuration;
extern uint8_t radio_TIMER;
#define RADIO_TIMER &radio_TIMER

int dispose_Messages(int);

/*
 *--------------------------------------------------------------------------
 *  ping  -  This ping utility sends an ieee 802.14.5 packet to elicit an
 *  ACK from a local responder.  The packet may optionally request a reply
 *	ping. Pings have a PING header, followed by a payload which consists of
 *  a Flag byte and up to 8 bytes of data. A ping is sent every TIME seconds.
 *--------------------------------------------------------------------------
 */

#include "wirelessping.h"
INTPROC setTimeOut(void *event);

struct ping_info Pinfo = {0, 0, 0, 0L};
uint8_t timeEvent;

int
wping(void)
{
	frame802154_t *frame;
	pingPacket_t *ping, *pr;
	int loop = 0;
	
	kprintf("Hello World\n");

#ifdef MASTER
	frame802154_t *reply;
	int fmlen;
	uint8_t seq = 0;
	int echo_reply = 0;
	uint32_t now, reply_time;
	uint32_t delta = 0;
	
	/* make a ping packet */
	ping = (pingPacket_t *)getmem(sizeof(pingPacket_t));
	ping->hdr = PINGAPP;
	ping->pcf.replyRequested = 1;
	ping->pcf.includeTimeVal = 1;
	ping->pcf.res = 0;
	memset(ping->data, 0, 8);
	
	frame = (frame802154_t *)getbuf(Radio.radiopool);
	/* fill every field! */
	frame->fcf.frameType = FRAME_TYPE_DATA;
	frame->fcf.frameSecurity = 0;
	frame->fcf.frameAckRequested = 0;
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
	
	
	while (TRUE) {
		
		Pinfo.pingsSent = loop++;
//		printf("Ping %d\n", loop);
		
		if (ping->pcf.includeTimeVal == 1)	{
			now = macSymbolCounterRead();
			memcpy(ping->data, &now, 4); /* get the symbol counter register */
		}
		memcpy(frame->data, ping, (frame->data_len = sizeof(pingPacket_t)));
		fmlen = frame->header_len + frame->data_len + FTR_LEN;
		
		write(RADIO, (unsigned char *)frame, fmlen);
		
		if (ping->pcf.replyRequested)	{
			/* want to receive the reply as soon as it comes ... */
			/* However, it may never come and we will need to cancel */
			timeEvent = Radio.fiport;
			tmset(timerPortID, &timeEvent, 8*macAckWaitDuration, &setTimeOut);	/* see mib.c */
			reply = (frame802154_t *)preceive(Radio.fiport); /* THE message that is replaced by SYSERR */
															 /* is a frame that needs to be freed!!!!! */
			if (reply == (frame802154_t *)SYSERR)	{
				Pinfo.pingsNoEcho++;
			}
			else	{
				tmclear(timerPortID, &timeEvent);			/* cancel TIMEOUT */
				pr = (pingPacket_t *)reply->data;
				reply_time =	(uint32_t)pr->data[0]		|
								(uint32_t)pr->data[1]<<8	|
								(uint32_t)pr->data[2]<<16	|
								(uint32_t)pr->data[3]<<24;
				delta += macSymbolCounterRead() - reply_time;
				if (loop%10 == 0)		{
					Pinfo.delta = delta/10;
					delta = 0;
				}
				freebuf((int *)reply);
				Pinfo.pingsRecvd = echo_reply++;
			}
		}
		
//		if (ping->pcf.replyRequested && loop%10 == 0)	{
//			printf("\n");
//			printf("pings sent %d, echoed pings received %d, ave. delta = %ld\n", loop, echo_reply, delta/10);
//			delta = 0;
//		}
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
			temp_pid = frame->dest_pid;
			frame->dest_pid = frame->src_pid;
			frame->src_pid = temp_pid;
			memcpy(temp_addr, frame->dest_addr, 8);
			memcpy(frame->dest_addr, frame->src_addr, 8);
			memcpy(frame->src_addr, temp_addr, 8);
			/* turn around delay of 12 symbol times */
			pauseSymbolTimes(RADIO_TIMER, 12);	/* standard 12 Symbol Times */
//			frameHeaderDump("Slave Echo", frame, 40);
			psend(Radio.foport, (int)frame);		/* ECHO: writes & freebufs frame */
		}
		else
			freebuf((int *)frame);
		
	}
#endif

	return(OK);
}

/**
 * setTimeOut() -- callback from 66 TIMEOUT interrupt
 */

INTPROC setTimeOut(void *event)
{
	int portID = (*(int *)event);
	int n;
//	struct pt *pptr = &ports[portID];
	
	n = pcount(portID);
	kprintf("setTimeOut: TIMEOUT on port %d; pcount = %d\n", portID, n);
	preset(portID, &dispose_Messages);
	return;
}

int dispose_Messages(int msg)
{
	kprintf("dispose msg = %x\n", msg);
	return (OK);
}
