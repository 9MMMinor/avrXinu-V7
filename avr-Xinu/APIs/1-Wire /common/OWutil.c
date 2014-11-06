/**
 *  \file OWutil.c
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
#include <1-WireAPI.h>

/********************************************************************************//*!
 * \fn int function()
 * \brief Enter function description.
 * \param enter parameter
 * \return enter return
 * \return else return
 ***********************************************************************************/

//
//	Reset 1W bus and access LaserROM
//
int OWSerialNumber(unsigned char *LaserROM)
{
	unsigned char sendpacket[10];
	int i;
	
	// select the device
	sendpacket[0] = 0x55;	// match command
	for (i = 0; i < 8; i++)
		sendpacket[i+1] = LaserROM[i];
	OWReset();				// Reset 1-Wire
	OWBlock(sendpacket,9);	// MATCH ROM sequence
	
	return ( OWAccessSerialNumber(LaserROM) );
}

//
//	return Verify access; do not reset 1-Wire bus
//
int OWAccessSerialNumber(unsigned char *LaserROM)
{
	int i;
	
	for (i = 0; i < 8; i++)
		ROM_NO[i] = LaserROM[i];
	return ( OWVerify() );
}

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
// Send 8 bits of communication to the 1-Wire Net and return the
// result 8 bits read from the 1-Wire Net.  The parameter 'sendbyte'
// least significant 8 bits are used and the least significant 8 bits
// of the result is the return byte.
//
// 'sendbyte' - 8 bits to send (least significant byte)
//
// Returns:  8 bits read from sendbyte
//
unsigned char OWTouchByte(unsigned char sendbyte)
{
	if (sendbyte == 0xFF)
		return OWReadByte();
	else
	{
		OWWriteByte(sendbyte);
		return sendbyte;
	}
}

//--------------------------------------------------------------------------
// The 'OWBlock' transfers a block of data to and from the
// 1-Wire Net. The result is returned in the same buffer.
//
// 'tran_buf' - pointer to a block of unsigned
//              chars of length 'tran_len' that will be sent
//              to the 1-Wire Net
// 'tran_len' - length in bytes to transfer
//
void OWBlock(unsigned char *tran_buf, int tran_len)
{
	int i;
	
	for (i = 0; i < tran_len; i++)
		tran_buf[i] = OWTouchByte(tran_buf[i]);
}

/*! \brief	Calculate the CRC8 of the byte value provided with the current
 *			global 'crc8' value.
 *
 *  \param  data  One byte of data to compute CRC from.
 *
 *  \return Returns current global crc8 value.
 *
 */
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
