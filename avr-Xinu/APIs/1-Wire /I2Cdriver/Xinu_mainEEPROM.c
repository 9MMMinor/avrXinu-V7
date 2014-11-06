//
//  Xinu_mainEEPROM.c
//  TWIDriver
//
//  Created by Michael Minor on 12/28/11.
//  Copyright (c) 2011 Michael M Minor. All rights reserved.
//

#include "twi.h"
#include <util/twi.h>	// includes <avr/io.h>
#include "24LC256.h"


/* avr-xinu */
#include <avr-Xinu.h>	/* includes avr-libc <stdio.h>	*/
#include <stdlib.h>
#include <string.h>

void clear_PageEEPROM(int);

#define TWI_CMD_MASTER_WRITE 0xAA

int main( void )
{
	unsigned char messageBuf[4];
	uint8_t myCounter, len;
	int memory, ret;
	unsigned char TWI_targetSlaveAddress;
	unsigned char *command = (unsigned char *)getmem(128);
	char *prompt = "%";
	
	twi_init();
	TWI_targetSlaveAddress   = EEPROM24LC256_ADDRESS;
	myCounter = 0;
	memory = 0;
	
	for (;;)
	{
		write(CONSOLE, (unsigned char *)prompt, 1);
		len = read(CONSOLE, command, 128);
		
		if ( strncmp((char *)command, "test", 2) == 0 )
		{
			printf("EEPROM 24LC256 test. Internal write is %s\n", testWriteBusyEEPROM()?"busy.":"not busy.");
		}
		else if ( strncmp((char *)command, "clear", 2) == 0)
		{
			printf("Clear page (64 bytes) command\n");
			clear_PageEEPROM(memory);
		}
		else if ( strncmp((char *)command, "echo", 3) == 0)
		{
			uint8_t waitTick = 0;
			printf("Write: %d\n", WriteEEPROM(64, command, len));
			while (testWriteBusyEEPROM())
			{
				waitTick++;
			}
			printf("Write done. %d waits, but NO BUSY WAITS!\n", waitTick);
			printf("Read: %d\n", ReadEEPROM(64, command, len));
			command[len] = '\0';
			printf("Echo: %s\n", command);
		}
		else if ( strncmp((char *)command, "write", 2) == 0)
		{
			messageBuf[0] = TWI_CMD_MASTER_WRITE;
			messageBuf[1] = myCounter;
			if ( (ret = WriteEEPROM(memory, messageBuf, 2) ) == 2)	{
				printf("Write 0X%02x: 0X%02x 0X%02x\n", memory, messageBuf[0], messageBuf[1]);
			}
			else	{
				printf("Write SYSERR (%d)\n", ret);
			}
		}
		else if ( strncmp((char *)command, "read", 2) == 0)
		{
			ret = ReadEEPROM(memory, messageBuf, 2);
			if (ret == 2)	{
				printf("Read 0X%02x: 0X%02x 0X%02x\n", memory, messageBuf[0], messageBuf[1]);
				memory += 2;
			}
			else	{
				printf("Read SYSERR (%d)\n", ret);
			}
		}
		else if ( strncmp((char *)command, "next", 2) == 0)
		{
			ret = current_address_ReadEEPROM(messageBuf, 1);
			if (ret == 1)	{
				printf("Next read 0X%02x: 0X%02x\n", memory++, messageBuf[0]);
			}
			else	{
				printf("Next read SYSERR (%d)\n", ret);
			}
		}

		else if ( strncmp((char *)command, "increment", 2) == 0)
		{
			printf("Increment command: myCounter=%d\n", ++myCounter);
		}
		else if ( strncmp((char *)command, "set memory ", 11) == 0 )
		{
			sscanf((const char *)&command[11], "%d", &memory);
			printf("Set read/write memory to 0X%2x\n", memory);
		}
		else
			printf("Bad command\n");
		
	}
}

void clear_PageEEPROM(int address)
{
	struct twi_Command cb;
	uint8_t *buffer;
	
	buffer = (uint8_t *)calloc(66, sizeof(uint8_t));
	buffer[0] = address>>8;		//address high byte
	buffer[1] = address&0xFF;	//address low byte
	cb.slarw = EEPROM24LC256_ADDRESS<<1|TW_WRITE;
	cb.data = buffer;
	cb.dataLength = 66;
	cb.chain = (struct twi_Command *)0;
	
	printf("clear return: %d\n", twi_doCommand(&cb));
	free(buffer);
}
