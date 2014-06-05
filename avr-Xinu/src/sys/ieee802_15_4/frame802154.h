//
//  frame802154.h
//  802_15_4_Mib
//
//  Created by Michael M Minor on 12/4/13.
//
//

#ifndef _02_15_4_Mib_frame802154_h
#define _02_15_4_Mib_frame802154_h

#define MAX_FRAME_LENGTH 127
#define MIN_FRAME_LENGTH 3
#define FTR_LEN 2
#define MAX_DATA_LENGTH MAX_FRAME_LENGTH-MIN_FRAME_LENGTH
#define MAX_HDR_LENGTH 38			/* +1 for header_len */

/* Frame Control Field (FCF) */
#define FRAME_TYPE_BEACON		0
#define FRAME_TYPE_DATA			1
#define FRAME_TYPE_ACK			2
#define FRAME_TYPE_MAC_COMMAND	3

#define FRAME_ADDRESS_MODE_NONE		0
#define FRAME_ADDRESS_MODE_SHORT	2
#define FRAME_ADDRESS_MODE_EXTENDED	3

#define FRAME_VERSION_2003	0
#define FRAME_VERSION_2006	1

#define FRAME_SECURITY_NONE	0
#define FRAME_SECURITY_EN	1

/*============================ TYPDEFS =======================================*/

typedef uint8_t octet_t;			/* smallest unit type of all packets*/
typedef uint16_t PanId_t;     		/* PAN identifier type declaration	*/
typedef uint8_t *CharAddr_t;		/* Character address type declaration */
typedef uint16_t ShortAddr_t;		/* Short address type declaration	*/
typedef uint64_t ExtAddr_t;			/* Extended address type declaration*/
typedef uint8_t frameReturn_t;

/** \struct frame_t
 *  \brief  This struct defines the basic storage unit for tx and rx frames
 *
 *  \see radioRead() and radioWrite()
 */
typedef struct {
	uint8_t length;						/* Frame Length Field (PHR), inserted at frame creation (TX)	*/
										/* or added after receipt (RX); an ACK frame length = 5,		*/
										/* a data frame is 9 - MAX_FRAME_LENGTH							*/
	octet_t data[ MAX_FRAME_LENGTH ];	/* Actual frame data */
	uint8_t lqi;						/* LQI value for received frame, added RX */
	uint8_t crc;						/* Flag - did CRC pass for received frame? */
} frame_t;

/**
 * \brief Defines the bitfields of the frame control field (FCF).
 */

typedef struct frameControlField	{
	uint16_t frameType:3;
	uint16_t frameSecurity:1;
	uint16_t framePending:1;
	uint16_t frameAckRequested:1;
	uint16_t framePanIDCompress:1;
	uint16_t frameReserved:3;
	uint16_t frameDestinationAddressMode:2;
	uint16_t frameVersion:2;
	uint16_t frameSourceAddressMode:2;
} frameControlField_t;

/** \brief 802.15.4 security control bitfield.  See section 7.6.2.2.1 in 802.15.4 specification */
typedef struct securityControlField	{
	uint8_t securityLevel:3;
	uint8_t keyIdMode:2;
	uint8_t reservedBits:3;
} securityControlField_t;

/** \brief 802.15.4 Aux security header */
typedef struct auxSecurityHeader {
	securityControlField_t scf;
	uint32_t frameCounter;
	octet_t key[9];			/* The key, or an index to the key */
} auxSecurityHeader_t;

typedef union	{
	char caddr[8];
	ShortAddr_t saddr;
	ExtAddr_t eaddr;
} radioAddr_t;

/** \brief Parameters used by the frame802154_create() function.  These
 *  parameters are used in the 802.15.4 frame header.  See the 802.15.4
 *  specification for details.
 *	Examples:
 *
 *	frame802154_t x;
 *	x.fcf.frameType = FRAME_TYPE_DATA;
 */
typedef struct {
	frameControlField_t fcf;		/* Frame control field  (2)				*/
	uint8_t seq;					/* Sequence number		(1)				*/
	uint16_t dest_pid;				/* Destination PAN ID	(0/2)			*/
	radioAddr_t dest_addr;			/* Destination address	(0/2/8)			*/
	uint16_t src_pid;				/* Source PAN ID		(0/2)			*/
	radioAddr_t src_addr;			/* Source address		(0/2/8)			*/
	auxSecurityHeader_t aux_hdr;	/* Aux security header	(0/5/6/10/14)	*/
	uint8_t header_len;				/* Actual header length (0)				*/
	uint8_t data_len;				/* Payload length		(0)				*/
	octet_t data[MAX_DATA_LENGTH];	/* Payload				(0 - 122)		*/
	uint16_t crc;					/* FCS = MFR			(2)				*/
} frame802154_t;

typedef struct {
	uint8_t	fsize;
	union {	frameControlField_t fcf; uint16_t fcfword; } fcfField;
	uint8_t seq;
	uint16_t crc;
} ackTXFrame_t;

typedef struct {
	frameControlField_t fcf;
	uint8_t seq;
	uint16_t crc;
} ackRXFrame_t;

/*============================ MAC Command Packets ===============================*/

//! 5.3 MAC Command Frame identifiers (IEEE Std 802.15.4-2011. - table 5).
enum COMMANDFRAME_Identifiers	{
	COMMANDFRAMEAssociationRequest			= 0x01,
	COMMANDFRAMEAssociationResponse			= 0x02,
	COMMANDFRAMEDisassociationNotification	= 0x03,
	COMMANDFRAMEDataRequest					= 0x04,
	COMMANDFRAMEPanidConflictNotification	= 0x05,
	COMMANDFRAMEOrphanNotification			= 0x06,
	COMMANDFRAMEBeaconRequest				= 0x07,
	COMMANDFRAMECoordinatorRealignment		= 0x08,
	COMMANDFRAMEGTSRequest					= 0x09,
};

//! 5.3.1.2 Capability Information field (IEEE Std 802.15.4-2011. - Figure 50).
typedef struct associateCapabilityInfo	{
	uint8_t res0:1;
	uint8_t type:1;			/* 1->FFD 0->RFD											*/
	uint8_t power:1;		/* 1->AC powered device, 0->otherwise (battery)				*/
	uint8_t recOnIdle:1;	/* 1->does not disable receiver when idle, 0-> otherwise	*/
	uint8_t res1:2;
	uint8_t security:1;		/* 1->capable of secure TX/RX, 0->otherwise					*/
	uint8_t addressAlloc:1;	/* 1->coordinator selects my short address, 0->otherwise	*/
} associateCapabilityInfo_t;

//! 5.3.1 Association request command (IEEE Std 802.15.4-2011).
typedef struct AssociationRequestPacket	{
	uint8_t	cmd;		/* =COMMANDFRAMEAssociationRequest	*/
	associateCapabilityInfo_t info;
} associationRequest_t;

enum associationStatusField	{
	ASSOCIATION_SUCCESS	= 0x00,
	PAN_AT_CAPACITY		= 0x01,
	PAN_ACCESS_DENIED	= 0x02,
};

//! 5.3.2 Association response command (IEEE Std 802.15.4-2011).
typedef struct AssociationResponsePacket	{
	uint8_t	cmd;		/* =COMMANDFRAMEAssociationResponse	*/
	ShortAddr_t addr;	/* short address on PAN, 0xffff (error), 0xfffe (use extended) */
	uint8_t status;		/* Association status field */
} associationResponse_t;

//! 5.3.3 Disassociation notification command
//! 5.3.4 Data request command
//! 5.3.5 PAN ID conflict notification command
//! 5.3.6 Orphan notification command
//! 5.3.7 Beacon request command
//! 5.3.8 Coordinator realignment command
//! 5.3.9 GTS request command

/*============================== MACROS =========================================*/
#define HEADERVARPTR(var) (octet_t *)(&(var))
#define COPY_EXTENDED_ADDR(TO, FROM) memcpy(&(TO), &(FROM), 8)
#define IS_EQ_EXTENDED_ADDR(A, B) (0 == memcmp(&(A), &(B), 8))

/*============================ PROTOTYPES =======================================*/
frame802154_t *frame802154_create(uint8_t);
octet_t	*radio_createDataHdr (
					 frame802154_t *,	/* source packet buffer */
					 ShortAddr_t,		/* destination address address or address_BCAST*/
					 PanId_t,			/* destination RADIO protocol panID	*/
					 ShortAddr_t,		/* source address		*/
					 PanId_t			/* source RADIO protocol panID	*/
);
uint8_t getFrameHdrLength(frame802154_t *);
void packTXFrame(frame802154_t *);
void unpackRXFrame(frame802154_t *);
frame802154_t *makeMACCommandHdr(frame802154_t *);
octet_t *copyOctets(octet_t *, octet_t *, int);
octet_t *copyRXOctets(octet_t *, octet_t *, int);
void frameDump(char *, uint8_t *, int);
void frameRAWHeaderDump(char *, octet_t *, int);
void frameHeaderDump(char *, frame802154_t *, int);

#endif
