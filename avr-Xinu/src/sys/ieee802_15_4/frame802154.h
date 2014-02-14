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

typedef uint8_t octet_t;			/* smallest unit type of all packets */
typedef uint8_t frameReturn_t;

/** \struct frame_t
 *  \brief  This struct defines the basic storage unit for tx and rx frames
 *	which are managed by the buffer pool mechanism.
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

/** \brief Parameters used by the frame802154_create() function.  These
 *  parameters are used in the 802.15.4 frame header.  See the 802.15.4
 *  specification for details.
 *	Examples:
 *
 *	frame802154_t x;
 *	x.fcf.frameType = FRAME_TYPE_DATA;
 */
typedef struct {
	frameControlField_t fcf;	/* Frame control field  (2)				*/
	uint8_t seq;				/* Sequence number		(1)				*/
	uint16_t dest_pid;			/* Destination PAN ID	(0/2)			*/
	uint8_t dest_addr[8];		/* Destination address	(0/2/8)			*/
	uint16_t src_pid;			/* Source PAN ID		(0/2)			*/
	uint8_t src_addr[8];		/* Source address		(0/2/8)			*/
	auxSecurityHeader_t aux_hdr;/* Aux security header	(0/5/6/10/14)	*/
	uint8_t *payload;			/* Pointer to 802.15.4 frame payload	*/
	uint8_t payload_len;		/* Length of payload field				*/
} frame802154_t;

#define HEADERVARPTR(var) (octet_t *)(&(var))

/*============================ PROTOTYPES =======================================*/
frameReturn_t frame802154_create(frame802154_t *, frame_t *);
octet_t *makeTXFrameHdr(frame802154_t *, octet_t *);
octet_t *makeMACCommandHdr(frame802154_t *, octet_t *);
octet_t *copyOctets(octet_t *, octet_t *, int);
void frameDump(char *, uint8_t *, int);
void frameHeaderDump(char *, frame802154_t *, int);

#endif
