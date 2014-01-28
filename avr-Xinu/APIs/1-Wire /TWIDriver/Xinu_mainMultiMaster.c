//
//  Xinu_mainMultiMaster.c
//  TWIDriver
//
//  Created by Michael Minor on 12/9/11.
//  Copyright 2011. All rights reserved.
//

/*****************************************************************************
 *
 * Atmel Corporation
 *
 * File              : main.c
 * Compiler          : IAR EWAAVR 2.28a/3.10c
 * Revision          : $Revision: 1.13 $
 * Date              : $Date: 24. mai 2004 11:31:20 $
 * Updated by        : $Author: ltwa $
 *
 * Support mail      : avr@atmel.com
 *
 * Supported devices : All devices with a TWI module can be used.
 *                     The example is written for the ATmega16
 *
 * AppNote           : AVR315 - TWI Master Implementation
 *
 * Description       : Example of how to use the driver for TWI master 
 *                     communication.
 *                     This code reads PORTD and sends the status to the TWI slave.
 *                     Then it reads data from the slave and puts the data on PORTB.
 *                     To run the example use STK500 and connect PORTB to the LEDS,
 *                     and PORTD to the switches. .
 *
 ****************************************************************************/

#include "ArduinoTwi.h"
#include <avr/io.h>


/* avr-xinu */
#include <avr-Xinu.h>	/* includes avr-libc <stdio.h>	*/
#include <date.h>

#define TWI_GEN_CALL         0x00  // The General Call address is 0

// Sample TWI transmission commands
#define TWI_CMD_MASTER_WRITE 0x10
#define TWI_CMD_MASTER_READ  0x20

// Sample TWI transmission states, used in the main application.
#define SEND_DATA             0x01
#define REQUEST_DATA          0x02
#define READ_DATA_FROM_BUFFER 0x03

//unsigned char TWI_Act_On_Failure_In_Last_Transmission ( unsigned char TWIerrorMsg )
//{
	// A failure has occurred, use TWIerrorMsg to determine the nature of the failure
	// and take appropriate actions.
	// Se header file for a list of possible failures messages.
	
	// Here is a simple sample, where if received a NACK on the slave address,
	// then a retransmission will be initiated.
	
//	if ( (TWIerrorMsg == TWI_MTX_ADR_NACK) | (TWIerrorMsg == TWI_MRX_ADR_NACK) )
//		TWI_Start_Transceiver();
    
//	return TWIerrorMsg; 
//}


int main( void )
{
	unsigned char messageBuf[4];
	uint8_t status;
	unsigned char TWI_targetSlaveAddress, myCounter=0;
	char *atime;
	int32_t now;
	unsigned char *command = (unsigned char *)getmem(128);
	char *prompt = "%";
	
	twi_init();
	TWI_targetSlaveAddress   = 0x10;
	
	for (;;)
	{
		write(CONSOLE, (unsigned char *)prompt, 1);
		read(CONSOLE, command, 128);
		if ( strncmp((char *)command, "test", 2) == 0 )
			{
			printf("Test in progress\n");
			}
		else if ( strncmp((char *)command, "clear", 2) == 0)
			{
			myCounter = 0;
			printf("Clear command\n");
			}
		else if ( strncmp((char *)command, "general", 3) == 0)
			{
			messageBuf[0] = 0xAA;             // The command or data to be included in the general call.
			twi_writeTo(TWI_GEN_CALL, messageBuf, 1);
			printf("General Call command\n");
			}
		else if ( strncmp((char *)command, "write", 2) == 0)
			{
			messageBuf[0] = TWI_CMD_MASTER_WRITE;             // The first byte is used for commands.
			messageBuf[1] = myCounter;                        // The second byte is used for the data.
			twi_writeTo(TWI_targetSlaveAddress, messageBuf, 2 );
			printf("Address Call command\n");
			}
		else if ( strncmp((char *)command, "read", 2) == 0)
			{
			// Send the request-for-data command to the Slave
			messageBuf[0] = TWI_CMD_MASTER_READ;             // The first byte is used for commands.
			status = twi_writeTo(TWI_targetSlaveAddress, messageBuf, 2 );
			status = twi_readFrom(TWI_targetSlaveAddress, messageBuf, 2 );
			
			// Get status from Transceiver
			printf("Read Command: status=%x\n", status);
			}
		else if ( strncmp((char *)command, "increment", 2) == 0)
			{
			++myCounter;
			printf("Increment command\n");
			}
		else if ( strncmp((char *)command, "time", 4) == 0 )
			{
//			atime = (char *)getmem(26);
//			gettime(&now);
//			ascdate(now, atime);
//			printf("%s\n", atime);
//			freemem(atime, 26);
			}
		else
			printf("Bad command\n");
		
	}
}

