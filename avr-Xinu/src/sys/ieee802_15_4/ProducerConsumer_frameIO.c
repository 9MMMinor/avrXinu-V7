/**
 *  \file ProducerConsumer_frameIO.c
 *  \project 802_15_4_Mib
 */

//
//  Created by Michael Minor on 3/2/14.
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
#include "frameIO.h"

extern struct radioinfo Radio;

/********************************************************************************//*!
 * \fn int function()
 * \brief Enter function description.
 * \param enter parameter
 * \return enter return
 * \return else return
 ***********************************************************************************/
int
main( void )
{
	int produced, consumed;
	int prod(int argc, int *argv);
	int cons(int argc, int *argv);
	
	kprintf("Hello World\n");
	
//	control(RADIO, RADIO_SET_READ_WITH_TIMEOUT, 0, 0);
#ifdef MASTER
	consumed = screate(1);
	produced = screate(0);
#endif
#ifdef SLAVE
	consumed = screate(0);
	produced = screate(1);
#endif
	
	resume( create(cons, 600, 20, "cons", 2, consumed, produced) );
	resume( create(prod, 600, 20, "prod", 2, consumed, produced) );
	return 0;
}

/*
 *	prod -- produce a frame, wait for Ack
 */
int
prod(int argc, int *argv)
{
	int consumed, produced;
	frame802154_t *frame;
	
	consumed = argv[0];
	produced = argv[1];

	for (;;)	{
		wait(consumed);
		frame = (frame802154_t *)getbuf(Radio.radiopool);
		frame->fcf.frameType = FRAME_TYPE_DATA;
		frame->fcf.frameSecurity = 0;
		frame->fcf.frameAckRequested = 1;						/* working with =0 */
		frame->fcf.frameVersion = FRAME_VERSION_2006;
		frame->fcf.frameDestinationAddressMode = FRAME_ADDRESS_MODE_SHORT;
		frame->fcf.frameSourceAddressMode = FRAME_ADDRESS_MODE_SHORT;
		frame->seq = 1;
		frame->dest_pid = 0xab;
		frame->dest_addr[0] = 0xbe;
		frame->dest_addr[1] = 0xba;
		frame->src_addr[0] = 0xa0;
		frame->src_addr[1] = 0xa1;
		frame->src_pid = 0x1234;
#ifdef MASTER
		memcpy(frame->data, "Master payload", 14);
		frame->data_len = 14;
#endif
#ifdef SLAVE
		memcpy(frame->data, "Slave payload", 13);
		frame->data_len = 13;
#endif
		frame->header_len = 0;
		frame->header_len = getFrameHdrLength(frame);
		kprintf("prod: psend\n");
		psend(Radio.foport, (int)frame);		/* writes & freebufs frame */
		
		signal(produced);
	}
	return 0;
}

/*
 *	cons -- consume a frame, write an Ack
 */
int
cons(int argc, int *argv)
{
	int consumed, produced;
	frame802154_t *frame;
	uint8_t buffer[128];
	
	consumed = argv[0];
	produced = argv[1];
	
	for (;;)	{
		wait(produced);
#ifdef MASTER
		kprintf("Master ");
#endif
#ifdef SLAVE
		kprintf("Slave ");
#endif
		frame = (frame802154_t *)preceive(Radio.fiport);
		kprintf("frame = %p\n", frame);
		memcpy(buffer, frame->data, frame->data_len);
		buffer[frame->data_len] = '\0';
		kprintf("cons: payload = %s\n", buffer);
		freebuf((int *)frame);		/* higher levels must free frame when they're done */
		
		sleep(15);
		
		signal(consumed);
	}
	return 0;
}
