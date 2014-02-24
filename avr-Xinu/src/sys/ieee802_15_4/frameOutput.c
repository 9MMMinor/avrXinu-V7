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
	frame802154_t frameStructure;
	uint8_t radioChannel = 11;
	
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

	memcpy(frameStructure.data, "this is my payload", 18);
	frameStructure.data_len = 18;
	frameStructure.header_len = 0;
	frameStructure.header_len = getFrameHdrLength(&frameStructure);
	
//	frameHeaderDump("frameOutput", (frame802154_t *)&myFrame.data, 40);
//	frameDump("frameOutput", (uint8_t *)&myFrame, 40);
	
	for (;;)	{
		write(RADIO, (unsigned char *)&frameStructure, frameStructure.header_len + frameStructure.data_len + 2);
		kprintf("write sequence = %d\n",frameStructure.seq);
		frameStructure.seq++;
		sleep(30);
	}
}
