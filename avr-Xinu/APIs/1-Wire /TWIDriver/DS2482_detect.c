//
//  DS2482_detect.c
//  TWIDriver
//
//  Created by Michael Minor on 1/9/12.
//  Copyright (c) 2012. All rights reserved.
//

#include <avr-Xinu.h>
#include "DS2482.h"
#include "twi.h"
#include <util/twi.h>

// Search state
unsigned char ROM_NO[8];
int LastDiscrepancy;
int LastFamilyDiscrepancy;
int LastDeviceFlag;
unsigned char crc8;

// DS2482 state
unsigned char I2C_address;
int short_detected;
int c1WS, cSPU, cPPM, cAPU;

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



//--------------------------------------------------------------------------
// DS2428 Detect routine that sets the I2C address and then performs a 
// device reset followed by writing the configuration byte to default values:
//   1-Wire speed (c1WS) = standard (0)
//   Strong pull-up (cSPU) = off (0)
//   Presence pulse masking (cPPM) = off (0)
//   Active pull-up (cAPU) = on (CONFIG_APU = 0x01)
//
// Returns: TRUE if device was detected and written
//          FALSE device not detected or failure to write configuration byte
//
int DS2482_detect(unsigned char addr)
{
	// set global address
	I2C_address = addr<<1;	// which we do not use!
	// avr-Xinu is using a single DS2482 and a constant address
	// TWI_SETUP_CMDBLOCK macro will not work with a variable address,
	//  but it would be an easy matter to OR an address into CommandBlock[0]
	//  for multiple DS2482s
	
	// reset the DS2482 ON selected address
	if (!DS2482_reset())
		return FALSE;
	
	// default configuration
	c1WS = FALSE;
	cSPU = FALSE;
	cPPM = FALSE;
	cAPU = 0x01;
	
	// write the default configuration setup
	if (!DS2482_write_config(c1WS | cSPU | cPPM | cAPU))
		return FALSE;
	
	return TRUE;
}
