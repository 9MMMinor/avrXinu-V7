//
//  DS2482_channel_select.c
//  TWIDriver
//
//  Created by Michael Minor on 1/9/12.
//  Copyright (c) 2012. All rights reserved.
//

#include <avr-Xinu.h>
#include "DS2482.h"
#include "twi.h"
#include <util/twi.h>


//------------Copyright (C) 2008 Maxim Integrated Products --------------
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY,  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL MAXIM INTEGRATED PRODCUTS BE LIABLE FOR ANY CLAIM, DAMAGES
// OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// Except as contained in this notice, the name of Maxim Integrated Products
// shall not be used except as stated in the Maxim Integrated Products
// Branding Policy.
// ---------------------------------------------------------------------------
//
//  an3684.C - Application Note 3684
//


//--------------------------------------------------------------------------
// Select the 1-Wire channel on a DS2482-800. 
//
// Returns: TRUE if channel selected
//          FALSE device not detected or failure to perform select
//
//***********************************UNTESTED*******************************
int DS2482_channel_select(int channel)
{
	unsigned char ch, ch_read, check;
	uint8_t cmd[2] = {DS2482_CMD_CHSL, 0x00};	// write Channel Select Command
	struct twi_Command cb2 = TWI_SETUP_CMDBLOCK(DS2482_SLAVEADDRESS, TW_READ, &check, 1, 0);
	struct twi_Command cb1 = TWI_SETUP_CMDBLOCK(DS2482_SLAVEADDRESS, TW_WRITE, cmd, 2, &cb2);
	
	switch (channel)
	{
		default: case 0: ch = 0xF0; ch_read = 0xB8; break;
		case 1: ch = 0xE1; ch_read = 0xB1; break;
		case 2: ch = 0xD2; ch_read = 0xAA; break;
		case 3: ch = 0xC3; ch_read = 0xA3; break;
		case 4: ch = 0xB4; ch_read = 0x9C; break;
		case 5: ch = 0xA5; ch_read = 0x95; break;
		case 6: ch = 0x96; ch_read = 0x8E; break;
		case 7: ch = 0x87; ch_read = 0x87; break;
	};
	
	cmd[1] = ch; //channel # byte
	
	if ( twi_doCommand(&cb1) == OK && check == ch_read)
		return TRUE;
	return FALSE;
}
