/**
 *  \file frameInput.c
 *  \project 802_15_4_Mib
 */

//
//  Created by Michael Minor on 1/31/14.
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

#include <frame802154.h>
#include <avr/io.h>
#include <registermap.h>
#include <radio.h>
#include <macSymbolCounter.h>
#include <stdlib.h>
#include <rfr2_radioDriver.h>

#define __ASSERT_USE_STDERR
#include <assert.h>
#define ASSERT(e) assert(e)

/********************************************************************************//*!
 * \fn PROCESS frameInput()
 * \brief Read frames and parse.
 * \param enter parameter
 ***********************************************************************************/

PROCESS frameInput(int nvar, int *pvar)
{
	frame802154_t *parsedFrame;
	int len;
	uint8_t radioChannel = 11;
	
	parsedFrame = (frame802154_t *)getmem(sizeof(frame802154_t));
	radio_set_operating_channel(radioChannel);
	ASSERT(radio_get_operating_channel()==11);
	kprintf("frameInput is running.\n");
	kprintf("Sniff channel 11\n");
	while (1)	{
		len = read(RADIO, (unsigned char *)parsedFrame, MAX_FRAME_LENGTH);
		if (len == SYSERR)	{
			kprintf("Can't read RADIO\n");
			kprintf("Exiting frameInput process\n");
			return (0);
		} else if (len == RADIO_TIMEOUT)	{
			kprintf(".");
		} else	{
			kprintf("\nlen = %d\n", len);
			kprintf("header length = %d\n", parsedFrame->header_len);
			kprintf("data length = %d\n", parsedFrame->data_len);
			kprintf("Source Address = <%02x%02x%02x%02x%02x%02x%02x%02x>\n",  parsedFrame->src_addr[7],
					parsedFrame->src_addr[6], parsedFrame->src_addr[5], parsedFrame->src_addr[4],
					parsedFrame->src_addr[3], parsedFrame->src_addr[2], parsedFrame->src_addr[1],
																		parsedFrame->src_addr[0]);
			frameDump("frameInput",(uint8_t *)parsedFrame->data,parsedFrame->data_len);
		}
	}
}
