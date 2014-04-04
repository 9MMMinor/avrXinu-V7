//
//  radioIO.h
//  802_15_4_Mib
//
//  Created by Michael Minor on 3/31/14.
//
//

#ifndef _02_15_4_Mib_radioIO_h
#define _02_15_4_Mib_radioIO_h

/* radioIO.h - declarations pertaining to ieee 802.15.4 Protocol (RADIO) */

#define	RADIO_SLOTS	4		/* num. of open RADIO endpoints	*/
#define	RADIO_QSIZ	5		/* packets enqueued per endpoint*/
#define RADIOBUFS	RADIO_SLOTS * RADIO_QSIZ + 1

/* Constants for the state of an entry */

#define	RADIO_FREE	0		/* entry is unused		*/
#define	RADIO_USED	1		/* entry is being used		*/
#define	RADIO_RECV	2		/* entry has a process waiting	*/

struct	radioentry {		/* entry in the RADIO endpoint tbl*/
	uint8_t	rdstate;		/* state of entry: free/used	*/
//	uint8_t	rddest_addr[8];	/* destination address (zero means "don't care") */
	uint64_t rddest_addr;
//	uint8_t	rdsrc_addr[8];	/* source address		*/
	uint64_t rdsrc_addr;
	uint16_t rddest_panID;	/* destination PAN ID	*/
	uint16_t rdsrc_panID;	/* source PAN ID	*/
	uint8_t	rdhead;			/* index of next packet to read	*/
	uint8_t	rdtail;			/* index of next slot to insert	*/
	uint8_t	rdcount;		/* count of packets enqueued	*/
	int	rdpid;				/* ID of waiting process	*/
	frame802154_t *rdqueue[RADIO_QSIZ];/* circular packet queue	*/
};

/* Network input process: procedure name and parameters */

struct	radioinfo	{			/* info and parms. for radio	*/
	int	radiopool;		/* radio packet buffer pool	*/
	Bool addressvalid;	/* MAC address loaded */
	int fiport;			/* frame input port */
	int foport;			/* frame output port */
	//	int faport;			/* frame ack port */
	int	nmutex;			/* output mutual excl. semaphore*/
	int	npacket;		/* # of packets processed	*/
	int	ndrop;			/* # of packets discarded	*/
	int	nover;			/* # dropped because queue full	*/
	int	nmiss;			/* # dropped	*/
	int	nerror;			/* # dropped	*/
};

extern struct radioentry radiotab[];	/* table of RADIO endpoints	*/
extern frame802154_t *currpkt;			/* pointer to current input frame */

/* Prototypes */
int netInit(void);
PROCESS netin(int, int *);
void radio_buffer_init(void);
void radio_in(void);
int radio_register(uint64_t, uint16_t, uint16_t);
uint8_t radio_recv(uint64_t, uint16_t, uint16_t, octet_t *, uint8_t, uint32_t);
uint8_t	radio_recvaddr(uint64_t *, uint16_t *, uint16_t, octet_t *, uint8_t, uint32_t);
INTPROC setTimeOut(void *event);
int radio_send(
				uint64_t dest_addr,			/* destination address address or address_BCAST*/
											/*  for a source broadcast	*/
				uint16_t dest_panID,		/* destination RADIO protocol panID	*/
				uint64_t src_addr,			/* source address address		*/
				uint16_t src_panID,			/* source RADIO protocol panID	*/
				octet_t *buff,				/* buffer of RADIO data		*/
				uint8_t len					/* length of data in buffer	*/
);
int radio_release (
				   uint64_t dest_addr,	/* destination address address or zero	*/
				   uint16_t dest_panID,	/* destination RADIO protocol panID	*/
				   uint16_t src_panID	/* source RADIO protocol panID	*/
);


#endif
