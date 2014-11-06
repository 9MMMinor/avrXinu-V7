/**
 *  \file DS18S20.c
 *  \project 1-Wire_SingleGPIO
 */

//
//  Created by Michael Minor on 11/1/14.
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
#include <1-WireAPI.h>

/********************************************************************************//*!
 * \fn int function()
 * \brief Enter function description.
 * \param enter parameter
 * \return enter return
 * \return else return
 ***********************************************************************************/

/*!
 **************************************************************************
 *	\fn void DS1820_readScrtachPad(uint8_t *data)
 *
 *	\brief Read DS18S20 Scratchpad. NOTE: only use with a single device on
 *			the 1-Wire bus.
 *	\param uint8_t *data	Pointer to scratchpad data array of length 8.
 **************************************************************************
 */
void DS1820_readScratchPad(uint8_t *data)
{
	int k;
	
	OWReset();
	OWWriteByte(0xCC); // Skip ROM
	OWWriteByte(0xBE); // Read Scratch Pad
	for (k = 0; k < 9; k++)	{
		*data++ = OWReadByte();
	}
}

/*!
 **************************************************************************
 *	\fn void DS1820_startConversion(void)
 *
 *	\brief Start a DS18S20 temperature conversion. NOTE: only use with a
 *			single device on the 1-Wire bus.
 **************************************************************************
 */
void DS1820_startConversion(void)
{
	
	OWReset();
	OWWriteByte(0xCC); // Skip ROM
	OWWriteByte(0x44); // Start a Conversion
}

/*!
 **************************************************************************
 *	\fn double DS1820_getTemperature(uint8_t *data)
 *
 *	\brief Convert DS18S20 Scratchpad data to high resolution temperature(C).
 *	\param uint8_t *data	Pointer to scratchpad data array of length 8.
 **************************************************************************
 */
double DS1820_getTemperature(uint8_t *scratchdata)
{
	float tmp, cr, cpc;
	int tsht;
//	double ipart;
//	float fraction;
//	char buf[10];
	
	tsht = (int)(scratchdata[1]<<8 | scratchdata[0]);
	tmp = (float)(tsht/2);
	cr = scratchdata[6];
	cpc = scratchdata[7];
	tmp = tmp - (float)0.25 + (cpc - cr)/cpc;
	
//	fraction = (float)modf( (double)tmp, &ipart );	//get fraction and ipart from tmp
//	tf = (int)(fraction*100.);
//	printf("High res temp = %d.%02d\n", (int)ipart, tf);
	
//	printf("Yet another TEMP = %s\n",dtostrf((double)tmp,6,2,(char *)buf));
	return ( (double)tmp );
}