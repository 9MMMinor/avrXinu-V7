/**
 *  \file x_pingserver.c
 *  \project 802_15_4_Mib
 */

//
//  Created by Michael Minor on 5/8/14.
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

/********************************************************************************//*!
 * \fn int function()
 * \brief Enter function description.
 * \param enter parameter
 * \return enter return
 * \return else return
 ***********************************************************************************/

#include <avr-Xinu.h>
#include "radio.h"
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
extern uint8_t radio_TIMER;
#define RADIO_TIMER &radio_TIMER

/*
 *--------------------------------------------------------------------------
 *  pingServer  -  This ping utility receives an ieee 802.14.5 packet which
 *	may request a reply.  If so, src and dst addresses are swapped and the
 *	packet is sent (back).
 *	Pings have a PING header, followed by a payload which consists of
 *  a Flag byte and up to 8 bytes of data.
 *--------------------------------------------------------------------------
 */

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

typedef struct pingPacket	{
	uint8_t hdr;
	struct pingControlField pcf;
	uint32_t time;
} pingPacket_t;

COMMAND
x_pingServer(int nargs, int *argv)
{
	frame802154_t *frame;
	pingPacket_t *ping;
	int slot = 0;
	int loop = 0;
	
	kprintf("Hello World\n");
	
	uint16_t temp_pid;
	ShortAddr_t temp_addr;
	
	kprintf("Ping daemon is listening\n");
	macPromiscuousMode = TRUE;
	
	Ping_data.dest_panID = 0xffff;
	Ping_data.dest_addr  = 0xffff;
	radio_set_short_address( (Ping_data.src_addr = 99) );
	radio_set_pan_id( (Ping_data.src_panID = 2) );
	
	/* For the first time after reboot, register through RadioIO */
	if ( ! Ping_data.registered ) {
		slot = radio_alloc(BROADCAST_ADDR, Ping_data.dest_panID,
								Ping_data.src_panID);
		Ping_data.registered = TRUE;
		Ping_data.seq = 0;
	}
	
	macBSN = Ping_data.seq++;
	
	while (TRUE) {
		frame = radio_read_frame(slot);
		if (frame == NOFRAME)	{
			return (0);
		}
		printf("Got ping %d\n", loop++);
		ping = (pingPacket_t *)frame->data;
		if (ping->pcf.replyRequested == 1)	{
			temp_pid = frame->dest_pid;
			frame->dest_pid = frame->src_pid;
			frame->src_pid = temp_pid;
			temp_addr = frame->dest_addr.saddr;
			frame->dest_addr.saddr = frame->src_addr.saddr;
			frame->src_addr.saddr = temp_addr;
			
			/* turn around delay of 12 symbol times */
			pauseSymbolTimes(RADIO_TIMER, 12);	/* standard 12 Symbol Times */
			//			frameHeaderDump("Slave Echo", frame, 40);
			radio_send_frame(	Ping_data.dest_addr,
								Ping_data.dest_panID,
								Ping_data.src_addr,
								Ping_data.src_panID,
								frame);
		}
		else
			freebuf((int *)frame);
		
	}
	
	return(OK);
}
