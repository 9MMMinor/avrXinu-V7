//
//  mib.h
//  802_15_4_Mib
//
//  Created by Michael M Minor on 12/6/13.
//
//

#ifndef _02_15_4_Mib_mib_h
#define _02_15_4_Mib_mib_h

/* defines */
#define true 1
#define false 0

/* structs and typedefs */
typedef uint8_t octet_t;			/* type of sub object ids	*/

/* Structure that contains the value of an MIB variable: {MIBtype,MIBvalue} */
struct mibValue {
	uint8_t MIBtype;				/* variable type		*/
	union	{
		int MIBint;						/* variable is an integer, */
		struct {
			octet_t *MIBstr;				/* string's contents		*/
			int MIBstrLen;					/* string's length		*/
		} MIBstr;						/* variable is an octet string	*/
		int MIBoid;						/* variable is an object id	*/
		uint16_t MIBshortAddress;		/* variable is a short address */
		uint64_t MIBextendedAddr;		/* variable is an extended address	*/
	} MIBvalue;						/* value of var is one of the above	*/
};

typedef uint16_t PanId_t;     ///< PAN identifier type declaration.
typedef uint16_t ShortAddr_t; ///< Short address type declaration.
typedef uint64_t ExtAddr_t;   ///< Extended address type declaration.

struct mib_info	{
	const char *mib_name;		/* name of mib variable */
	const char *mib_prefix;		/* prefix name (e.g., "phy.") */
	int mib_objectID;			/* ieee802.15.4 mib identifier (2003-Table 71) */
	int mib_vartype;			/* sizeof(var) */
	int mib_writable;			/* is varible writable? */
	int (*mib_func)();			/* function that implements get/set/getnext */
	void *mib_param;			/* pointer to parameter used with function */
	struct mib_info *mib_next;	/* link */
};

//! PHY PIB attribute identifiers (IEEE Std 802.15.4-2006. - table 23).
enum PHY_Identifiers	{
	PHYCurrentChannel		= 0x00,					/* PHY_CC_CCA */
	PHYChannelsSupported	= 0x01,   // Read-only
	PHYTransmitPower		= 0x02,					/* PHY_TX_PWR */
	PHYCCAMode				= 0x03,					/* PHY_CC_CCA */
	PHYCurrentPage			= 0x04,
	PHYMaxFrameDuration		= 0x05,   // Read-only
	PHYSHRDuration			= 0x06,   // Read-only
	PHYSymbolsPerOctet		= 0x07,   // Read-only
};

//! CCA mode types. IEEE 802.15.4-2006 6.9.9 Clear channel assessment (CCA).
typedef enum	{
	PHY_CCA_MODE_N = 0,
	PHY_CCA_MODE_1 = 1,
	PHY_CCA_MODE_2 = 2,
	PHY_CCA_MODE_3 = 3
} PHY_CcaMode_t;

//! PHY PIB attribute types (IEEE Std 802.15.4-2006. - table 23).
typedef union
{
	uint8_t         channel;
	uint32_t        channelsSupported;
	int8_t          transmitPower;
	PHY_CcaMode_t   ccaMode;
	uint8_t         currentPage;
	uint16_t        maxFrameDuration;
	uint8_t         shrDuration;
	uint8_t         symbolsPerOctet;
} PHY_PibAttr_t;

//! PHY PIB attribute types (IEEE Std 802.15.4-2006. - table 86).
enum Mac_Identifiers	{
	MACAckWaitDuration				= 0x40,
    MACAssociationPermit			= 0x41,
    MACAutoRequest					= 0x42,
    MACBattLifeExt					= 0x43,
    MACBattLifeExtPeriods			= 0x44,
    MACBeaconPayload				= 0x45,
    MACBeaconPayloadLength			= 0x46,
    MACBeaconOrder					= 0x47,
    MACBeaconTxTime					= 0x48,
    MACBSN							= 0x49,
    MACCoordExtendedAddress			= 0x4a,
    MACCoordShortAddress			= 0x4b,
    MACDSN							= 0x4c,
    MACGTSPermit					= 0x4d,
    MACMaxCSMAbackoffs				= 0x4e,
    MACMinBE						= 0x4f,
    MACPANId						= 0x50,
    MACPromiscuousMode				= 0x51,
    MACRxOnWhenIdle					= 0x52,
    MACShortAddress					= 0x53,
    MACSuperframeOrder				= 0x54,
    MACTransactionPersistenceTime	= 0x55,
    MACAssociatedPANCoord			= 0x56,
    MACMaxBE						= 0x57,
    MACMaxFrameTotalWaitTime		= 0x58,
    MACMaxFrameRetries				= 0x59,
    MACResponseWaitTime				= 0x5a,
    MACSyncSymbolOffset				= 0x5b,
    MACTimestampSupported			= 0x5c,
    MACSecurityEnabled				= 0x5d,
    MACMinLIFSPeriod				= 0x5e
};

enum SAP_Identifiers	{
	SAP_REQUEST,
	SAP_INDICATION,
	SAP_RESPONSE,
	SAP_CONFIRM,
};

enum sap_Primatives	{
	MLME_ASSOCIATE,				/* 7.1.3.1 */
	MLME_DISASSOCIATE,			/* 7.1.4.1 */
	MLME_BEACON_NOTIFY,			/* 7.1.5.1 */
	MLME_GET,					/* 7.1.6.1 7.1.6.2 */
	MLME_GTS,					/* 7.1.7.1 7.1.7.3 7.1.7.2 */
	MLME_ORPHAN,				/* 7.1.8.1 7.1.8.2 */
	MLME_RESET,					/* 7.1.9.1 7.1.9.2 */
	MLME_RX_ENABLE,				/* 7.1.10.1 7.1.10.2 */
	MLME_SCAN,					/* 7.1.11.1 7.1.11.2 */
	MLME_COMM_STATUS,			/* 7.1.12.1 */
	MLME_SET,					/* 7.1.13.1 7.1.13.2 */
	MLME_START,					/* 7.1.14.1 7.1.14.2 */
	MLME_SYNC,					/* 7.1.15.1	*/
	MLME_SYNC_LOSS,				/* 7.1.15.2 */
	MLME_POLL					/* 7.1.16.1 */
};

/* Information about MIB tables.  Contains functions to implement 	*/
/* operations upon variables in the tables.				*/

struct sap_info {
	int	(*sap_request)(struct sap_info *, struct mib_info *);
	int	(*sap_indication)(struct sap_info *, struct mib_info *);
	int	(*sap_response)(struct sap_info *, struct mib_info *);
	int	(*sap_confirm)(struct sap_info *, struct mib_info *);
	struct sap_info *next; // pointer to sap information record
};


/* prototypes */
int mib_Function(void *);
struct mib_info *get_MibEntry(int);
int ASSOCIATE_request(struct sap_info *,  struct mib_info *);
int ASSOCIATE_indication(struct sap_info *, struct mib_info *);
int ASSOCIATE_response(struct sap_info *, struct mib_info *);
int ASSOCIATE_confirm(struct sap_info *, struct mib_info *);
int DISASSOCIATE_request(struct sap_info *,  struct mib_info *);
int DISASSOCIATE_indication(struct sap_info *, struct mib_info *);
int DISASSOCIATE_confirm(struct sap_info *, struct mib_info *);
int BEACON_NOTIFY_indication(struct sap_info *, struct mib_info *);
int GET_request(struct sap_info *,  struct mib_info *);
int GET_confirm(struct sap_info *, struct mib_info *);
int GTS_request(struct sap_info *,  struct mib_info *);
int GTS_indication(struct sap_info *, struct mib_info *);
int GTS_confirm(struct sap_info *, struct mib_info *);
int ORPHAN_indication(struct sap_info *, struct mib_info *);
int ORPHAN_response(struct sap_info *, struct mib_info *);
int RESET_request(struct sap_info *,  struct mib_info *);
int RESET_confirm(struct sap_info *, struct mib_info *);
int RX_ENABLE_request(struct sap_info *,  struct mib_info *);
int RX_ENABLE_confirm(struct sap_info *, struct mib_info *);
int SCAN_request(struct sap_info *,  struct mib_info *);
int SCAN_confirm(struct sap_info *, struct mib_info *);
int COMM_STATUS_indication(struct sap_info *, struct mib_info *);
int SET_request(struct sap_info *,  struct mib_info *);
int SET_confirm(struct sap_info *, struct mib_info *);
int START_request(struct sap_info *,  struct mib_info *);
int START_confirm(struct sap_info *, struct mib_info *);
int SYNC_request(struct sap_info *,  struct mib_info *);
int SYNC_LOSS_indication(struct sap_info *, struct mib_info *);
int POLL_request(struct sap_info *,  struct mib_info *);
int POLL_confirm(struct sap_info *, struct mib_info *);
int NOFUNCTION_error(struct sap_info *, struct mib_info *mip);

#endif
