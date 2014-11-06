/**
 *  \file DS2438.c
 *  \project 1-Wire API
 */

//
//  Created by Michael Minor on 6/23/14.
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
//  an3684.C - Application Note 3684.
//		and
//	Public Domain 1-Wire Net Functions
//	Version 3.10
//	3/24/06
//	Copyright (C) 2006 Dallas Semiconductor
/*	Copyright (C) 1999-2006 Dallas Semiconductor Corporation,
 *	All Rights Reserved.
 
 *	Permission is hereby granted, free of charge,
 *	to any person obtaining a copy of this software and
 *	associated documentation files (the "Software"), to
 *	deal in the Software without restriction, including
 *	without limitation the rights to use, copy, modify,
 *	merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom
 *	the Software is furnished to do so, subject to the
 *	following conditions:
 *
 *	The above copyright notice and this permission notice
 *	shall be included in all copies or substantial portions
 *	of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
 *	ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
 *	TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 *	PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *	IN NO EVENT SHALL DALLAS SEMICONDUCTOR BE LIABLE FOR ANY
 *	CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *	OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *	IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *	DEALINGS IN THE SOFTWARE.
 *
 *	Except as contained in this notice, the name of
 *	Dallas Semiconductor shall not be used except as stated
 *	in the Dallas Semiconductor Branding Policy.
 */

//	1-Wire routines for Xinu are derived from owpd310r2.zip
//	Avail from Maxim: http://www.maximintegrated.com/en/products/ibutton/software/1wire/wirekit.cfm
//  Created by Michael Minor on 3/14/13.
//  Copyright 2013.

/********************************************************************************//*!
 * \fn int function()
 * \brief Enter function description.
 * \param enter parameter
 * \return enter return
 * \return else return
 ***********************************************************************************/

#include <1-WireAPI.h>

/*
 * Sets the DS2438 Status/Configuration Byte
 * (Sets the DS2438 to read Vad or Vdd)
 *
 * configurationByte	sets bits (AD | EE | CA | IAD)
 * in Page 0, Byte 0.
 * AD = 1 (default) selects VDD
 * AD = 0 selects VAD
 * LaserROM the serial number for the part that the read is
 *          to be done on.
 *
 * returns the positive status byte if the part is present and responding
 * else a negative error code
 */

int SetupAtoD(uint8_t configurationByte, unsigned char *LaserROM)
{
	uint8_t status;
	int busybyte;
	
	if ( !OWSerialNumber(LaserROM) )	{
		// return (negative_error_code);
		return -1;
	}
	printf("SetupAtoD: config byte = 0x%02X\n", configurationByte);
	// Recall the Status/Configuration page
	OWWriteByte(RECALLSCRATCH);	// Recall command
	OWWriteByte(0x00);			// Page to Recall
	OWReset();
	
	OWAccessSerialNumber(LaserROM);
	// Read the Status/Configuration byte
	OWWriteByte(READSCRATCH);	// Read scratchpad command
	OWWriteByte(0x00);			// Page for the Status/Configuration byte
	status = OWReadByte();
	OWReset();
	
	if( (status & 0x0F) == configurationByte )	{
		printf("SetupAtoD: Read config byte = 0x%02X\n", status);
		///////// NO CHANGE
		return status;
	}
	///////// CHANGE
	//	printf("SetupAtoD: Change config byte 0x%02X to 0x%02X\n", status, configurationByte);
	OWAccessSerialNumber(LaserROM);
	
	// Write the Status/Configuration byte
	OWWriteByte(WRITESCRATCH);	// Write scratchpad command
	OWWriteByte(0x00);			// Write page
	OWWriteByte(configurationByte & 0x0F);
	OWReset();
	
	OWAccessSerialNumber(LaserROM);
	// Copy the Status/Configuration byte
	OWWriteByte(READSCRATCH);	// Read scratchpad command
	OWWriteByte(0x00);			// Read page
	status = OWReadByte();
	OWReset();
	
	OWAccessSerialNumber(LaserROM);
	// Copy the Status/Configuration byte
	OWWriteByte(COPYSCRATCH);	// Copy scratchpad command
	OWWriteByte(0x00);			// Copy page
	for (busybyte = OWReadByte(); busybyte == 0; busybyte = OWReadByte())	{
		sleep10(1);
	}
	OWReset();
	printf("SetupAtoD: Write config byte =0x%02X done.\n", status);
	return status;
}

/*
 * Read the Vdd or the Vad from the DS2438
 *
 * before the call, a call to SetupAtoD() might be necessary.
 * LaserROM the serial number for the part that the read is
 *          to be done on.
 *
 * return the floating point value of Vad or Vdd
 */

double ReadAtoD(unsigned char *LaserROM)
{
	int volt;
	
	if (( volt = getDS2438Voltage(LaserROM) ) == SYSERR)	{
		return (double)-1.0;
	}
	return (double) volt/100;
}

/*
 * Read the Vdd or the Vad from the DS2438
 *
 * before the call, a call to SetupAtoD() might be necessary.
 * LaserROM the serial number for the part that the read is
 *          to be done on.
 *
 * return the value of Vad or Vdd in units of 10 mV
 */

int getDS2438Voltage(unsigned char *LaserROM)
{
	uint8_t pageByte[9];	// data + CRC
	uint8_t byte;
	int i;
	int busybyte;
	uint8_t dataCRC;
	
	if ( !OWAccessSerialNumber(LaserROM) )	{
		return SYSERR;
	}
	OWWriteByte(CONVERTV);		// Convert V Command
	// Read time slots to detect done
	for (busybyte = OWReadByte(); busybyte == 0; busybyte = OWReadByte())	{
		//		printf("V conversion Busy\n");
		sleep10(1);		// sleep 1 TICK (way too long)
	}				// The conversion normally takes up to 4ms
	OWReset();
	
	if ( !OWAccessSerialNumber(LaserROM) )	{
		return SYSERR;
	}
	OWWriteByte(RECALLSCRATCH);	// Recall Memory command
	OWWriteByte(0x00);			// Page to Recall
	OWReset();
	
	if ( !OWAccessSerialNumber(LaserROM) )	{
		return SYSERR;
	}
	// Read scratchpad command
	OWWriteByte(READSCRATCH);	// Read Scratchpad
	// Page for the Status/Configuration byte
	OWWriteByte(0x00);
	crc8 = 0;
	for(i=0;i<8;i++)	{
		byte = OWReadByte();
		calc_crc8(byte);
		pageByte[i] = byte;
	}
	dataCRC = OWReadByte();
	
	// Read the Status/Configuration byte

//	printf("ReadAtoD status = 0x%02X\n", pageByte[0]);
//	printf("ReadAtoD CRC=%x crc8=%x\n", dataCRC, crc8);
	
	if (dataCRC == crc8)	{
		return ( (pageByte[VOLT_MSB] << 8) | pageByte[VOLT_LSB] );
	}
	return SYSERR;
}

/**
 * Reads the temperature from the DS2438.
 *
 * LaserROM the serial number for the part that the read is
 *          to be done on.
 *
 * @return the temperature in returned in C.
 */
double Get_Temperature(unsigned char *LaserROM)
{
	uint8_t pageByte[9];	// data + CRC
	uint8_t busybyte;
	int i;
	
	if ( !OWSerialNumber(LaserROM) )	{
		return -99.0;
	}
	OWWriteByte(CONVERTT);
	// Read time slots to detect done
	for (busybyte = OWReadByte(); busybyte == 0; busybyte = OWReadByte())	{
		//		printf("T conversion Busy\n");
		sleep10(1);		// sleep 1 TICK (way too long)
	}					// The conversion normally takes up to 10ms
	OWReset();
	
	if ( !OWAccessSerialNumber(LaserROM) )	{
		return -98.0;
	}
	// Recall the Status/Configuration page
	OWWriteByte(RECALLSCRATCH);	// Recall command
	OWWriteByte(0x00);	// Page to Recall
	OWReset();
	
	if ( !OWAccessSerialNumber(LaserROM) )	{
		return -98.0;
	}
	// Read the Status/Configuration byte
	OWWriteByte(READSCRATCH);	// Read scratchpad command
	OWWriteByte(0x00);			// Page for the Status/Configuration byte
	for (i=0; i<9; i++)
		pageByte[i] = OWReadByte();
	OWReset();
	
	return (double)( (((pageByte[TEMP_MSB] << 8) | pageByte[TEMP_LSB]) >> 3) * 0.03125 );
}

/**
 * Reads the Current from the DS2438.
 *
 * LaserROM the serial number for the part that the read is
 *          to be done on.
 *
 * @return the Current is returned in mA.
 */
double Get_Current(unsigned char *LaserROM)
{
	uint8_t pageByte[9];	// data + CRC
	int i;
	
	if ( !OWSerialNumber(LaserROM) )	{
		return -99.0;
	}
	
	if ( !OWAccessSerialNumber(LaserROM) )	{
		return -98.0;
	}
	// Recall the Status/Configuration page
	OWWriteByte(RECALLSCRATCH);	// Recall command
	OWWriteByte(0x00);	// Page to Recall
	OWReset();
	
	if ( !OWAccessSerialNumber(LaserROM) )	{
		return -98.0;
	}
	// Read the Status/Configuration byte
	OWWriteByte(READSCRATCH);	// Read scratchpad command
	OWWriteByte(0x00);			// Page for the Status/Configuration byte
	for (i=0; i<9; i++)
		pageByte[i] = OWReadByte();
	OWReset();
	
	return ( (double)(((pageByte[CURR_MSB] << 8) | pageByte[CURR_LSB]) * 2.441) );
}

/*
 *	Read the DS2438 Page, page, into the byte string, pageByte
 *
 *	LaserROM the serial number for the part that the read is
 *          to be done on.
 *	Should read the CRC and check it.
 *
 */

int Read_DS2438Page(unsigned char *LaserROM, int page, uint8_t *pageByte)
{
	uint8_t Page = page & 0x07;
	uint8_t byte;
	int i;
	
	if ( !OWSerialNumber(LaserROM) )	{
		// return (negative_error_code);
		return -1;
	}
	// Recall the page
	OWWriteByte(RECALLSCRATCH);	// Recall command
	OWWriteByte(Page);			// Page to Recall
	OWReset();
	
	OWAccessSerialNumber(LaserROM);
	// Read the Status/Configuration byte
	OWWriteByte(READSCRATCH);	// Read scratchpad command
	OWWriteByte(Page);			// Page for the Status/Configuration byte
	crc8 = 0;
	for (i=0; i<8; i++)	{
		byte = OWReadByte();
		*pageByte++ = byte;
		calc_crc8(byte);
	}
	byte = OWReadByte();		// Read crc8
	OWReset();
	if (byte == crc8)
		return OK;
	else	{
		printf("Read page %d CRC error\n", page);
		return -99;
	}
}

/*
 *	Write DS2438 Page, page, from the byte string, pageByte.
 *
 *	LaserROM the serial number for the part that the read is
 *          to be done on.
 *	Should write one more byte, the CRC, and not Reset.
 *
 */

int Write_DS2438Page(unsigned char *LaserROM, int page, uint8_t *pageByte)
{
	uint8_t Page = page & 0x07;
	int i, busybyte;
	
	if ( !OWSerialNumber(LaserROM) )	{
		// return (negative_error_code);
		return -1;
	}
	OWWriteByte(WRITESCRATCH);	// Write scratchpad command
	OWWriteByte(Page);			// Write page
	for (i=0; i<8; i++)	{
		OWWriteByte(*pageByte++);
	}
	OWReset();
	
	OWAccessSerialNumber(LaserROM);
	OWWriteByte(COPYSCRATCH);	// Copy scratchpad command
	OWWriteByte(Page);			// Copy page
	for (busybyte = OWReadByte(); busybyte == 0; busybyte = OWReadByte())	{
		sleep10(1);
	}
	OWReset();
	
	return OK;
}
