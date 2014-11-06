//
//  OWWriteBit.c OWReadbit.c OWTouchBit.c
//  TWIDriver
//
//  Created by Michael Minor on 1/9/12.
//  Copyright (c) 2012. All rights reserved.
//

#include <avr-Xinu.h>
#include <1-WireAPI.h>
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
// Send 1 bit of communication to the 1-Wire Net.
// The parameter 'sendbit' least significant bit is used.
//
// 'sendbit' - 1 bit to send (least significant byte)
//
void OWWriteBit(unsigned char sendbit)
{
	OWTouchBit(sendbit);
}

//--------------------------------------------------------------------------
// Reads 1 bit of communication from the 1-Wire Net and returns the
// result
//
// Returns:  1 bit read from 1-Wire Net
//
unsigned char OWReadBit(void)
{
	return OWTouchBit(0x01);
}

//--------------------------------------------------------------------------
// Send 1 bit of communication to the 1-Wire Net and return the
// result 1 bit read from the 1-Wire Net.  The parameter 'sendbit'
// least significant bit is used and the least significant bit
// of the result is the return bit.
//
// 'sendbit' - the least significant bit is the bit to send
//
// Returns: 0:   0 bit read from sendbit
//          1:   1 bit read from sendbit
//
unsigned char OWTouchBit(unsigned char sendbit)
{
	unsigned char status;
	int poll_count = 0;
	struct twi_Command cb1, cb2;
	uint8_t OWsb[2];
	
	// 1-Wire bit (Case B)
	//   S AD,0 [A] 1WSB [A] BB [A] Sr AD,1 [A] [Status] A [Status] A\ P
	//                                          \--------/        
	//                           Repeat until 1WB bit has changed to 0
	//  [] indicates from slave
	//  BB indicates byte containing bit value in msbit
	
	//	I2C_start();
	//	I2C_write(I2C_address | I2C_WRITE, EXPECT_ACK);
	//	I2C_write(CMD_1WSB, EXPECT_ACK);
	//	I2C_write(sendbit ? 0x80 : 0x00, EXPECT_ACK);
	//	I2C_rep_start();
	//	I2C_write(I2C_address | I2C_READ, EXPECT_ACK);
	
	// loop checking 1WB bit for completion of 1-Wire operation 
	// abort if poll limit reached
	//	status = I2C_read(ACK);
	//	do
	//	{
	//		status = I2C_read(status & DS2482_STATUS_1WB);
	//	}
	//	while ((status & DS2482_STATUS_1WB) && (poll_count++ < POLL_LIMIT));
	
	//	I2C_stop();
	OWsb[0] = DS2482_CMD_1WSB;			//1-Wire Single Bit command
	OWsb[1] = sendbit ? 0x80 : 0x00;
	cb1.slarw = DS2482_SLAVEADDRESS<<1|TW_WRITE;
	cb1.data = OWsb;
	cb1.dataLength = 2;
	cb1.chain = &cb2;
	cb2.slarw = DS2482_SLAVEADDRESS<<1|TW_READ;
	cb2.data = &status;
	cb2.dataLength = 1;
	cb2.chain = (struct twi_Command *)0;
	
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
		return 0;
	}
	
	// return bit state
	if (status & DS2482_STATUS_SBR)
		return 1;
	else
		return 0;
}
