//
//  x_ping.c
//  802_15_4_Mib
//
//  Created by Michael Minor on 3/28/14.
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
#include "radioIO.h"
#include "radio.h"
#include "macSymbolCounter.h"
#include <cmd.h>
#include <shell.h>
#include <frame802154.h>
#include <mib.h>

#include <stdlib.h>

#define PINGAPP 0xab
#define PING_TIMEOUT 400UL		/* Symbol Times */
extern struct radioinfo Radio;

/*
 *------------------------------------------------------------------------
 *  x_ping  -  report ping data
 *------------------------------------------------------------------------
 */

struct ping_data	{
	Bool registered;
	radioAddr_t dest_addr;		/* destination address */
	uint16_t dest_panID;		/* destination RADIO protocol panID	*/
	radioAddr_t src_addr;		/* source address address		*/
	uint16_t src_panID;
	uint8_t  seq;
} Ping_data;

struct pingControlField	{
	uint8_t replyRequested:1;
	uint8_t includeTimeVal:1;
	uint8_t res:6;
};

struct pingPacket	{
	uint8_t hdr;
	struct pingControlField pcf;
	uint32_t time;
} pingPkt, reply;


/*------------------------------------------------------------------------
 *  ping  -  do echo to a given destination
 *------------------------------------------------------------------------
 */

COMMAND
x_ping(int nargs, int *argv)
{

	int	acked, drop, dump, got, i;
	int	np;		// # packets to send
	int retval;
	uint32_t delta, sendTime;
	Bool noechoflag = FALSE;
	int offset = 0;
	
	np = 10;
	acked = got = drop = 0;
	
	if (nargs > 1 && strncmp((char *)argv[1],"-noecho",3) == 0) {
		printf("%s\n",(char *)argv[1]);
		nargs--;
		offset = 1;
		noechoflag = TRUE;
	}

	if (nargs == 3) {
		np = atoi((char *)argv[offset+2]);
	}
	else if (nargs != 2) {
		printf("usage: ping [-noecho] dest_addr [no. of pings]\n");
		return OK;
	}
		
	Ping_data.dest_panID = 0xffff;
	Ping_data.dest_addr.saddr  = atoi((char *)argv[offset+1]);
	Ping_data.src_addr.saddr   = macShortAddress;
	Ping_data.src_panID  = macPANId;
	Ping_data.registered = FALSE;
	
	pingPkt.hdr = PINGAPP;
	if (noechoflag)	{
		printf("Ping with Ack request\n");
		pingPkt.pcf.replyRequested = 0;
		radio[0].doRequestACK = TRUE;
	} else	{
		printf("Ping with Echo request\n");
		pingPkt.pcf.replyRequested = 1;
		radio[0].doRequestACK = FALSE;
//		radio_set_short_address(macShortAddress);
//		radio_set_pan_id(macPANId);
	}
	pingPkt.pcf.includeTimeVal = 1;
	pingPkt.pcf.res = 0;
	pingPkt.time = 0;
	
	/* For the first time after reboot, register through RadioIO */
	if ( ! Ping_data.registered ) {
		retval = radio_register(BROADCAST_ADDR,	Ping_data.dest_panID,
									Ping_data.src_panID);
		Ping_data.registered = TRUE;
		Ping_data.seq = 0;
	}
	
	macBSN = Ping_data.seq++;
	delta = 0;
	sendTime = 0;
	
	printf("ping: dest addr=<%04x> dest PAN-Id=<%04x> src addr=<%04x>\n",
		   Ping_data.dest_addr.saddr,
		   Ping_data.dest_panID,
		   Ping_data.src_addr.saddr);
	
	for (i = 0; i < np; i++, sleep(1), printf(".")) {
		/* Send a copy of the message */
		
		if (pingPkt.pcf.includeTimeVal == 1)	{
			sendTime = pingPkt.time = macSymbolCounterRead();
		}

		dump = radio_clear(BROADCAST_ADDR, Ping_data.dest_panID, Ping_data.src_panID);
		if (dump > 0)	{
			printf("ping error: dump %d packets\n", dump);
		}
		/* radio_send_data returns OK or SYSERR */
		/* if SYSERR see radio[0].errorCode		*/
		retval = radio_send_data(Ping_data.dest_addr.saddr,
									Ping_data.dest_panID,
									Ping_data.src_addr.saddr,
									Ping_data.src_panID,
									(octet_t *)&pingPkt,
									sizeof(pingPkt));
		if (retval == SYSERR) {
			if (radio[0].errorCode == RADIO_NO_ACK)	{
				drop++;
				continue;
			}
			else	{
				printf("Cannot send ping\n");
				return SYSERR;
			}
		}
		
		/* Receive a reply */
		if (noechoflag)	{
			acked++;
			delta += macSymbolCounterRead() - sendTime;
		} else	{
			retval = radio_recv_data(BROADCAST_ADDR,
								Ping_data.dest_panID,
								Ping_data.src_panID,
								(octet_t *)&reply,
								sizeof(pingPkt),
								10*PING_TIMEOUT);
//			printf("retval=%d\n",retval);
			if (retval == RDTIMEOUT) {
				drop++;
			}
			else if (retval == SYSERR) {
				printf("Error reading remote file reply\n");
				return SYSERR;
			}
			else	{
				delta += macSymbolCounterRead() - reply.time;
				got++;
			}
		}
	}
	if (noechoflag)	{
		if (acked > 0) delta = delta/acked;
		printf("\npings %d Acked %d packets (%d %% loss) Average delta = %ld\n", i, acked,
			   (100 * (i-acked))/i, delta);
	} else	{
		if (got > 0) delta = delta/got;
		printf("\npings %d echo %d packets (%d %% loss) Average delta = %ld\n", i, got,
			   (100 * (i-got))/i, delta);
	}
	radio_release(0xffff, Ping_data.dest_panID, Ping_data.src_panID);
	Ping_data.registered = FALSE;
	return(OK);
}
