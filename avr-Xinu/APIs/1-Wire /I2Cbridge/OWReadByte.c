//
//  OWReadByte.c
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
// Send 8 bits of read communication to the 1-Wire Net and return the
// result 8 bits read from the 1-Wire Net.
//
// Returns:  8 bits read from 1-Wire Net
//
unsigned char OWReadByte(void)
{
	unsigned char status;
	uint8_t OWrb[2];
	int poll_count = 0;
	struct twi_Command cb2 = TWI_SETUP_CMDBLOCK(DS2482_SLAVEADDRESS, TW_READ, &status, 1, 0);
	struct twi_Command cb1 = TWI_SETUP_CMDBLOCK(DS2482_SLAVEADDRESS, TW_WRITE, OWrb, 2, &cb2);
	
	// 1-Wire Read Bytes (Case C)
	//   S AD,0 [A] 1WRB [A] Sr AD,1 [A] [Status] A [Status] NAK 
	//                                   \--------/        
	//                     Repeat until 1WB bit has changed to 0
	//   Sr AD,0 [A] SRP [A] E1 [A] Sr AD,1 [A] DD A\ P
	//                                  
	//  [] indicates from slave
	//  DD data read
	
	//	I2C_start();
	//	I2C_write(I2C_address | I2C_WRITE, EXPECT_ACK);
	//	I2C_write(CMD_1WRB, EXPECT_ACK);
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
	//	// check for failure due to poll limit reached
	//	if (poll_count >= POLL_LIMIT)
	//	{
	//		// handle error
	//		// ...
	//		DS2482_reset();
	//		return 0;
	//	}
	OWrb[0] = DS2482_CMD_1WRB;			//1-Wire Read Byte command
//	cb1.slarw = DS2482_SLAVEADDRESS<<1|TW_WRITE;
//	cb1.data = OWrb;
	cb1.dataLength = 1;
//	cb1.chain = &cb2;
//	cb2.slarw = DS2482_SLAVEADDRESS<<1|TW_READ;
//	cb2.data = &status;
//	cb2.dataLength = 1;
//	cb2.chain = (struct twi_Command *)0;
	
	if ( twi_doCommand(&cb1) == SYSERR )
		goto errorReturn;
	while ( twi_doCommand(&cb2) == OK && (status & DS2482_STATUS_1WB) && (poll_count++ < POLL_LIMIT))	{
		continue;
	}
	
	// check for failure due to poll limit reached
	if (poll_count >= POLL_LIMIT)
	{
	errorReturn:
		kprintf("OWReadByte Error: pollct=%d\n", poll_count);
		// handle error
		// ...
		DS2482_reset();
		return 0;
		
	}
	
	//	I2C_rep_start();
	//	I2C_write(I2C_address | I2C_WRITE, EXPECT_ACK);
	//	I2C_write(CMD_SRP, EXPECT_ACK);
	//	I2C_write(0xE1, EXPECT_ACK);
	//	I2C_rep_start();
	//	I2C_write(I2C_address | I2C_READ, EXPECT_ACK);
	//	data =  I2C_read(NACK);
	//	I2C_stop();
	OWrb[0] = DS2482_CMD_SRP;					//1-Wire Set Read Pointer command
	OWrb[1] = DS2482_SELECT_READDATAREGISTER;	//read data register into 'status'
//	cb1.slarw = DS2482_SLAVEADDRESS<<1|TW_WRITE;
//	cb1.data = OWrb;
	cb1.dataLength = 2;
//	cb1.chain = &cb2;
//	cb2.slarw = DS2482_SLAVEADDRESS<<1|TW_READ;
//	cb2.data = &status;
//	cb2.dataLength = 1;
//	cb2.chain = (struct twi_Command *)0;
	twi_doCommand(&cb1);
//	printf("OWReadByte %02x\n", status);
	return status;
}
