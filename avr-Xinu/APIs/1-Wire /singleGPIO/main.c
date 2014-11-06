/**
 *  \file main.c
 *  \project 1-Wire_SingleGPIO
 */

//
//  Created by Michael Minor on 10/31/14.
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
#include <stdlib.h>
#include <1-WireAPI.h>

/********************************************************************************//*!
 * \fn int function()
 * \brief Enter function description.
 * \param enter parameter
 * \return enter return
 * \return else return
 ***********************************************************************************/

// global search state
unsigned char ROM_NO[8];
int LastDiscrepancy;
int LastFamilyDiscrepancy;
int LastDeviceFlag;
unsigned char crc8;

//--------------------------------------------------------------------------
// TEST BUILD MAIN
//
int main(void)
{
	int rslt,i,cnt;
	double temp;
	char buf[12];
	
	OW_Enable();
	// find ALL devices
	printf("\nFIND ALL\n");
	cnt = 0;
	rslt = OWFirst();
	while (rslt)	{
		// print device found
		for (i = 7; i >= 0; i--)
			printf("%02X", ROM_NO[i]);
		printf("  %d\n",++cnt);
		
		rslt = OWNext();
	}
	
	// find only 0x10
	printf("\nFIND ONLY 0x10\n");
	cnt = 0;
	OWTargetSetup(0x10);
	while (OWNext())	{
		// check for incorrect type
		if (ROM_NO[0] != 0x10)
			break;
		
		// print device found
		for (i = 7; i >= 0; i--)
			printf("%02X", ROM_NO[i]);
		printf("  %d\n",++cnt);
	}
	if (cnt == 1)	{
		DS1820_startConversion();
		uint8_t spdata[10];
		sleep10(7);
		DS1820_readScratchPad(spdata);
		temp = DS1820_getTemperature(spdata);
		temp = temp*9.0/5.0 +32.0;
		printf("TEMP(F) = %s\n\n",dtostrf(temp,7,2,(char *)buf));
	}
	
	// find all but 0x04, 0x1A, 0x23, and 0x01
	printf("\nFIND ALL EXCEPT 0x10, 0x04, 0x0A, 0x1A, 0x23, 0x01\n");
	cnt = 0;
	rslt = OWFirst();
	while (rslt)	{
		// check for incorrect type
		if ((ROM_NO[0] == 0x04) || (ROM_NO[0] == 0x1A) ||
			(ROM_NO[0] == 0x01) || (ROM_NO[0] == 0x23) ||
			(ROM_NO[0] == 0x0A) || (ROM_NO[0] == 0x10))
			OWFamilySkipSetup();
		else	{
			// print device found
			for (i = 7; i >= 0; i--)
				printf("%02X", ROM_NO[i]);
			printf("  %d\n",++cnt);
		}
		
		rslt = OWNext();
	}
	////////////////////////////////////////////////////////////////////////////////////////////////
	// find ALL DS2438
	double voltage = 0.0;
	int myDS2438 = 0;
	uint8_t DS2438deviceNumber[8];
	double bigT = -1.;
	int micro10voltage = 0;
	
	printf("\nFind all DS2438; Read Temperature and Voltage\n");
	for (rslt=OWFirstType(0x26); rslt; rslt=OWNextType(0x26))	{
		// print device found
		for (i = 7; i >= 0; i--)
			printf("%02X", ROM_NO[i]);
		printf("\n");
		
		// Save the first ROM
		if (myDS2438 == 0)	{
			myDS2438++;
			for (i = 0; i < 8; i++)	{
				DS2438deviceNumber[i] = ROM_NO[i];
			}
		}
		
		if (myDS2438 > 0)	{
			// (default) Read Vdd
			SetupAtoD(0x0F, DS2438deviceNumber);
			
			bigT = Get_Temperature(DS2438deviceNumber);
			printf("TEMP = %s\n\n",dtostrf(bigT,7,3,(char *)buf));
			
			voltage = ReadAtoD(DS2438deviceNumber);
			printf("VDD = %s\n",dtostrf(voltage,6,2,(char *)buf));
		}
		if (myDS2438 > 0)	{
			// (default) Read Vad
			SetupAtoD(0x07, DS2438deviceNumber);
			voltage = ReadAtoD(DS2438deviceNumber);
			micro10voltage = getDS2438Voltage(DS2438deviceNumber);
			printf("VAD = %s\n",dtostrf(voltage,6,2,(char *)buf));
			printf("Micro V = %d\n", micro10voltage);
		}
	}
	
	sleep(1);
	return 0;
}
