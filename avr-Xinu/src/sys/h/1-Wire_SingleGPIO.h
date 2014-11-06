//
//  1-Wire_SingleGPIO.h
//  1-Wire_SingleGPIO
//
//  Created by Michael Minor on 10/8/14.
//
//

#ifndef __1Wire_SingleGPIO_h
#define __1Wire_SingleGPIO_h

#include <avr/io.h>

/* global search state */
extern unsigned char ROM_NO[8];
extern int LastDiscrepancy;
extern int LastFamilyDiscrepancy;
extern int LastDeviceFlag;
extern unsigned char crc8;

/* prescale = 8, 64 */
#define PRESCALE8		(2L*(8L*1000000L)/F_CPU)
#define PRESCALE64		(2L*(64L*1000000L)/F_CPU)

/* Port and pin settings. */
#define OW_PIN_NUMBER    0		//!< Set pin number for 1-Wire DQ.
#define DQ	OW_PIN_NUMBER		//!< Dallas nomenclature.
#define OW_PORT          PORTD	//!< Set port for 1-Wire DQ.
#define OW_PIN           PIND	//!< Set pin for 1-Wire DQ.
#define OW_DDR           DDRD	//!< Data direction register.

// Internal/External Pullup Resistor (~4.7K)
#define OW_USE_INTERNAL_PULLUP	// Set 1-Wire pin to input and enable internal pull-up resistor.

struct TC_0_ControlRegB	{
	uint8_t clockSel:3;
	uint8_t wgm02:1;
	uint8_t reserved:2;
	uint8_t forceOCB:1;
	uint8_t forceOCA:1;
};
#define TC_0_ControlRegB (*((volatile struct TC_0_ControlRegB*)&TCCR0B)) //!< Define the timer control register

#define STOP_OW_TIMER()		(TC_0_ControlRegB.clockSel = 0)
#define START_OW_TIMER()	(TC_0_ControlRegB.clockSel = 3)

/****************************************************************************
 *	Macros
 * \li Application Note:   AVR318 - Dallas 1-Wire(R) master.
 *
 *
 * \li Description:        Header file for OWIBitFunctions.c
 ****************************************************************************/

/*! \brief Pull 1-Wire bus low.
 *
 *  This macro sets the direction of the 1-Wire pin (DQ) to output and
 *  pulls the OW_PIN_NUMBER-pin low on port, OW_PORT.
 *
 */
#define OW_PULL_BUS_LOW() \
		do	{ \
			OW_DDR |= (1<<OW_PIN_NUMBER); \
			OW_PORT &= ~(1<<OW_PIN_NUMBER); \
			}	while (0)

/*! \def    OWI_RELEASE_BUS()
 *
 *  \brief  Release the bus.
 *
 *  This macro releases DQ and enables the internal pull-up if
 *  it is used.
 *
 */
#ifdef OW_USE_INTERNAL_PULLUP
// Set 1-Wire pin(s) to input and enable internal pull-up resistor.
#define OW_RELEASE_BUS() \
		do	{ \
			OW_DDR &= ~(1<<OW_PIN_NUMBER); \
			OW_PORT |= (1<<OW_PIN_NUMBER); \
			}	while (0)

#else
// Set 1-Wire pin(s) to input mode. No internal pull-up enabled.
#define OW_RELEASE_BUS() \
		do	{ \
			OW_DDR &= ~(1<<OW_PIN_NUMBER); \
			OW_PORT &= ~(1<<OW_PIN_NUMBER); \
			}	while (0)

#endif

/*! \brief Read 1-Wire pin.
 *
 *  This macro reads the 1-Wire pin (DQ) on port, OW_PORT.
 *
 */
#define OW_READ_BUS_PIN()	( OW_PIN & (1<<OW_PIN_NUMBER) )

/* Pin macros.  */
#define INITIALIZE_OW_PIN() OW_RELEASE_BUS()
#define READ_OW_PIN()	OW_READ_BUS_PIN()

/* Macros for standard AVR ports. */
#define SET_OW_PIN() OW_RELEASE_BUS()
#define CLEAR_OW_PIN() OW_PULL_BUS_LOW()

/****************************************************************************/

#define OW_RESET_INTERRUPT_VECTOR  TIMER0_COMPA_vect      //!< OW compare interrupt vector.

/* Timer macros. These are device dependent. */
#define CLEAR_OW_TIMER_ON_COMPARE_MATCH()     (TCCR0A |= (1<<WGM01))	//!< clear timer on compare match (CTC).
#define SET_OW_TIMER_COMPARE(x)  (OCR0A = (x)) //!< Sets the timer compare register to the number of microseconds to next operation.
#define CLEAR_OW_TIMER()                      (TCNT0 = 0x00)
#define ENABLE_OW_TIMER_INTERRUPT()           (TIMSK0 |= (1<<OCIE0A))
#define DISABLE_OW_TIMER_INTERRUPT()          (TIMSK0 &= ~(1<<OCIE0A))
#define CLEAR_OW_TIMER_INTERRUPT_FLAG()       (TIFR0 = (1<<OCF0A))

/*****************************************************************************
 *	Timing parameters
 *	APPLICATION NOTE 126: http://www.maximintegrated.com/an126
 *	1-Wire Communication Through Software
 *	May 30, 2002
 *****************************************************************************/
// Table 1. 1-Wire Operations
//
// Operation		Description								Implementation
// ---------		-----------------------------------		----------------------
//	Write 1 bit		Send a '1' bit to the 1-Wire slaves		Drive bus low, delay A
//					(Write 1 time slot)						Release bus, delay B
//
//	Write 0 bit		Send a '0' bit to the 1-Wire slaves		Drive bus low, delay C
//					(Write 0 time slot)						Release bus, delay D
//
//	Read bit		Read a bit from the 1-Wire slaves		Drive bus low, delay A
//					(Read time slot)						Release bus, delay E
//															Sample bus to read bit
//															from slave, Delay F
//
//	Reset			Reset the 1-Wire bus slave devices		Delay G
//					and ready them for a command			Drive bus low, delay H
//															Release bus, delay I
//															Sample bus, 0 = device(s) present,
//															1 = no device present
//															Delay J

// Standard Speed timing delays in usec.
#define     OW_DELAY_A_STD_MODE    (6  )
#define     OW_DELAY_B_STD_MODE    (64 )
#define     OW_DELAY_C_STD_MODE    (60 )
#define     OW_DELAY_D_STD_MODE    (10 )
#define     OW_DELAY_E_STD_MODE    (9  )
#define     OW_DELAY_F_STD_MODE    (55 )
#define     OW_DELAY_G_STD_MODE    (0  )
#define     OW_DELAY_H_STD_MODE    (480)
#define     OW_DELAY_I_STD_MODE    (70 )
#define     OW_DELAY_J_STD_MODE    (410)

#define RESET_DQ_INIT	2L*(OW_DELAY_H_STD_MODE-3)/PRESCALE64
#define RESET_SAMPLE	2L*(OW_DELAY_I_STD_MODE-3)/PRESCALE64
#define RESET_SETTLE	2L*(OW_DELAY_J_STD_MODE-3)/PRESCALE64

#endif
