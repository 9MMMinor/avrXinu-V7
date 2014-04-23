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
#define PING_TIMEOUT 200UL		/* Symbol Times */
extern struct radioinfo Radio;

/*
 *------------------------------------------------------------------------
 *  x_ping  -  report ping data
 *------------------------------------------------------------------------
 */

//BUILTIN	x_ping(int stdIN, int stdOUT, int stdERR, int nargs, char *args[])
//{
	
//	if (ping(nargs,args) == SYSERR)
//		return (0);
//	return (OK);
//}


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

COMMAND
x_ping(int nargs, int *argv)
{
	int	drop, got, i;
	int	np;		// # packets to send
	int retval;
	uint32_t delta, delta2, sendTime;
	
	np = 10;
	got = i = drop = 0;
	
//	if (nargs == 3) {
//		np = atoi(args[2]);
//	} else if (nargs != 2) {
//		printf("usage: ping dest_addr [no. of pings]\n");
//		return OK;
//	}
	
	/*	avr-gcc passes most arguments in registers. Not wanting to get into
	 *  the details, and gcc's strict use of variable length argument passing,
	 *  for avr-Xinu, create() creates new threads with just two arguments:
	 *  newproc(int argc, int *argv).
	 *  But then the shell is bizare and has never been fixed (mmm 4/13/2014):
	 */
	if (nargs == 7) {
		np = atoi((char *)argv[6]);
	}
	else if (nargs != 6) {
		printf("usage: ping dest_addr [no. of pings]\n");
		return OK;
	}
	/*  Yikes!!  */
		
	Ping_data.dest_panID = 0xffff;
	Ping_data.dest_addr  = atoi((char *)argv[5]);
	Ping_data.src_addr   = 	0xbabe;
	Ping_data.src_panID  = 101;
	
	pingPkt.hdr = PINGAPP;
	pingPkt.pcf.replyRequested = 1;
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
	sendTime = delta2 = 0;
	
	for (i = 0; i < np; i++) {
		/* Send a copy of the message */
		
		if (pingPkt.pcf.includeTimeVal == 1)	{
			sendTime = pingPkt.time = macSymbolCounterRead();
		}

//		dump = radio_clear(0, Ping_data.dest_panID, Ping_data.src_panID);
//		if (dump > 0)	{
//			kprintf("ping: dump %d packets\n", dump);
//		}
		retval = radio_send(Ping_data.dest_addr,
							Ping_data.dest_panID,
							Ping_data.src_addr,
							Ping_data.src_panID,
							(octet_t *)&pingPkt,
							sizeof(pingPkt));
		if (retval == SYSERR) {
			printf("Cannot send ping\n");
			return SYSERR;
		}
		
		/* Receive a reply */

		retval = radio_recv(BROADCAST_ADDR,
								Ping_data.dest_panID,
								Ping_data.src_panID,
								(octet_t *)&reply,
								sizeof(pingPkt),
								10*PING_TIMEOUT);
		if (retval == TIMEOUT) {
			drop++;
		}
		else if (retval == SYSERR) {
			printf("Error reading remote file reply\n");
			return SYSERR;
		}
		else	{
			delta += macSymbolCounterRead() - reply.time;
			delta2 += macSymbolCounterRead() - sendTime;
			got++;
//			printf("receive %d\n",got);
		}
		
		
		sleep(1);
		printf(".");
	}
	if (got > 0)		{
		delta2 = delta2/got;
		delta = delta/got;
	}
	printf("\nreceived %d/%d packets (%d %% loss) Average delta = %ld %ld\n", got, i,
			(100 * (i-got))/i, delta, delta2);
	radio_release(0, Ping_data.dest_panID, Ping_data.src_panID);
	Ping_data.registered = FALSE;
	return(OK);
}
