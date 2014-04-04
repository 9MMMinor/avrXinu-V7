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
#include "macSymbolCounter.h"
#include <cmd.h>
#include <shell.h>
#include <frame802154.h>
#include <mib.h>

#include <stdlib.h>

#define PINGAPP 0xab
#define PING_TIMEOUT 200000UL		/* Symbol Times */
extern struct radioinfo Radio;
int ping(int, char**);

/*
 *------------------------------------------------------------------------
 *  x_ping  -  report ping data
 *------------------------------------------------------------------------
 */

BUILTIN	x_ping(int stdIN, int stdOUT, int stdERR, int nargs, char *args[])
{
	
	if (ping(nargs,args) == SYSERR)
		return (0);
	return (OK);
}


struct ping_data	{
	Bool registered;
	uint64_t dest_addr;		/* destination address address */
	uint16_t dest_panID;	/* destination RADIO protocol panID	*/
	uint64_t src_addr;		/* source address address		*/
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

int
ping(int nargs, char *args[])
{
	ExtAddr_t dst;
	int	got, i;
	int	np;		// # packets to send
	
	int retval;
	np = 10;
	got = i = 0;
	uint32_t delta;
	
	if (nargs == 3) {
		np = atoi(args[2]);
	} else if (nargs != 2) {
		printf("usage: ping dest_addr [no. of pings]\n");
		return OK;
	}
	dst = (ExtAddr_t)atoi(args[1]);
	
	Ping_data.dest_panID = 0xab;
	Ping_data.dest_addr  = dst;
	Ping_data.src_addr   = 0xa1a0;
	Ping_data.src_panID  = 0x1234;
	
	pingPkt.hdr = PINGAPP;
	pingPkt.pcf.replyRequested = 1;
	pingPkt.pcf.includeTimeVal = 1;
	pingPkt.pcf.res = 0;
	pingPkt.time = 0;
	
	/* For the first time after reboot, register through RadioIO */
	if ( ! Ping_data.registered ) {
		retval = radio_register(0,	Ping_data.dest_panID,
									Ping_data.src_panID);
		Ping_data.registered = TRUE;
		Ping_data.seq = 0;
	}
	
	macBSN = Ping_data.seq++;
	delta = 0;
	
	while (TRUE) {
		/* Send a copy of the message */
		
		if (pingPkt.pcf.includeTimeVal == 1)	{
			pingPkt.time = macSymbolCounterRead();
		}

		retval = radio_send(Ping_data.dest_addr,
							Ping_data.dest_panID,
							macLongAddr,
							Ping_data.src_panID,
							(octet_t *)&pingPkt,
							sizeof(pingPkt));
		if (retval == SYSERR) {
			kprintf("Cannot send ping\n");
			return SYSERR;
		}
		
		/* Receive a reply */

		retval = radio_recv(0,	Ping_data.dest_panID,
								Ping_data.src_panID,
								(octet_t *)&reply,
								sizeof(pingPkt),
								PING_TIMEOUT);
		if (retval == TIMEOUT) {
			continue;
		} else if (retval == SYSERR) {
			kprintf("Error reading remote file reply\n");
			return SYSERR;
		} else {
			if (i >= np)
				break;
			i++;
		}
		
		delta += macSymbolCounterRead() - reply.time;
		
		got++;
		sleep(1);
		printf(".");
	}
	delta = delta/got;
	printf("\nreceived %d/%d packets (%d %% loss) Average delta = %ld\n", got, i,
			(100 * (i-got))/i, delta);
	
	return(OK);
}
