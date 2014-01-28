//
//  Xinu_mainDS2482.c
//  TWIDriver
//
//  Created by Michael Minor on 1/2/12.
//  Copyright (c) 2012. All rights reserved.
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
//

#include "twi.h"
#include "DS2482.h"


uint8_t OWFirstType(unsigned char type)
{
	uint8_t ret;

	OWTargetSetup(type);
	ret = OWNext();
	// check for incorrect type
	if (ROM_NO[0] != type)
		return FALSE;
	return ret;
}

uint8_t OWNextType(unsigned char type)
{
	uint8_t ret;
	
	ret = OWNext();
	// check for incorrect type
	if (ROM_NO[0] != type)
		return FALSE;
	return ret;
}


//--------------------------------------------------------------------------
// AN3684 Code example using avr-Xinu 
//--------------------------------------------------------------------------
int main(void)
{
	unsigned char buf[200];
	int i,rslt;
	int cnt=0;
	unsigned char sendpacket[10], crc_verify;
	int sendlen=0;
	
	twi_init();
	
	// verify DS2482 is present on default address (0x30)
	if (!DS2482_detect(DS2482_SLAVEADDRESS))
	{
		printf("Failed to find and setup DS2482\n");
		return 0;
	}
	
	// The following code performs various operations using the API
	
	// find ALL devices
	printf("\nFIND ALL\n");

	for (cnt = 0, rslt = OWFirst(); rslt; rslt = OWNext())
	{
		// print device found
		for (i = 7; i >= 0; i--)
			printf("%02X", ROM_NO[i]);
		printf("  %d\n",++cnt);
		
	}
	
	// find only 0x1A
	printf("\nFIND ONLY 0x1A\n");
	cnt = 0;
//	OWTargetSetup(0x1A);
	while (OWNextType(0x1A))
	{
//		// check for incorrect type
//		if (ROM_NO[0] != 0x1A)
//			break;
		
		// print device found
		for (i = 7; i >= 0; i--)
			printf("%02X", ROM_NO[i]);
		printf("  %d\n",++cnt);
	}
	
	// find all but 0x04, 0x1A, 0x23, and 0x01
	printf("\nFIND ALL EXCEPT 0x10, 0x04, 0x0A, 0x1A, 0x23, 0x01\n");

	for (cnt = 0, rslt = OWFirst(); rslt; rslt = OWNext())
	{
		// check for incorrect type
		if ((ROM_NO[0] == 0x04) || (ROM_NO[0] == 0x1A) || 
			(ROM_NO[0] == 0x01) || (ROM_NO[0] == 0x23) ||
			(ROM_NO[0] == 0x0A) || (ROM_NO[0] == 0x10))
			OWFamilySkipSetup();
		else
		{
			// print device found
			for (i = 7; i >= 0; i--)
				printf("%02X", ROM_NO[i]);
			printf("  %d\n",++cnt);
		}		
	}
	
	// find ALL DS1920 
	printf("\nFind all DS1920/DS1820 and do conversion\n");
	for (rslt=OWFirstType(0x10); rslt; rslt=OWNextType(0x10))
	{
		// print device found
		for (i = 7; i >= 0; i--)
			printf("%02X", ROM_NO[i]);
		printf("\n");

		// device already selected from search
		// send the convert command
		if (!OWWriteBytePower(0x44))
			printf("Fail convert command\n");
		
		// delay for 1 second
		sleep(1);
			
		// turn off the 1-Wire Net strong pull-up
		OWLevel(MODE_STANDARD); 
	
		// verify complete
		if (OWReadByte() != 0xFF)
			printf("ERROR, temperature conversion was not complete\n");
			
		// select the device
		sendpacket[0] = 0x55; // match command
		for (i = 0; i < 8; i++)
			sendpacket[i+1] = ROM_NO[i];
			
		// Reset 1-Wire 
		if (OWReset())
		{
			// MATCH ROM sequence
			OWBlock(sendpacket,9);
			
			// Read Scratch pad
			sendlen = 0;
			sendpacket[sendlen++] = 0xBE;
			for (i = 0; i < 9; i++)
				sendpacket[sendlen++] = 0xFF;
			
			OWBlock(sendpacket,sendlen);
			
			printf("Scatchpad result = ");
			for (i = 0; i < sendlen; i++)
				printf("%02X",sendpacket[i]);
			printf("\n");
			
			char temp_msb = (char)sendpacket[2]; // Sign byte + lsbit
			char temp_lsb = (char)sendpacket[1]; // Temp data plus lsb
			if (temp_msb <= 0x80){temp_lsb = (temp_lsb/2);} // shift to get whole degree
			temp_msb = temp_msb & 0x80; // mask all but the sign bit
			if (temp_msb >= 0x80) {temp_lsb = (~temp_lsb)+1;} // twos complement
			if (temp_msb >= 0x80) {temp_lsb = (temp_lsb/2);}// shift to get whole degree
			if (temp_msb >= 0x80) {temp_lsb = ((-1)*temp_lsb);} // add sign bit
			printf( "\nTempC= %d degrees C\n", (int)temp_lsb ); // print temp. C
			char temp_c = temp_lsb; // ready for conversion to Fahrenheit
			char temp_f = (((int)temp_c)* 9)/5 + 32;
			printf( "\nTempF= %d degrees F\n", (int)temp_f ); // print temp. F
		}
		else
			printf("NO RESET\n");
	}
	
	// Find a DS2502 and read the entire contents
	printf("\nFind DS2502 and read contents\n");
	if (OWFirstType(0x09))
	{
		// print device found
		for (i = 7; i >= 0; i--)
			printf("%02X", ROM_NO[i]);
		printf("\n");
		
		// device already selected from search
		// send read memory command
		crc8 = 0;
		OWWriteBytePower(0xF0);
		calc_crc8(0xF0);
		
		// send address and verify CRC8
		OWWriteBytePower(0x00);
		calc_crc8(0x00);
		OWWriteBytePower(0x00);
		calc_crc8(0x00);
		crc_verify = OWReadByte();
		
		// check CRC on command sequence
		if (crc_verify != crc8)
			printf("ERROR, CRC8 on command sequence was incorrect\n");
		else 
			printf("CRC8 verified on command sequence\n");
		
		// read the memory
		crc8 = 0;
		// construct block to read device
		for (i = 0; i < 128; i++)
			buf[i] = 0xFF;
		OWBlock(buf,128);
		
		// print the block and calculate CRC8
		printf("DS2502 Contents:\n");
		for (i = 0; i < 128; i++)
		{
			printf("%02X ", buf[i]);
			calc_crc8(buf[i]);
		}
		
		// read CRC8 from device
		crc_verify = OWReadByte();
		
		// check CRC on command sequence
		if (crc_verify != crc8)
			printf("ERROR, CRC8 on command sequence was incorrect\n");
		else 
			printf("CRC8 verified on data\n");
	}
	
	printf("AN3684 end\n");
	sleep(1);
	return 1;
}
