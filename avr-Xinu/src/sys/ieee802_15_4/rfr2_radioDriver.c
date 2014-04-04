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
#include <frameIO.h>

#define __ASSERT_USE_STDERR
#include <assert.h>
#define ASSERT(e) assert(e)

extern uint32_t macAckWaitDuration;			/* needs to be in mib.h */

#ifdef Nradio
struct rfDeviceControlBlock radio[Nradio];
#endif
extern uint8_t radio_TIMER;
extern struct radioinfo Radio;
#define RADIO_TIMER &radio_TIMER

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
INTPROC setNoAck(void *);
void sendAckFrame(struct rfDeviceControlBlock *);


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
	rfPtr->IRQ_save = 0;
	rfPtr->transactionStatus = 0;
	rfPtr->writeRetrys = 0;	/* NO retry for now */
	rfPtr->operatingMode = BASIC;
	rfPtr->freeTXFrame = 0;			/* FALSE - free() TX frame				*/
	rfPtr->doRXTimeout = FALSE;		/* FALSE - enable RX cancel				*/
	rfPtr->ackTXPending = FALSE;		/* no BASIC mode TX ack pending			*/
//	rfPtr->ackTXFrame.fcf.frameType = FRAME_TYPE_ACK;	/* initialize ackTXFrame	*/
//	rfPtr->ackTXFrame.fcf.frameSecurity = 0;
//	rfPtr->ackTXFrame.fcf.frameAckRequested = 0;		/* don't ack the ack */
//	rfPtr->ackTXFrame.fcf.frameVersion = FRAME_VERSION_2006;
//	rfPtr->ackTXFrame.fcf.frameDestinationAddressMode = 0;
//	rfPtr->ackTXFrame.fcf.frameSourceAddressMode = 0;
//	rfPtr->ackTXFrame.seq = 1;
	rfPtr->ackTXSeq = 0;
	rfPtr->ackRXPending = FALSE;	/* no BASIC mode RX ack pending			*/
//	rfPtr->ackRXSeq = 0;
	ANT_div.ANT_DIV_En = 0;			/* Antenna Diversity algorithm disabled */
	ANT_div.ANT_EXT_SW_En = 0;		/* Disable Antenna Switch */
//	ANT_div.ANT_Sel = 1;			/* 2->Antenna0, 1->Antenna1 */
	DDRG |= (1<<DIG1);			/* Ceramic Antenna on the Xplain Pro Board: */
	DDRF |= (1<<DIG2);			/* must be selected							*/
	PORTG |= (1<<DIG1);			/* DIG1 = PG1 = 1 */
	PORTF &= ~(1<<DIG2);		/* DIG2 = PF2 = 0 */
//	radio_init();				/* initializes low-level 802.15.4 PHY + MAC */
	
//	IRQ_status.TX_End = 1;					/* clears TX_End bit   */
//	IRQ_mask.TX_END_Enable = 1;				/* enable TX interrupt */
	
//	ASSERT(TRX_status.TRX_Status == STATUS_TRX_OFF);
	kprintf("radioInit\n");
		
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
			break;
			
		case RADIO_CLEAR_FREE_TX_FRAME:
			/** This function forces the driver to free() the TX frame
			 *	after transmission.
			 */
			rfPtr->freeTXFrame = FALSE;
			break;
			
		case RADIO_SET_READ_WITH_TIMEOUT:
			/** This enables driver to cancel a read request after TIMEOUT_TIME
			 *	seconds.
			 */
			rfPtr->doRXTimeout = TRUE;
			break;
			
		case RADIO_CLEAR_READ_WITH_TIMEOUT:
			/** This disables driver to cancel a read request after TIMEOUT_TIME
			 *	seconds (read() blocks until a frame is received).
			 */
			rfPtr->doRXTimeout = FALSE;
			break;

		default:
			restore(ps);
			return(SYSERR);
	}
	restore(ps);
	return(OK);
}

/*
 *------------------------------------------------------------------------
 *		read(RADIO, buff, len);  comes here
 *  radioRead - read a single frame from the radio interface and UNPACK
 *		send an Ack Frame if it was requested.
 *------------------------------------------------------------------------
 */

DEVCALL radioRead(struct devsw *devptr, frame802154_t *frame, int len)
{
	STATWORD ps;
	struct rfDeviceControlBlock *rfPtr;
	radio_status_t status;
	uint8_t timer_event = 0;
	int ret;
	
	rfPtr = &radio[devptr->dvminor];
//	wait(rfPtr->readSemaphore);		/* exclude other threads from using read */
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
	
	/* set a time-out EVENT if we don't want to block forever */
	if (rfPtr->doRXTimeout)	{
		timer_event = MKEVENT(READ_WITH_TIMEOUT, devptr->dvminor);
		tmset(timerPortID, &timer_event, TIME_OUT_TIME, &cancelRead);
	}
	
	suspend( (rfPtr->readPid = currpid) );
	/* resume from radioIInt() */
	ret = rfPtr->RX_frame->header_len + rfPtr->RX_frame->data_len + FTR_LEN;
	if (rfPtr->doRXTimeout)	{
		if (rfPtr->errorCode == RADIO_TIMEOUT)	{
			kprintf("radioRead: RADIO_TIMEOUT\n");
			rfPtr->errorCode = 0;
			ret = RADIO_TIMEOUT;
		} else	{
			kprintf("radioRead: clear timer\n");
			tmclear(timerPortID, &timer_event);
		}
	}
	if (rfPtr->operatingMode == BASIC &&
			rfPtr->RX_frame->fcf.frameAckRequested &&
			ret != RADIO_TIMEOUT)	{
		/* send an ACK Frame out - wait only for TX_END */
		pauseSymbolTimes(RADIO_TIMER, 12);	/* standard 12 Symbol Times */
		radio_set_trx_state(CMD_PLL_ON);
		
		sendAckFrame(rfPtr);
	}

//	signal(rfPtr->readSemaphore);
	
	restore(ps);
	return (ret);	/* actual Received Frame Length */
}

/**
 * cancelRead() -- callback from timer TIMEOUT interrupt
 */

INTPROC cancelRead(void * timed_event)
{
	struct rfDeviceControlBlock *rfPtr = &radio[UNIT(*(uint8_t *)timed_event)];

	IRQ_status.RX_End = 1;					/* clears RX_End bit   */
	IRQ_mask.RX_END_Enable = 0;				/* disable RX interrupt */

	rfPtr->errorCode = RADIO_TIMEOUT;
	resume(rfPtr->readPid);
}

/**
 * Input (TRX24_RX_END) interrupt service routine
 *		reached from confisr.c
 */

INTPROC radioIInt(struct rfDeviceControlBlock *rfPtr)
{
	frame802154_t *frame = rfPtr->RX_frame;
//	octet_t *q = (octet_t *)&TRXFBST;
	
	if (rfPtr->ackRXPending)	{
		rfPtr->ackRXPending = FALSE;
//		kprintf("resume from ackRXPending\n");
//		ready(rfPtr->writePid, RESCHYES);
//		frameDump("radioIInt",q,TST_RX_LENGTH);
		resume(rfPtr->writePid);
		return;
	}

	if (isbadpid(rfPtr->readPid)) {
		return;							/* no read pending */
	}
	
	unpackRXFrame(frame);
	if (frame->fcf.frameAckRequested)	{
		rfPtr->ackTXSeq = frame->seq;
	}
	/* add lqi and crc to the end of the frame */
//	rfPtr->RX_frame->lqi = PHY_rssi.Rssi;		/* LQI can be much more sophisticated! */
//	rfPtr->RX_frame->crc = PHY_rssi.CRC_Valid;	/* CRC flag */

	IRQ_mask.RX_END_Enable = 0;					/* disable RX interrupt */
	ready(rfPtr->readPid, RESCHYES);
}

void
sendAckFrame(struct rfDeviceControlBlock *rfPtr)
{
	frame802154_t *frame = (frame802154_t *)getbuf(Radio.radiopool);	/* get a frame for the ack */
	uint16_t temp_pid;
	uint8_t temp_addr[8];
	
	memcpy(frame, rfPtr->RX_frame, MAX_HDR_LENGTH);		/* copy just the header */
	frame->fcf.frameType = FRAME_TYPE_ACK;				/* and modify			*/
	frame->fcf.frameAckRequested = 0;
	frame->data_len = 0;
	
	temp_pid = frame->dest_pid;							/* swap src and dest	*/
	frame->dest_pid = frame->src_pid;
	frame->src_pid = temp_pid;
	memcpy(temp_addr, frame->dest_addr, 8);
	memcpy(frame->dest_addr, frame->src_addr, 8);
	memcpy(frame->src_addr, temp_addr, 8);
	
	packTXFrame(frame);
	IRQ_status.TX_End = 1;					// clears TX_End bit
	IRQ_mask.TX_END_Enable = 1;				// enable TX interrupt
	rfPtr->ackTXPending = TRUE;
	
//	frameHeaderDump("sendAckFrame", frame, 40);
	
	TRX_state.TRX_Cmd = CMD_TX_START;
	suspend(currpid);			/* wait for TX_END with ackPending */
	freebuf((int *)frame);
}

/*
 *------------------------------------------------------------------------
 *	write(RADIO, frame, len) comes here
 *  radioWrite - write a single frame to the transmit buffer and send it.
 *		receive and Ack Frame if it's requested.
 *------------------------------------------------------------------------
 */

DEVCALL radioWrite(struct devsw *devptr, frame802154_t *frame, int dataLen)
{
	struct rfDeviceControlBlock *rfPtr;
	radio_status_t status = 0;
	int ret = dataLen;
	STATWORD ps;
	
	rfPtr = &radio[devptr->dvminor];
	/* it's OK to write less than all the data, but not less than a full header */
	if (dataLen > MAX_FRAME_LENGTH || dataLen < frame->header_len)	{
		kprintf("radioWrite: bad dataLen = %d\n", dataLen);
		rfPtr->errorCode = RADIO_INVALID_ARGUMENT;
		return (SYSERR);
	}
	
	wait(rfPtr->writeSemaphore);	/* wait (possibly) for previous TX to finish  */
	disable(ps);
	
	rfPtr->IRQ_save = IRQ_mask.RX_END_Enable;		/* save IRQ mask */
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
	
	rfPtr->TX_frame = frame;

//	kprintf("Write: 1st suspend\n");
	suspend( (rfPtr->writePid = currpid) );
	
	if ( (frame->fcf.frameAckRequested) && (rfPtr->operatingMode == BASIC) )	{
		/* we either get notified that an Ack frame was received	*/
		/* or we get RADIO_NO_ACK									*/
		rfPtr->ackRXPending = TRUE;
//		rfPtr->ackRXSeq = frame->seq;
		IRQ_status.RX_End = 1;					/* clears RX_End bit   */
		IRQ_mask.RX_END_Enable = 1;				/* enable RX interrupt */
		/* set an ack time-out EVENT */
		
		/*	5.1.6.4.2 Acknowledgment
		 A frame transmitted with the AR field set to request an acknowledgment, as defined in 5.2.1.1.4, shall be
		 acknowledged by the recipient. If the intended recipient correctly receives the frame, it shall generate and
		 send an acknowledgment frame containing the same DSN from the data or MAC command frame that is
		 being acknowledged.	The transmission of an acknowledgment frame in a nonbeacon-enabled PAN or in the CFP
		 shall commence macSIFSPeriod after the reception of the last symbol of the data or MAC command frame.
		 The transmission of an acknowledgment frame in the CAP shall commence either macSIFSPeriod after the
		 reception of the last symbol of the data or MAC command frame or at a backoff period boundary.
		 In the latter case, the transmission of an acknowledgment frame shall commence between macSIFSPeriod
		 and (macSIFSPeriod + aUnitBackoffPeriod) after the reception of the last symbol of the data or
		 MAC command frame. The message sequence chart in Figure 26 [ieee 2011] shows the scenario for transmitting
		 a single frame of data from an originator to a recipient with an acknowledgment requested.
		 
		 A device that sends a data or MAC command frame with its AR field set to acknowledgment requested shall
		 wait for at most macAckWaitDuration for the corresponding acknowledgment frame to be received.
		 */
		
		/* set-up to wait until we get an Ack frame or an Ack TIMEOUT */
		
		/*
		 * Received frames are passed to the frame filtering unit, refer to section "Frame Filtering"
		 * on page 58. If the content of the MAC addressing fields of a frame (refer to
		 * IEEE 802.15.4 section 7.2.1) matches to the expected addresses, which is further
		 * dependent on the addressing mode, an address match interrupt (TRX24_XAH_AMI) is
		 * issued, refer to "Interrupt Logic" on page 36. The expected address values are to be
		 * stored in the registers Short-Address, PAN-ID and IEEE-address. Frame filtering is
		 * available in Basic and Extended Operating Mode, refer to section "Frame Filtering" on
		 * page 58.
		 */

		status = radio_set_trx_state(CMD_RX_ON);
		/* static ????? */
		static uint8_t event = MKEVENT(WRITE_WITH_ACK, 0);
		rfPtr->errorCode = RADIO_SUCCESS;
		tmset(timerPortID, &event, 2*macAckWaitDuration, &setNoAck);	/* see mib.c */
		
		suspend(currpid); /* resume if ack received OR ack TIMEOUT */

		ackFrame_t *ack = (ackFrame_t *)&TRXFBST;
//		kprintf("ack received, ackRXSeq=%d\n",rfPtr->ackRXSeq);
		if (rfPtr->errorCode == RADIO_NO_ACK)	{
			ret = SYSERR;
			kprintf("TIMEOUT: No ack frame\n");
		}
		else if ( ack->fcf.frameType == FRAME_TYPE_ACK && frame->seq == ack->seq )		{	/* the right Ack! */
//			kprintf("Ack frame: len=%d\n", TST_RX_LENGTH);
			tmclear(timerPortID, &event);	/* clear time-out */
			ret = rfPtr->RX_frame->header_len + rfPtr->RX_frame->data_len + FTR_LEN;
		}
		else	{
			rfPtr->errorCode = RADIO_NO_ACK;
			ret = SYSERR;
			kprintf("Bad ack frame: len=%d\n", TST_RX_LENGTH);
		}
	}
	radio_set_trx_state(rfPtr->TX_saveState);	/* restore state */
	IRQ_mask.RX_END_Enable = rfPtr->IRQ_save;	/* restore mask  */
	signal(rfPtr->writeSemaphore);				/* signal TX done */
	restore(ps);
	return( ret );
}

//ISR(TRX24_TX_END_vect)
//{
//	radioOInt(&radio[0]);
//}

/*
 ******************************************************************************
 *  TRX24_TX_END_vect interrupt service routine.
 ******************************************************************************
 */

INTPROC	radioOInt(struct rfDeviceControlBlock *rfPtr)
{

	if (rfPtr->ackTXPending)	{
		rfPtr->ackTXPending = FALSE;
//		kprintf("resume from ackTXPending\n");
		ready(rfPtr->readPid, RESCHYES);
		return;
	}
	if (isbadpid(rfPtr->writePid)) {
		kprintf("Bad pid\n");
		rfPtr->transactionStatus = INVALID;
		return;
	}
	
	if (rfPtr->operatingMode == EXTENDED)		{
		switch(TRX_state.TRAC_Status)	{
        	case SUCCESS:							/* (RX_AACK, TX_ARET)	*/
        	case SUCCESS_DATA_PENDING:				/* (TX_ARET)			*/
            	rfPtr->transactionStatus = SUCCESS;
            	break;
        	case NO_ACK:							/* (TX_ARET)			*/
        	case CHANNEL_ACCESS_FAILURE:			/* (TX_ARET)			*/
            	rfPtr->transactionStatus = NO_ACK;
#if RADIOSTATS
				rfPtr->RADIO_TXfail++;
#endif
            	break;
        	case SUCCESS_WAIT_FOR_ACK:				/* (RX_AACK)			*/
            	//  should only happen in RX mode
        	case INVALID:							/* (RX_AACK, TX_ARET)	*/
            	//  should never happen here
        	default:
#if RADIOSTATS
				rfPtr->RADIO_TXfail++;
#endif
				rfPtr->transactionStatus = INVALID;
				break;
		}
	} else	{
		rfPtr->transactionStatus = SUCCESS;			/* (PLL_ON)				*/
	

	}
	ready(rfPtr->writePid, RESCHYES);
	return;
}

/**
 * setNoAck() -- callback from Ack TIMEOUT interrupt
 */

INTPROC setNoAck(void *event)
{
	struct rfDeviceControlBlock *rfPtr = &radio[UNIT(*(uint8_t *)event)];

//	kprintf("setNoAck: TIMEOUT\n");
	rfPtr->ackRXPending = FALSE;		/* cancel pending */
	rfPtr->errorCode = RADIO_NO_ACK;
	resume(rfPtr->writePid);
}

