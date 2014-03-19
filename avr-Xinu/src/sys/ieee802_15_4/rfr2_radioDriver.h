//
//  rfr2_radioDriver.h
//  802_15_4_Mib
//
//  Created by Michael M Minor on 12/18/13.
//
//

#ifndef _02_15_4_Mib_rfr2_radioDriver_h
#define _02_15_4_Mib_rfr2_radioDriver_h

#include <kernel.h>
#include <frame802154.h>

#define RADIOSTATS 1				/* keep stats if 1 */
#define TIME_OUT_TIME 10*62500UL		/* 10 seconds       */
#define RADIO_TIMEOUT -99
#define DIG1 PING1
#define DIG2 PINF2

#define TMASK 0x07
#define EVENT(x) ((x)&TMASK)
#define UNIT(x)  ((x) >> 3)
#define MKEVENT(timer,unit) ( ((unit)<<3) | ( ((timer) & TMASK)) )
#define READ_WITH_TIMEOUT	1			/* timer EVENT */
#define WRITE_WITH_ACK		2

typedef enum	{	/* Time[us], typ. 				STATE TRANSITION				*/
/*					   --------------				----------------				*/
    TTR2	=			240,					/*         SLEEP->TRX_OFF			*/
    TTR3	=			 22,					/*       TRX_OFF->SLEEP (16 MHz)	*/
	TTR4	=			110,					/*       TRX_OFF->PLL_ON			*/
	TTR6	=			110,					/*       TRX_OFF->RX_ON				*/
	TTR10	=			 16,					/*        PLL_ON->BUSY_TX			*/
	TTR11	=			 32,					/*       BUSY_TX->PLL_ON			*/
	TTR12	=			  1,					/* FORCE_TRX_OFF->ALL				*/
	TTR13	=			 37,					/*         RESET->TRX_OFF			*/
	TTR14	=			  1,					/*       VARIOUS->PLL_ON			*/
} radio_trx_timing_t;

#define TIME_TO_ENTER_P_ON			510		/**<  Transition time from VCC is applied to P_ON. */
#define TIME_P_ON_TO_TRX_OFF		510		/**<  Transition time from P_ON to TRX_OFF. */
#define TIME_SLEEP_TO_TRX_OFF		TTR2	/**<  Transition time from SLEEP to TRX_OFF. */
#define TIME_RESET					6		/**<  Time to hold the RST pin low during reset */
#define TIME_ED_MEASUREMENT			140		/**<  Time it takes to do a ED measurement. */
#define TIME_CCA					140		/**<  Time it takes to do a CCA. */
#define TIME_PLL_LOCK				155		/**<  Maximum time it should take for the PLL to lock. */
#define TIME_FTN_TUNING				25		/**<  Maximum time it should take to do the filter tuning. */
#define TIME_NOCLK_TO_WAKE			6		/**<  Transition time from *_NOCLK to being awake. */
#define TIME_CMD_FORCE_TRX_OFF		TTR12	/**<  Time it takes to execute the FORCE_TRX_OFF command. */
#define TIME_TRX_OFF_TO_PLL_ACTIVE	TTR4	/**<  Transition time from TRX_OFF to PLL_ON	*/
#define TIME_STATE_TRANSITION_PLL_ACTIVE TTR14
#define TIME_SIFS_WITH_BACKOFF		200000	/* macSIFSPeriod+aUnitBackoffPeriod	*/

struct rfDeviceControlBlock	{
	struct devsw *rfDevice;				/* switch table entry */
	int readSemaphore;					/* mutual exclusion semaphore */
	int writeSemaphore;					/* mutual exclusion semaphore */
	int readPid;						/* process ID with a read in progress */
	int writePid;						/* process ID with a write in progress */
	frame802154_t *TX_frame;			/* transmit (unpacked) frame buffer */
	frame802154_t *RX_frame;			/* receive (unpacked) frame buffer */
	uint8_t TX_saveState;				/* transmit state */
	uint8_t IRQ_save;					/* IRQ_mask RX_END_Enable save/restore */
	uint8_t operatingMode;				/* tranceiver op mode (basic, extended) */
	uint8_t freeTXFrame;				/* frame is malloced, free the frame after write() */
	Bool doRXTimeout;					/* cancel read() after TIME_OUT_TIME seconds (10)	*/
	uint8_t transactionStatus;			/* status return */
	int errorCode;						/* SYSERR reason */
	uint8_t writeRetrys;				/* number of write attempts */
	Bool ackTXPending;					/* TRUE if an ack is being TXed */
	uint8_t ackTXSeq;					/* sequence for ACK frame */
	ackFrame_t ackTXFrame;				/* an initialized ACK */
	Bool ackRXPending;					/* TRUE if an ack is expected */
	uint8_t ackRXSeq;					/* sequence for ACK frame */
	ackFrame_t *ackRXFrame;				/* a received frame POINTER */
#if RADIOSTATS
	int RADIO_RXfail;
	int RADIO_TXfail;
#endif
};

#ifndef Nradio
#define Nradio 1						/* Number of radio devices */
									/* Wow - atmega2564rfr2 with an attached rf230! */
#endif



enum operating_mode	{
	BASIC,
	EXTENDED
};

enum contol_functions	{
	RADIO_RESET,
	RADIO_SET_BASIC_OPERATING_MODE,
	RADIO_SET_EXTENDED_OPERATING_MODE,
	RADIO_SET_FREE_TX_FRAME,
	RADIO_CLEAR_FREE_TX_FRAME,
	RADIO_SET_READ_WITH_TIMEOUT,
	RADIO_CLEAR_READ_WITH_TIMEOUT
};

typedef enum radioReturnValues	{
	RADIO_SUCCESS,
	RADIO_UNSUPPORTED_DEVICE,
	RADIO_INVALID_ARGUMENT,
	RADIO_TIMED_OUT,
	RADIO_WRONG_STATE,
	RADIO_BUSY_STATE,
	RADIO_STATE_TRANSITION_FAILED,
	RADIO_CCA_IDLE,
	RADIO_CCA_BUSY,
	RADIO_TRX_BUSY,
	RADIO_BAT_LOW,
	RADIO_BAT_OK,
	RADIO_CRC_FAILED,
	RADIO_CHANNEL_ACCESS_FAILURE,
	RADIO_NO_ACK
}radio_status_t;

extern struct rfDeviceControlBlock radio[];

DEVCALL radioInit(struct devsw *);
DEVCALL radioRead(struct devsw *, frame802154_t *, int);
DEVCALL radioWrite(struct devsw *, frame802154_t *, int);
DEVCALL radioCntl(struct devsw *, int);
INTPROC radioIInt(struct rfDeviceControlBlock *);
INTPROC	radioOInt(struct rfDeviceControlBlock *);

#endif
