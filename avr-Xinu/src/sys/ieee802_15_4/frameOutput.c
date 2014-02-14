/**
 *  \file frameOutput.c
 *  \project 802_15_4_Mib
 */

//
//  Created by Michael Minor on 2/12/14.
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

#include <avr-Xinu.h>	/* includes avr-libc <stdio.h>	*/
#include <frame802154.h>
#include <avr/io.h>
#include <registermap.h>
#include <radio.h>
#include <macSymbolCounter.h>
#include <stdlib.h>
#include <rfr2_radioDriver.h>
#include <avr/pgmspace.h>


#define __ASSERT_USE_STDERR
#include <assert.h>
#define ASSERT(e) assert(e)

/********************************************************************************//*!
* \fn PROCESS frameOutput()
* \brief Write a test frame.
************************************************************************************/

PROCESS frameOutput(int nvar, int *pvar)
{
	frame_t myFrame;
	frame802154_t frameStructure;
	uint8_t radioChannel = 11;
	uint8_t *payload;
	frame802154_t *header;
	
	radio_set_operating_channel(radioChannel);
	ASSERT(radio_get_operating_channel()==11);
	kprintf("frameOutput is running.\n");

	frameStructure.fcf.frameType = FRAME_TYPE_DATA;
	frameStructure.fcf.frameSecurity = 0;
	frameStructure.fcf.frameAckRequested = 0;
	frameStructure.fcf.frameVersion = FRAME_VERSION_2006;
	frameStructure.fcf.frameDestinationAddressMode = FRAME_ADDRESS_MODE_SHORT;
	frameStructure.fcf.frameSourceAddressMode = FRAME_ADDRESS_MODE_SHORT;
	
	frameStructure.seq = 0;
	frameStructure.dest_pid = 0xab;
	frameStructure.dest_addr[0] = 0xbe;
	frameStructure.dest_addr[1] = 0xba;
	
	frameStructure.src_addr[0] = 0xa0;
	frameStructure.src_addr[1] = 0xa1;

	frameStructure.src_pid = 0x1234;

	payload = makeTXFrameHdr(&frameStructure, (octet_t *)&myFrame.data);
	memcpy( (void *)payload, "this is my payload", 18);
	myFrame.length = MAX_FRAME_LENGTH;
	
	frameHeaderDump("frameOutput", (frame802154_t *)&myFrame.data, 40);
	frameDump("frameOutput", (uint8_t *)&myFrame, 40);
	header = (frame802154_t *)&myFrame.data;
	
	for (;;)	{
		write(RADIO, (unsigned char *)&myFrame, MAX_FRAME_LENGTH);
		kprintf("write sequence = %d\n",header->seq);
		header->seq++;
		sleep(30);
	}
	return (0);
}
