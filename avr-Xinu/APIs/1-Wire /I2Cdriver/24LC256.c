//
//  24LC256.c
//  TWIDriver
//
//	Microchip 24LC256 I2C CMOS Serial EEPROM
//
//  Created by Michael Minor on 12/27/11.
//  Copyright (c) 2011 Michael M Minor. All rights reserved.
//

#include "24LC256.h"
#include "twi.h"
#include <avr-Xinu.h>
#include <stdlib.h>
#include <util/twi.h>

//***********************************************************
//	read one or more bytes into data from the current address
//***********************************************************
int current_address_ReadEEPROM(uint8_t *data, uint8_t nbytes)
{
	struct twi_Command cb =
		TWI_SETUP_CMDBLOCK(EEPROM24LC256_ADDRESS, TW_READ, 0, 0, 0);
	
//	cb.slarw = EEPROM24LC256_ADDRESS<<1|TW_READ;
	cb.data = data;
	cb.dataLength = nbytes;
//	cb.chain = (struct twi_Command *)0;
	if ( twi_doCommand(&cb) == OK )
		return ((int)nbytes);
	return (SYSERR);
}

//**********************************************************
//	write one or more bytes from data to address
//**********************************************************
/* If a Page Write command attempts to write across a physical
 * page boundary, the result is that the data wraps around to
 * the beginning of the current page (overwriting data previously
 * stored there), instead of being written to the next page, as
 * might be expected.
 */
/*+++++++++++++++++ THIS DOES NOT WORK +++++++++++++++++++++++++
int WriteEEPROM(int address, uint8_t *data, uint8_t nbytes)
{
	struct twi_Command cb1, cb2;
	uint8_t addr_buf[3];

	addr_buf[0] = address>>8;		//address high byte
	addr_buf[1] = address&0xFF;		//address low byte
 	addr_buf[2] = *data++;
	cb1.slarw = EEPROM24LC256_ADDRESS<<1|TW_WRITE;
	cb1.data = addr_buf;
	cb1.dataLength = 3;
 	if (nbytes > 1)	{
		cb1.chain = &cb2;
		cb2.slarw = cb1.slarw;
		cb2.data = data;
		cb2.dataLength = nbytes-1;
		cb2.chain = (struct twi_Command *)0;
	}
	
	if ( twi_doCommand(&cb1) == OK )
		return((int)nbytes);
	return (SYSERR);
}
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


int WriteEEPROM(int address, uint8_t *data, uint8_t nbytes)
{
	struct twi_Command cb;
	uint8_t *buf, ret;
	
	buf = (uint8_t *)malloc(nbytes+2);
	buf[0] = address>>8;		//address high byte
	buf[1] = address&0xFF;		//address low byte
	memcpy(&buf[2], data, nbytes);
	cb.slarw = EEPROM24LC256_ADDRESS<<1|TW_WRITE;
	cb.data = buf;
	cb.dataLength = nbytes+2;
	cb.chain = (struct twi_Command *)0;
	
	ret = twi_doCommand(&cb);
	free(buf);
	if (ret == OK)
		return((int)nbytes);
	return (SYSERR);
}



//**********************************************************
//	read one or more bytes from address to data
//**********************************************************
int ReadEEPROM(int address, uint8_t *data, uint8_t nbytes)
{
	struct twi_Command cb1, cb2;
	uint8_t addr_buf[2];
	
	addr_buf[0] = address>>8;		//address high byte
	addr_buf[1] = address&0xFF;		//address low byte
	cb1.slarw = EEPROM24LC256_ADDRESS<<1|TW_WRITE;
	cb1.data = addr_buf;
	cb1.dataLength = 2;
	cb1.chain = &cb2;
	cb2.slarw = EEPROM24LC256_ADDRESS<<1|TW_READ;
	cb2.data = data;
	cb2.dataLength = nbytes;
	cb2.chain = (struct twi_Command *)0;
	
	if ( twi_doCommand(&cb1) == OK )
		return ((int)nbytes);
	
	return (SYSERR);
}

/*-----------------------------------------------------------------------
 *	testWriteBusyEEPROM - if write -> SYSERR && NACK, internal write is busy
 *-----------------------------------------------------------------------
 */

int testWriteBusyEEPROM(void)
{
	uint8_t status;
	struct twi_Command cb =
			TWI_SETUP_CMDBLOCK(EEPROM24LC256_ADDRESS, TW_WRITE, &status, 0, 0);
	
	if (twi_doCommand(&cb) == SYSERR && twi_internal_state == TW_MT_SLA_NACK)
		return (TRUE);
	return (FALSE);
}

