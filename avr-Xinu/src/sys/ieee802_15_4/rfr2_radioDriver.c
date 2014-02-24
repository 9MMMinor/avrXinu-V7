//
//  rfr2_radioDriver.c
//  radio-15-4
//
//  Created by Michael M Minor on 11/4/13.
//
//

/*
 * avr-Configuration entry for
 * Radio using rfr2 chip:
 ********************************************************************************
 * radio:																		*
 *		on RFR2		-i radioInit	-o ioerr		-c ioerr					*
 *					-r radioRead	-w radioWrite	-s ioerr					*
 *					-n radioCntl	-g ioerr		-p ioerr					*
 *					-iint radioIInt	-oint radioOInt								*
 * %																			*
 * Physical 802.15.4 raw packet interface										*
 *																				*
 * RADIO is radio on RFR2	ivec=TRX24_RX_END_vect	ovec=TRX24_TX_END_vect		*
 *								                								*
 ********************************************************************************
 */

/*
 ********************************************************************************
 *	The normal driver state is with the transceiver in PLL_ON state.
 *	When there is a Read pending, as there almost always is when network software
 *	is active, the transceiver will be in RX_ON or RX_AACK_ON state depending on
 *	the driver mode, BASIC or EXTENDED. A write must switch to PLL_ON (BASIC
 *	mode) or to the TX_ARET_ON state (EXTENDED mode) and initiate the frame
 *	transmission. When the transmission is done, the radioWrite() function
 *	must restore the state to PLL_ON, RX_ON, or RX_AACK_ON.
 ********************************************************************************
 */ 

#include <avr-Xinu.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <registermap.h>
#include <rfr2_radioDriver.h>
#include <frame802154.h>
#include <macSymbolCounter.h>
#include <mib.h>
#include <radio.h>

#define __ASSERT_USE_STDERR
#include <assert.h>
#define ASSERT(e) assert(e)


#ifdef Nradio
struct rfDeviceControlBlock radio[Nradio];
#endif

int freebuf(int *);
int resched(void);
int ready(int, int);


#ifdef DEBUG
void packet_Dump(char * routine, struct epacket *epkptr, int len);
void packet_DataDump(char * routine, uint8_t *p, int len);
void packet_HeaderDump(char * routine, struct epacket *epkptr, int len);
int printable(char ch);
#endif

INTPROC cancelRead(void *);


/*
 *------------------------------------------------------------------------
 *  radioInit  -  initialize rfr2 and buffers
 *		called from initialize (process 0) - no sleeping or suspending
 *		allowed.
 *------------------------------------------------------------------------
 */

DEVCALL radioInit(struct devsw *devptr)
{
	struct rfDeviceControlBlock *rfPtr;
	
	rfPtr = &radio[devptr->dvminor];
	rfPtr->rfDevice = devptr;
	rfPtr->readSemaphore = screate(1);
	rfPtr->writeSemaphore = screate(1);
	rfPtr->readPid = 0;
	rfPtr->writePid = 0;
	rfPtr->TX_saveState = 0;
	rfPtr->transactionStatus = 0;
	rfPtr->writeRetrys = 0;	/* NO retry for now */
	rfPtr->operatingMode = BASIC;
	rfPtr->freeTXFrame = 0;			/* FALSE - free() TX frame				*/
	ANT_div.ANT_DIV_En = 0;			/* Antenna Diversity algorithm disabled */
	ANT_div.ANT_EXT_SW_En = 0;		/* Disable Antenna Switch */
//	ANT_div.ANT_Sel = 1;			/* 2->Antenna0, 1->Antenna1 */
	DDRG |= (1<<DIG1);			/* Ceramic Antenna on the Xplain Pro Board: */
	DDRF |= (1<<DIG2);			/* must be selected							*/
	PORTG |= (1<<DIG1);			/* DIG1 = PG1 = 1 */
	PORTF &= ~(1<<DIG2);		/* DIG2 = PF2 = 0 */
	radio_init();				/* initializes low-level 802.15.4 PHY + MAC */
	
	IRQ_status.TX_End = 1;					/* clears TX_End bit   */
	IRQ_mask.TX_END_Enable = 1;				/* enable TX interrupt */
	
	ASSERT(TRX_status.TRX_Status == STATUS_TRX_OFF);
		
	return(OK);
}

/*
 *------------------------------------------------------------------------
 *  radioCntl  -  control a 802-15-4 device by setting modes and/or
 *		performing command.
 *------------------------------------------------------------------------
 */

int radioCntl(struct devsw *devptr, int func)
{
	STATWORD ps;
	struct rfDeviceControlBlock *rfPtr;
	
	rfPtr = &radio[devptr->dvminor];
	disable(ps);
	switch ( func )	{

		case RADIO_RESET:
			/**	This function will reset all the registers and the state machine of
			 *  the radio transceiver.
			 */
			radio_reset_trx();
			break;

		case RADIO_SET_BASIC_OPERATING_MODE:
			/** This function sets the transciever to basic operating mode.
			 */
			rfPtr->operatingMode = BASIC;
			break;

		case RADIO_SET_EXTENDED_OPERATING_MODE:
			/** This function sets the transciever to extended operating mode.
			 */
			rfPtr->operatingMode = EXTENDED;
			break;
			
		case RADIO_SET_FREE_TX_FRAME:
			/** This function forces the driver to free() the TX frame
			 *	after transmission.
			 */
			rfPtr->freeTXFrame = TRUE;
			
		case RADIO_CLEAR_FREE_TX_FRAME:
			/** This function forces the driver to free() the TX frame
			 *	after transmission.
			 */
			rfPtr->freeTXFrame = FALSE;

		default:
			restore(ps);
			return(SYSERR);
	}
	restore(ps);
	return(OK);
}

/*
 *------------------------------------------------------------------------
 *  radioRead - read a single frame from the radio interface and UNPACK
 *		read(RADIO, buff, len);  comes here
 *------------------------------------------------------------------------
 */

DEVCALL radioRead(struct devsw *devptr, frame802154_t *frame, int len)
{
	STATWORD ps;
	struct rfDeviceControlBlock *rfPtr;
	radio_status_t status;
	int ret;
	
	rfPtr = &radio[devptr->dvminor];
	wait(rfPtr->readSemaphore);		/* exclude other threads from using read */
	disable(ps);
	if ( (rfPtr->operatingMode == BASIC) && ((status = radio_set_trx_state(CMD_RX_ON)) != RADIO_SUCCESS))	{
		rfPtr->errorCode = RADIO_WRONG_STATE;
		kprintf("radioRead status = %x\n", status);
		restore(ps);
		return(SYSERR);
	} else if ( (rfPtr->operatingMode == EXTENDED) && ((status = radio_set_trx_state(CMD_RX_AACK_ON)) != RADIO_SUCCESS))	{
		rfPtr->errorCode = RADIO_WRONG_STATE;
		kprintf("radioRead status = %x\n", status);
		restore(ps);
		return(SYSERR);
	}
	rfPtr->RX_frame = frame;

//	frame->lqi = 0;
//	frame->crc = FALSE;
	IRQ_status.RX_End = 1;					/* clears RX_End bit   */
	IRQ_mask.RX_END_Enable = 1;				/* enable RX interrupt */
	
//	ASSERT(TRX_status.TRX_Status == STATUS_RX_ON);
	
	/* set a time-out */
	tmset(timerPortID, rfPtr, TIME_OUT_TIME, &cancelRead);
	
	suspend( (rfPtr->readPid = currpid) );
	/* resume from radioIInt() */
	if (rfPtr->errorCode == RADIO_TIMEOUT)	{
		rfPtr->errorCode = 0;
		ret = RADIO_TIMEOUT;
	} else	{
		tmclear(timerPortID, rfPtr);	/* clear time-out */
		ret = rfPtr->RX_frame->header_len + rfPtr->RX_frame->data_len;
	}
	signal(rfPtr->readSemaphore);
	
	restore(ps);
	return (ret);	/* actual Received Frame Length */
}

/**
 * cancelRead() -- callback from timer TIMEOUT interrupt
 */

INTPROC cancelRead(void *p)
{
	struct rfDeviceControlBlock *rfPtr = p;
	
	IRQ_status.RX_End = 1;					/* clears RX_End bit   */
	IRQ_mask.RX_END_Enable = 0;				/* disable RX interrupt */

	rfPtr->errorCode = RADIO_TIMEOUT;
	resume(rfPtr->readPid);
}

/**
 * Input (RX_END) interrupt service routine
 *		reached from confisr.c
 */

INTPROC radioIInt(struct rfDeviceControlBlock *rfPtr)
{


	if (isbadpid(rfPtr->readPid)) {
		return;							/* no read pending */
	}
	
	unpackRXFrame(rfPtr->RX_frame);
	/* add lqi and crc to the end of the frame */
//	rfPtr->RX_frame->lqi = PHY_rssi.Rssi;		/* LQI can be much more sophisticated! */
//	rfPtr->RX_frame->crc = PHY_rssi.CRC_Valid;	/* CRC flag */

	IRQ_mask.RX_END_Enable = 0;					/* disable RX interrupt */
	ready(rfPtr->readPid, RESCHYES);
}


/*
 *------------------------------------------------------------------------
 *  radioWrite - write a single frame to the transmit buffer and send it
 *------------------------------------------------------------------------
 */

DEVCALL radioWrite(struct devsw *devptr, frame802154_t *frame, int dataLen)
{
	struct rfDeviceControlBlock *rfPtr;
	radio_status_t status = 0;
	STATWORD ps;
	
	rfPtr = &radio[devptr->dvminor];
	/* it's OK to write less than all the data, but not less than a full header */
	if (dataLen > MAX_FRAME_LENGTH || dataLen < frame->header_len)	{
		rfPtr->errorCode = RADIO_INVALID_ARGUMENT;
		return (SYSERR);
	}
	
	wait(rfPtr->writeSemaphore);	/* wait (possibly) for previous TX to finish  */
	
	disable(ps);
		
	rfPtr->TX_saveState = TRX_status.TRX_Status;	/* save state */
	
	if ( (rfPtr->operatingMode == BASIC) && ((status = radio_set_trx_state(CMD_PLL_ON)) != RADIO_SUCCESS))	{
		rfPtr->errorCode = RADIO_WRONG_STATE;
		restore(ps);
		return(SYSERR);
	} else if ( (rfPtr->operatingMode == EXTENDED) && ((status = radio_set_trx_state(CMD_TX_ARET_ON)) != RADIO_SUCCESS))	{
		rfPtr->errorCode = RADIO_WRONG_STATE;
		restore(ps);
		return(SYSERR);
	}
	
	rfPtr->writePid = currpid;
	
	IRQ_status.TX_End = 1;					/* clears TX_End bit   */
	IRQ_mask.TX_END_Enable = 1;				/* enable TX interrupt */

	/* you can change the order of the following two statements */
	packTXFrame(frame);
	TRX_state.TRX_Cmd = CMD_TX_START;

	suspend( (rfPtr->writePid = currpid) );

	radio_set_trx_state(rfPtr->TX_saveState);	/* restore state */
	signal(rfPtr->writeSemaphore);				/* signal TX done */
	restore(ps);
	return(dataLen);
}

//ISR(TRX24_TX_END_vect)
//{
//	radioOInt(&radio[0]);
//}

/*
 **************************SOMEDAY WILL****************************************
 *	The output interrupt assumes that the address it was passed is that of a
 *	buffer allocated from the buffer pool mechanism, and invokes freebuf()
 *	to return the buffer to its pool once output is complete.
 ******************************************************************************
 */

INTPROC	radioOInt(struct rfDeviceControlBlock *rfPtr)
{

	if (isbadpid(rfPtr->writePid)) {
		kprintf("Bad pid\n");
		rfPtr->transactionStatus = INVALID;
		return;
	}
	
//	freebuf( (int *)rfPtr->TX_frame );
	
	switch(TRX_status.TRX_Status)	{
        case SUCCESS:
        case SUCCESS_DATA_PENDING:
            rfPtr->transactionStatus = SUCCESS;
            break;
        case NO_ACK:
        case CHANNEL_ACCESS_FAILURE:
            rfPtr->transactionStatus = NO_ACK;
#if RADIOSTATS
			rfPtr->RADIO_TXfail++;
#endif
            break;
        case SUCCESS_WAIT_FOR_ACK:
            //  should only happen in RX mode
        case INVALID:
            //  should never happen here
        default:
#if RADIOSTATS
            rfPtr->RADIO_TXfail++;
#endif
			rfPtr->transactionStatus = INVALID;
            break;
	}
	/* upon return to write, switch state to rfPtr->TX_saveState */
	/* can't do it here because of the pause until saved state becomes effective */
	ready(rfPtr->writePid, RESCHYES);
	return;
}

