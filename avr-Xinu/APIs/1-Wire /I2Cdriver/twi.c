//
//  twi.c
//  TWIDriver
//
//  Created by Michael Minor on 12/28/11.
//  Copyright (c) 2011 Michael M Minor. All rights reserved.
//

/* avr-xinu */
#include <avr-Xinu.h>	/* includes avr-libc <stdio.h>	*/
//#include <bufpool.h>


/* avr-libc */
#include <stdlib.h>
#include <avr/interrupt.h>
#include <util/twi.h>

#include <twi.h>

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

volatile uint8_t twi_internal_state;
static volatile uint8_t twi_state;
static int twi_pid;
static int twi_pool_id;
struct twi_Command *twi_Command;
static volatile uint8_t twi_error;

static void twi_StopOrChain(void);


void twi_init(void)
{
	// initialize state
	twi_state = TWI_READY;

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega8__) || defined(__AVR_ATmega328P__)
    // activate internal pull-ups for twi
    // as per note from atmega8 manual pg167
    sbi(PORTC, 4);
    sbi(PORTC, 5);
#elif defined(_AVR_IOM128_H_)
    // activate internal pull-ups for twi
    // as per note from atmega128 manual pg204
    sbi(PORTD, 0);
    sbi(PORTD, 1);
#elif defined(_AVR_IOM1284P_H_)
	// activate internal pull-ups for twi
    // See ATmega1284p manual section 19.5.1, pg213.
    sbi(PORTC, 0);	//SCL - needed on AVR-P40-USB board
//    sbi(PORTC, 1);	//SDA - pulled up on AVR-P40-USB board
#else
#error "TWI Port Pins may require pull-ups"
#endif
	
	// initialize twi prescaler and bit rate
	cbi(TWSR, TWPS0);
	cbi(TWSR, TWPS1);
	TWDR = 0xff;
	TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;
	
	/* twi bit rate formula from atmega1284p manual pg 214
	 SCL Frequency = CPU Clock Frequency / (16 + (2 * TWBR))
	 note: TWBR should be 10 or higher for master mode
	 It is 72 for a 16mhz Wiring board with 100kHz TWI */
	
	// enable twi module and twi interrupt
	TWCR = _BV(TWEN) | _BV(TWIE);
	
	// allocate buffers
	
	//Here is where we can use bpool routines:
	twi_pool_id = mkpool(TWI_BUFFER_LENGTH, 5);

//	twi_masterBuffer = (uint8_t*) getbuf(twi_pool_id);
//	twi_txBuffer = (uint8_t*) getbuf(twi_pool_id);
//	twi_rxBuffer = (uint8_t*) getbuf(twi_pool_id);
}

int twi_doCommand( struct twi_Command *twiCB )
{
	int rvalue;
	
//	wait(twi_sem);			// wait for shared resource

	twi_pid = getpid();
	twi_Command = twiCB;	// copy command block pointer to global pointer.
	
	TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTA);	// Initiate a START condition
	rvalue = receive();			// could be receive or time-out:
	//	rvalue = recvtim(10);	// wait 10 TICKS
	//	if (rvalue == TIMEOUT)
	//		{
	//		something else;
	//		signal(twi_sem);
	//		return (SOMEERROR);
	//		}
//	signal(twi_sem);		// shared resource available
	return (rvalue);
}

/*
 *--------------------------------------------------------------------------------
 * twi_StopOrChain -- Stop or chain next command block.
 *--------------------------------------------------------------------------------
 */

static void twi_StopOrChain(void)
{
	if (twi_Command->chain)	{
		twi_Command = twi_Command->chain;
		TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWSTA);	// Restart
	}
	else	{
		TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWSTO);
		// release I2C bus
		// wait for stop condition to be exectued on bus
		// TWINT is not set after a stop condition!
		while(TWCR & _BV(TWSTO)){
			continue;
		}
		twi_state = TWI_READY;
		// notify the upper half with OK message
		if (twi_pid)
			send(twi_pid, OK);
	}
}

SIGNAL(TWI_vect)
{
	static volatile uint8_t *bufNext;
	static volatile uint8_t bufCount;
	
	switch ( (twi_internal_state = TWSR) )
	{
		case TW_START:								// START has been transmitted
		case TW_REP_START:							// Repeated START has been transmitted
			bufNext = twi_Command->data;
			bufCount = twi_Command->dataLength;
			TWDR = twi_Command->slarw;
			TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA);
			break;
		case TW_MT_SLA_ACK:					// SLA+W has been tramsmitted and ACK received
		case TW_MT_DATA_ACK:				// Data byte has been tramsmitted and ACK received
			if (bufCount-- > 0)
			{
				TWDR = *bufNext++;
				TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT); 
			}
			else
			{
				twi_StopOrChain();
			}
			break;
		case TW_MR_DATA_ACK:			// Data byte has been received and ACK tramsmitted
			*bufNext++ = TWDR;
		case TW_MR_SLA_ACK:				// SLA+R has been tramsmitted and ACK received
			if ( --bufCount > 0 )		// Send ACK
			{
				TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWEA);
			}
			else						// Send NACK
			{
				TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT);
			}    
			break; 
		case TW_MR_DATA_NACK:			// Data byte has been received and NACK tramsmitted
			*bufNext = TWDR;
			twi_StopOrChain();
			break;      
		case TW_MT_ARB_LOST:			// Arbitration lost - (Re)start
			TWCR = _BV(TWEN) | _BV(TWIE) | _BV(TWINT) | _BV(TWSTA);
			break;
		case TW_MT_SLA_NACK:		// SLA+W has been tramsmitted and NACK received
		case TW_MR_SLA_NACK:		// SLA+R has been tramsmitted and NACK received    
		case TW_MT_DATA_NACK:		// Data byte has been tramsmitted and NACK received
		case TW_BUS_ERROR:			// Bus error due to an illegal START or STOP condition
		default:
			// Reset TWI Interface
			TWCR = _BV(TWEN);
			twi_state = TWI_READY;
			// notify the upper half with SYSERR message
			if (twi_pid)
				send(twi_pid, SYSERR);
	}
}
