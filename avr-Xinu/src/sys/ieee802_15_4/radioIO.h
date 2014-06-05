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
#define RDTIMEOUT_TIME 2*62500UL		/* 2 seconds       */

/* Constants for the state of an entry */

#define	RADIO_FREE	0		/* entry is unused		*/
#define	RADIO_USED	1		/* entry is being used		*/
#define	RADIO_RECV	2		/* entry has a process waiting	*/

/* Broadcast address Any PAN-ID (level 3 match) */

#define BROADCAST_ADDR 0xffff
#define ANY_PAN_ID 0xffff

/* Error handling */
#define NOFRAME (frame802154_t *)0
#define RDSUCCESS 1
#define RDNOSLOT 0xfe
#define RDBAD 0xff
#define RDTIMEOUT 0xfd

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
	Bool rdtimeout;			/* true if time out on read	*/
	uint8_t rdstatus;		/* error code				*/
	frame802154_t *rdqueue[RADIO_QSIZ];/* circular packet queue	*/
};

/* Network input process: procedure name and parameters */

struct	radioinfo	{			/* info and parms. for radio	*/
	int	radiopool;		/* radio packet buffer pool	*/
	Bool addressvalid;	/* MAC address loaded */
	int	nmutex;			/* output mutual excl. semaphore*/
	int	npacket;		/* # of packets received	*/
	int ndata;			/* number of valid data packets */
	int nbeacon;		/* number of beacon packets */
	int ncmd;			/* number of mac command packets */
	int	ndrop;			/* # of data packets not matched	*/
	int	nover;			/* # data packets dropped because queue full	*/
	int	nerror;			/* # dropped because of error	*/
};

extern struct radioentry radiotab[];	/* table of RADIO endpoints	*/
extern frame802154_t *currpkt;			/* pointer to current input frame */

/* Prototypes */
int netInit(void);
PROCESS netin(int, int *);
void radio_buffer_init(void);
void radio_in(void);
int radio_register	(
					 ShortAddr_t dest_addr,	/* destination address address or BROADCAST_ADDR */
					 PanId_t dest_panID,	/* destination RADIO protocol panID	*/
					 PanId_t src_panID		/* source RADIO protocol panID	*/
);
int radio_alloc(
			ShortAddr_t dest_addr,	/* destination address address or zero	*/
			PanId_t dest_panID,		/* destination RADIO protocol panID	*/
			PanId_t src_panID		/* source RADIO protocol panID	*/
);
int radio_recv_data	(
				 ShortAddr_t dest_addr,	/* destination address address or 0xffff	*/
				 PanId_t dest_panID,	/* destination RADIO protocol panID			*/
				 PanId_t src_panID,		/* source RADIO protocol panID				*/
				 octet_t *buff,			/* buffer to hold RADIO data				*/
				 uint8_t len,			/* length of buffer							*/
				 uint32_t timeout		/* read timeout in Symbol Times				*/
);
int radio_read_data(uint8_t slot, octet_t *buff, int len);
frame802154_t *radio_recv_frame	(
					 ShortAddr_t dest_addr,	/* destination address address or 0xffff	*/
					 PanId_t dest_panID,	/* destination RADIO protocol panID			*/
					 PanId_t src_panID,		/* source RADIO protocol panID				*/
					 uint32_t timeout		/* read timeout in Symbol Times				*/
);
frame802154_t *radio_read_frame(int slot);
int	radio_recvaddr	 (
					  ShortAddr_t *dest_addr,	/* ptr destination address or 0	*/
					  PanId_t *dest_panID,		/* ptr destination RADIO protocol panID	*/
					  PanId_t src_panID,		/* source RADIO protocol panID	*/
					  octet_t *buff,			/* buffer to hold RADIO data	*/
					  uint8_t len,				/* length of buffer		*/
					  uint32_t timeout			/* read timeout in symbol times		*/
);
INTPROC setTimeOut(void *event);
int radio_send_frame (
					  ShortAddr_t dest_addr,	/* destination address address or address_BCAST*/
					  PanId_t dest_panID,		/* destination RADIO protocol panID	*/
					  ShortAddr_t src_addr,		/* source address		*/
					  PanId_t src_panID,		/* source RADIO protocol panID	*/
					  frame802154_t *frame		/* radio frame to send	*/
);
int radio_send_data (
				ShortAddr_t dest_addr,		/* destination address address or address_BCAST*/
											/*  for a source broadcast	*/
				PanId_t dest_panID,			/* destination RADIO protocol panID	*/
				ShortAddr_t src_addr,		/* source address address		*/
				PanId_t src_panID,			/* source RADIO protocol panID	*/
				octet_t *buff,				/* buffer of RADIO data		*/
				uint8_t len					/* length of data in buffer	*/
);
int radio_release	(
					ShortAddr_t dest_addr,/* destination address address or zero	*/
					PanId_t dest_panID,	/* destination RADIO protocol panID	*/
					PanId_t src_panID		/* source RADIO protocol panID	*/
);
int radio_clear	 (
				  ShortAddr_t dest_addr,	/* destination address address or 0xffff	*/
				  PanId_t dest_panID,		/* destination RADIO protocol panID			*/
				  PanId_t src_panID			/* source RADIO protocol panID				*/
);
int radio_getSlot(
				  ShortAddr_t dest_addr,	/* destination address address or 0xffff	*/
				  PanId_t dest_panID,		/* destination RADIO protocol panID			*/
				  PanId_t src_panID			/* source RADIO protocol panID				*/
);



#endif
