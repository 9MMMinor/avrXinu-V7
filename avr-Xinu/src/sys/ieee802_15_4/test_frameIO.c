/**
 *  \file test_frameIO.c
 *  \project 802_15_4_Mib
 */

//
//  Created by Michael Minor on 3/1/14.
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
	frame802154_t *frame;
	char buffer[128];
	
	printf("Hello World\n");
	
	control(RADIO, RADIO_SET_READ_WITH_TIMEOUT, 0, 0);
	
	for (;;)	{
		frame = (frame802154_t *)preceive(Radio.fiport);
		memcpy(buffer, frame->data, frame->data_len);
		buffer[frame->data_len] = '\0';
		printf("main: payload = %s\n", buffer);
		freebuf((int *)frame);		/* higher levels must free frame when they're done */
	}
	return 0;
}