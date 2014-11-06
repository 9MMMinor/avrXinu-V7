//
//  DS2482_reset.c
//  TWIDriver
//
//  Created by Michael Minor on 1/9/12.
//  Copyright (c) 2012. All rights reserved.
//

#include <avr-Xinu.h>
#include <DS2482.h>
#include <twi.h>
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
// Perform a device reset on the DS2482
//
// Returns: TRUE if device was reset
//          FALSE device not detected or failure to perform reset
//
int DS2482_reset()
{
	uint8_t status;
	uint8_t drst = DS2482_CMD_DRST;		// Reset command
	struct twi_Command cb2 = TWI_SETUP_CMDBLOCK(DS2482_SLAVEADDRESS, TW_READ, &status, 1, 0);
	struct twi_Command cb1 = TWI_SETUP_CMDBLOCK(DS2482_SLAVEADDRESS, TW_WRITE, &drst, 1, &cb2);
	
	if ( twi_doCommand(&cb1) == OK && (status & 0xF7) == 0x10 )
		return TRUE;
	
	return FALSE;
}
