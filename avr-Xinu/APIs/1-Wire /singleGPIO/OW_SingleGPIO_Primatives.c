//
//  OW_SingleGPIO_Primatives.c
//  1-Wire_SingleGPIO
//
//  Created by Michael Minor on 10/29/14.
//
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
#include <avr/interrupt.h>
#include <util/delay.h>
#include <1-Wire_SingleGPIO.h>

/* Locals */
static uint8_t OW_status;
static uint8_t OW_state;
static uint16_t OW_pid;
static uint8_t ow_time_slot;

/*! \brief  1-Wire enable.
 *
 *  This function initializes the timer for reset
 *  and sets up the port pin used for the 1-wire bus.
 *  It is important to run this function before calling
 *  any of the other OW functions.
 *
 *  \note   The OW can be stopped by disabling the
 *          timer interrupt.
 */
void OW_Enable(void)
{
	//Tri-state communication pin.
	INITIALIZE_OW_PIN();
	
	OW_status = 0x00;
	OW_state = 0;
    //	CLEAR_OW_TIMER_ON_COMPARE_MATCH();
}

/*! \brief  Disable the 1-Wire.
 *
 *  This function disables the 1-Wire by disabling the timer.
 */
void OW_Disable(void)
{
 	OW_status = 0x00;
 	DISABLE_OW_TIMER_INTERRUPT();
 	STOP_OW_TIMER();
}

ISR(OW_RESET_INTERRUPT_VECTOR)
{
	static uint8_t presence;
	
	/* executing the ISR automatically clears the OCF0A Flag */
	STOP_OW_TIMER();
	
	switch ( OW_state++ )		{
		case 0:
			ow_time_slot = 0;
			CLEAR_OW_TIMER();
			CLEAR_OW_PIN();						// pull the DQ line low
			SET_OW_TIMER_COMPARE(RESET_DQ_INIT);
			stopclk();							// defer all context switching (enable premptive scheduling)
			break;
		case 1:			// Release DQ
			ow_time_slot += TCNT0;
			CLEAR_OW_TIMER();
			SET_OW_PIN();						// release DQ
			SET_OW_TIMER_COMPARE(RESET_SAMPLE);
			break;
		case 2:			// Sample DQ
			ow_time_slot += TCNT0;
			CLEAR_OW_TIMER();
			SET_OW_TIMER_COMPARE(RESET_SETTLE);
			presence = READ_OW_PIN();
			break;
		case 3:			// Release DQ
			ow_time_slot += TCNT0;
			CLEAR_OW_TIMER();
			DISABLE_OW_TIMER_INTERRUPT();
			strtclk();							// resume normal context switching
			send(OW_pid, presence);
			return;
	}
	START_OW_TIMER();
}

/*! \brief  Send a Reset signal and listen for Presence signal.
 *
 *  Generates the waveform for transmission of a Reset pulse on the
 *  1-Wire bus and listens for presence signals.
 *
 *  \return TRUE(1):  presence pulse(s) detected, device(s) reset
 *			FALSE(0): no presence pulses detected
 */
int OWReset(void)
{
	int rvalue;
	
	//	wait(OW_sem);				// wait for shared resource
	
	OW_pid = getpid();
	OW_state = 0;
	STOP_OW_TIMER();				//Stop timer to make sure prescaler is reset.
	CLEAR_OW_TIMER();
	SET_OW_TIMER_COMPARE(0);		//Cause immediate interrupt.
	CLEAR_OW_TIMER_INTERRUPT_FLAG();
	ENABLE_OW_TIMER_INTERRUPT();
	START_OW_TIMER();
	
	rvalue = receive();
	if ( ow_time_slot > 250 )	{
		printf("Bad RESET time_slot time = %d\n", ow_time_slot);
		return FALSE;
	}
	
	//	signal(OW_sem);				// shared resource available
	if (rvalue)
		return FALSE;
	return TRUE;
}

/*! \brief  Read a bit from the 1-Wire (DQ) pin.
 *
 *  Generates the waveform for reception of a bit on the 1-Wire bus.
 *
 *  \return The bit value of the 1-Wire DQ pin.
 */
unsigned char OWReadBit(void)
{
	STATWORD ps;
	unsigned char bit;
	
	disable(ps);
	CLEAR_OW_PIN();
	_delay_us(OW_DELAY_A_STD_MODE);
	SET_OW_PIN();
	_delay_us(OW_DELAY_E_STD_MODE);
	bit = READ_OW_PIN();
	_delay_us(OW_DELAY_F_STD_MODE);
	restore(ps);
	return ( bit );
}

/*! \brief  Write a bit to the 1-Wire (DQ) pin.
 *
 *  Generates the waveform for transmission of a bit on the 1-Wire
 *  bus.
 *
 *  \param  bitval    The bit value to write.
 */
void OWWriteBit(unsigned char bitval)
{
	STATWORD ps;
	
	disable(ps);
	if (bitval == 1)	{
		CLEAR_OW_PIN();
		_delay_us(OW_DELAY_A_STD_MODE);
		SET_OW_PIN();
		_delay_us(OW_DELAY_B_STD_MODE);
	}
	else	{
		CLEAR_OW_PIN();
		_delay_us(OW_DELAY_C_STD_MODE);
		SET_OW_PIN();
		_delay_us(OW_DELAY_D_STD_MODE);
	}
	restore(ps);
}

/*! \brief  Read one byte of data from the 1-Wire bus.
 *
 *	Read a complete byte of data on the 1-Wire bus, one bit at a time.
 *
 *  \return     The byte read from the bus.
 */
unsigned char OWReadByte(void)
{
	unsigned char i;
	unsigned char value = 0;
	for (i=0;i<8;i++)
	{
		if(OWReadBit()) value|=0x01<<i; // reads byte in, one byte at a time and then
		// shifts it left
		_delay_us(6); // wait for rest of timeslot
	}
	return(value);
}

/*! \brief  Write one byte of data on the 1-Wire bus.
 *
 *  Write a complete byte of data on the 1-Wire bus, one bit at a time.
 *
 *  \param  byte_value    The data to send on the bus.
 *
 */
void OWWriteByte(unsigned char val)
{
	unsigned char i;
	unsigned char temp;
	
	for (i=0; i<8; i++)     // writes byte, one bit at a time
	{
		temp = val>>i;      // shifts val right 'i' spaces
		temp &= 0x01;       // copy that bit to temp
		OWWriteBit(temp);    // write bit in temp into
	}
	_delay_us(5);
}
