//
//  OWWriteByte.c
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
// Send 8 bits of communication to the 1-Wire Net and verify that the
// 8 bits read from the 1-Wire Net is the same (write operation).
// The parameter 'sendbyte' least significant 8 bits are used.
//
// 'sendbyte' - 8 bits to send (least significant byte)
//
// Returns:  TRUE: bytes written and echo was the same
//           FALSE: echo was not the same
//
void OWWriteByte(unsigned char sendbyte)
{
	unsigned char status;
	uint8_t OWwb[2] = {DS2482_CMD_1WWB, 0x00};		//1-Wire Write Byte command
	int poll_count = 0;
	struct twi_Command cb2 = TWI_SETUP_CMDBLOCK(DS2482_SLAVEADDRESS, TW_READ, &status, 1, 0);
	struct twi_Command cb1 = TWI_SETUP_CMDBLOCK(DS2482_SLAVEADDRESS, TW_WRITE, OWwb, 2, &cb2);
	
	// 1-Wire Write Byte (Case B)
	//   S AD,0 [A] 1WWB [A] DD [A] Sr AD,1 [A] [Status] A [Status] A\ P
	//                                          \--------/        
	//                             Repeat until 1WB bit has changed to 0
	//  [] indicates from slave
	//  DD data to write
	
	//	I2C_start();
	//	I2C_write(I2C_address | I2C_WRITE, EXPECT_ACK);
	//	I2C_write(CMD_1WWB, EXPECT_ACK);
	//	I2C_write(sendbyte, EXPECT_ACK);
	//	I2C_rep_start();
	//	I2C_write(I2C_address | I2C_READ, EXPECT_ACK);
	//	
	//	// loop checking 1WB bit for completion of 1-Wire operation 
	//	// abort if poll limit reached
	//	status = I2C_read(ACK);
	//	do
	//	{
	//		status = I2C_read(status & DS2482_STATUS_1WB);
	//	}
	//	while ((status & DS2482_STATUS_1WB) && (poll_count++ < POLL_LIMIT));
	//	
	//	I2C_stop();
	
	OWwb[1] = sendbyte;
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
	}
}
