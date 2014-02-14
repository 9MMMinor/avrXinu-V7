//
//  macSymbolCounter.h
//  radio-15-4
//
//  Created by Michael M Minor on 11/10/13.
//
//

#ifndef radio_15_4_macSymbolCounter_h
#define radio_15_4_macSymbolCounter_h

typedef uint32_t time_t;
#define NULLQ (struct tqent *)0
#define MAXTIMERMESSAGES	15

/* 802.15.4-2011
 The MLME shall timestamp each received beacon frame at the same symbol boundary within each frame,
 the location of which is described by the macSyncSymbolOffset attribute. The symbol boundary shall be
 the same as that used in the timestamp of the outgoing beacon frame, stored in macBeaconTxTime. The
 timestamp value shall be that of the local clock of the device at the time of the symbol boundary. The
 timestamp is intended to be a relative time measurement that may or may not be made absolute, at the
 discretion of the implementer.
 */
struct timestamp_timeval	{
	time_t	sysSeconds;			/* system seconds at reset of Symbol Counter (32 bit, SCCNT) */
	time_t	timestamp;			/* SCRSTR – Symbol Counter Received Frame Timestamp Register */
};

/* A timer queue list entry */

struct	tqent {
	uint32_t tq_compare;			/* time to expire (Symbol Counts)		*/
	uint32_t tq_time;				/* time this entry was queued			*/
	uint8_t tq_port;				/* port to send the event				*/
	void *tq_msg;					/* data to send when expired			*/
	INTPROC (*tq_callThru)(void *);	/* INTPROC called from event match ISR	*/
	struct tqent *tq_next;			/* next in the list						*/
};
/* timer process declarations and definitions */

extern int tqmutex;
extern int tqpid;
extern struct tqent *tqhead;
extern int timerPortID;

/* Prototypes */
void radioTimerEventInit(void);
void macSymbolCounterInit(void);
uint32_t macSymbolCounterRead(void);
void macSymbolCounterWrite(uint32_t value);
void macCompareUnit1Write(uint32_t value);
uint32_t tmclear(int, void *), tmleft(int, void *);
int tmset(uint8_t, void *, uint32_t, INTPROC (*tq_callThru)(void *));
void tqdump(void);
void tqwrite(void);

#endif
