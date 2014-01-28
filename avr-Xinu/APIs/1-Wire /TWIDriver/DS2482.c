//
//  DS2482.c
//  avr-Xinu TWIDriver
//
//	MAXIM DS2482-100 Single-Channel 1-Wire Master
//	Modtronix Engineering im1WP
//	implemented on Olimex AVR-P40-USB development board with ATmega1284p
//
//  Created by Michael Minor on 1/2/12.
//  Copyright (c) 2012. All rights reserved.
//

#include <avr-Xinu.h>
#include "DS2482.h"
#include "twi.h"
#include <stdlib.h>
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
//  an3684.C - Application Note 3684 example implementation using CMAXQUSB.
//

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

//--------------------------------------------------------------------------
// Write the configuration register in the DS2482. The configuration 
// options are provided in the lower nibble of the provided config byte. 
// The uppper nibble are bitwise inverted when written to the DS2482.
//  
// Returns:  TRUE: config written and response correct
//           FALSE: response incorrect
//
int DS2482_write_config(unsigned char config)
{
	unsigned char read_config;
	uint8_t cmd[2] = {DS2482_CMD_WCFG, 0x00};	//Write Configuration command
	struct twi_Command cb2 = TWI_SETUP_CMDBLOCK(DS2482_SLAVEADDRESS, TW_READ, &read_config, 1, 0);
	struct twi_Command cb1 = TWI_SETUP_CMDBLOCK(DS2482_SLAVEADDRESS, TW_WRITE, cmd, 2, &cb2);
		

	cmd[1] = config | (~config << 4); //Configuration byte
	
	if ( twi_doCommand(&cb1) == OK && config == read_config)
		return TRUE;
	
	// handle error
	// ...
	DS2482_reset();
		
	return FALSE;
}

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

//--------------------------------------------------------------------------
// Calculate the CRC8 of the byte value provided with the current 
// global 'crc8' value. 
// Returns current global crc8 value
//
unsigned char calc_crc8(unsigned char data)
{
	int i; 
	
	// See Application Note 27
	crc8 = crc8 ^ data;
	for (i = 0; i < 8; ++i)
	{
		if (crc8 & 1)
			crc8 = (crc8 >> 1) ^ 0x8c;
		else
			crc8 = (crc8 >> 1);
	}
	
	return crc8;
}

//--------------------------------------------------------------------------
// Use the DS2482 help command '1-Wire triplet' to perform one bit of a 1-Wire
// search. This command does two read bits and one write bit. The write bit
// is either the default direction (all device have same bit) or in case of 
// a discrepancy, the 'search_direction' parameter is used. 
//
// Returns the DS2482 status byte result from the triplet command or SYSERR
//
unsigned char DS2482_search_triplet(int search_direction)
{
	unsigned char status;
	int poll_count = 0;
	uint8_t OWtc[2] = {DS2482_CMD_1WT, 0x00};	//1-Wire Triplet command
	struct twi_Command cb2 = TWI_SETUP_CMDBLOCK(DS2482_SLAVEADDRESS, TW_READ ,&status, 1, 0);
	struct twi_Command cb1 = TWI_SETUP_CMDBLOCK(DS2482_SLAVEADDRESS, TW_WRITE, OWtc, 2, &cb2);

	OWtc[1] = search_direction ? 0x80 : 0x00;
	
	if ( twi_doCommand(&cb1) == SYSERR )
		goto errorReturn;
	while ( twi_doCommand(&cb2) == OK && (status & DS2482_STATUS_1WB) && (poll_count++ < POLL_LIMIT))	{
		continue;
	}
	
	// check for failure due to poll limit reached
	if (poll_count >= POLL_LIMIT)
	{
	errorReturn:
		// handle error
		// ...
		DS2482_reset();
		return SYSERR;
	}
	
	// return status byte
	return status;
}
