/* mib.c */

#include <avr-Xinu.h>
#include <avr/pgmspace.h>
#include "frame802154.h"
#include "mib.h"


const char phy[] PROGMEM = "phy.";
const char CurrentChannel[] PROGMEM = "CurrentChannel";
const char ChannelsSupported[] PROGMEM = "ChannelsSupported";   // Read-only
const char TransmitPower[] PROGMEM = "TransmitPower";
const char CCAMode[] PROGMEM = "CCAMode";
const char CurrentPage[] PROGMEM = "CurrentPage";
const char MaxFrameDuration[] PROGMEM = "MaxFrameDuration";		// Read-only
const char SHRDuration[] PROGMEM = "SHRDuration";				// Read-only
const char SymbolsPerOctet[] PROGMEM = "SymbolsPerOctet";		// Read-only

const char Mac[] PROGMEM = "mac.";
const char AckWaitDuration[] PROGMEM = "AckWaitDuration";
const char AssociationPermit[] PROGMEM = "AssociationPermit";
const char AutoRequest[] PROGMEM = "AutoRequest";
const char BattLifeExt[] PROGMEM = "BattLifeExt";
const char BattLifeExtPeriods[] PROGMEM = "BattLifeExtPeriods";
const char BeaconPayload[] PROGMEM = "BeaconPayload";
const char BeaconPayloadLength[] PROGMEM = "BeaconPayloadLength";
const char BeaconOrder[] PROGMEM = "BeaconOrder";
const char BeaconTxTime[] PROGMEM = "BeaconTxTime";
const char BSN[] PROGMEM = "BSN";
const char CoordExtendedAddress[] PROGMEM = "CoordExtendedAddress";
const char CoordShortAddress[] PROGMEM = "CoordShortAddress";
const char DSN[] PROGMEM = "DSN";
const char GTSPermit[] PROGMEM = "GTSPermit";
const char MaxCSMAbackoffs[] PROGMEM = "MaxCSMAbackoffs";
const char MinBE[] PROGMEM = "MinBE";
const char PANId[] PROGMEM = "PANId";
const char PromiscuousMode[] PROGMEM = "PromiscuousMode";
const char RxOnWhenIdle[] PROGMEM = "RxOnWhenIdle";
const char ShortAddress[] PROGMEM = "ShortAddress";
const char SuperframeOrder[] PROGMEM = "SuperframeOrder";
const char TransactionPersistenceTime[] PROGMEM = "TransactionPersistenceTime";
const char AssociatedPANCoord[] PROGMEM = "AssociatedPANCoord";
const char MaxBE[] PROGMEM = "MaxBE";
const char MaxFrameTotalWaitTime[] PROGMEM = "MaxFrameTotalWaitTime";
const char MaxFrameRetries[] PROGMEM = "MaxFrameRetries";
const char ResponseWaitTime[] PROGMEM = "ResponseWaitTime";
const char SyncSymbolOffset[] PROGMEM = "SyncSymbolOffset";
const char TimestampSupported[] PROGMEM = "TimestampSupported";
const char SecurityEnabled[] PROGMEM = "SecurityEnabled";
const char MinLIFSPeriod[] PROGMEM = "MinLIFSPeriod";

/* PHY global variables */ /* = 0 Uninitialized; = DEFAULT initialized */
uint8_t			phyCurrentChannel		= 11;
uint32_t		phyChannelsSupported	= 0;
int8_t          phyTransmitPower		= 0;				/* 3.5 dB (maximum) */
PHY_CcaMode_t   phyCCAMode				= PHY_CCA_MODE_N;
uint8_t         phyCurrentPage			= 0x03;				/* Table 67 802.15.4(2011) */
uint16_t        phyMaxFrameDuration;
uint8_t         phySHRDuration;
uint8_t         phySymbolsPerOctet;

/* MAC global variables */
uint32_t		macAckWaitDuration		= 54;			/* symbol times (846 us) */
/*		The read-only attribute macAckWaitDuration is dependent on a combination of constants and PHY PIB
		attributes. The PHY PIB attributes are listed in Table 71. The formula for relating the constants and attributes
		is:
			macAckWaitDuration = aUnitBackoffPeriod + aTurnaroundTime + phySHRDuration + ceiling (6
																					  × phySymbolsPerOctet)
		where the number 6 comes from the number of PHY header octets plus the number of PSDU octets in an
		acknowledgment frame and ceiling() is a function that returns the smallest integer value greater than or equal
		to its argument value.
 */
Bool			macAssociationPermit;
Bool			macAutoRequest;
//Bool			macBattLifeExt;
//uint8_t		macBattLifeExtPeriods;
uint8_t			*macBeaconPayload;
uint8_t			macBeaconPayloadLength;
//uint8_t		macBeaconOrder;
//uint32_t		macBeaconTxTime;
uint8_t			macBSN;
ExtAddr_t		macCoordExtendedAddress;
ShortAddr_t		macCoordShortAddress;
uint8_t			macDSN;
Bool			macGTSPermit;
uint8_t			macMaxCsmaBackoffs;
uint8_t			macMinBE;
PanId_t			macPANId;
Bool			macPromiscuousMode	= FALSE;
Bool			macRxOnWhenIdle;
ShortAddr_t		macShortAddress = 0xbabe;
ExtAddr_t		macExtAddress;                   // Additional parameter. NOT described in the standard.
// This parameter replaces MAC_EXTENDED_ADDRESS constant.
// Device own extended address.
//uint8_t		macSuperframeOrder;
uint16_t		macTransactionPersistenceTime;
uint32_t		macTransactionPersistenceTimeInMs;
Bool			macAssociatedPanCoord;
uint8_t			macMaxBE;
uint8_t			macMaxFrameTotalWaitTime;
uint8_t			macMaxFrameRetries;
uint8_t			macResponseWaitTime;
Bool			macTimestampSupported;
Bool			macSecurityEnabled;
Bool			macBeaconTxEnable;
uint32_t		macResponseWaitTimeMs; // MAC response wait time in !!!milliseconds!!!.

/** \brief Our own long address. This needs to be read from EEPROM or
 *  other secure memory storage.
 */
uint64_t macLongAddr = 0x000425191801058C;				/* my Xplained Pro */
uint8_t macLongAddrBuf[8];		/* for debug? */

/*
 *-------------------------------------------------------------------------------------------------------------------------
 *	mib_info -- contains an entry for every MIB variable and some PHY variables. Each entry contains all information needed
 *	to access or modify the variable. The mib-info table is searchable by mib_objectID
 *	(ieee802.15.4 pib identifier (2003-Table 71, p135)).
 *-------------------------------------------------------------------------------------------------------------------------
 */
struct mib_info mib[] = {
	/* PHY MIB */
	{CurrentChannel, phy, PHYCurrentChannel, sizeof(phyCurrentChannel), true, mib_Function, (void *)&phyCurrentChannel, 0},
	{ChannelsSupported, phy, PHYChannelsSupported, sizeof(phyChannelsSupported), false, mib_Function, (void *) &phyChannelsSupported, 0},
	{TransmitPower, phy, PHYTransmitPower, sizeof(phyTransmitPower), true, mib_Function, (void *) &phyTransmitPower, 0},
	{CCAMode, phy, PHYCCAMode, sizeof(phyCCAMode), true, mib_Function, (void *) &phyCCAMode, 0},
	{CurrentPage, phy, PHYCurrentPage, sizeof(phyCurrentPage), false, mib_Function, (void *) &phyCurrentPage, 0},
//	{MaxFrameDuration, phy, PHYMaxFrameDuration, sizeof(phyMaxFrameDuration), false, mib_Function, (void *) &phyMaxFrameDuration, 0},
//	{SHRDuration, phy, PHYSHRDuration, sizeof(phySHRDuration), false, mib_Function, (void *) &phySHRDuration, 0},
//	{SymbolsPerOctet, phy, PHYSymbolsPerOctet, sizeof(phySymbolsPerOctet), false, mib_Function, (void *) &phySymbolsPerOctet, 0},
	/* MAC MIB */
	{AckWaitDuration, Mac, MACAckWaitDuration, sizeof(macAckWaitDuration), true, mib_Function, (void *)&macAckWaitDuration, 0},
	{AssociationPermit,  Mac, MACAssociationPermit, sizeof(macAssociationPermit), true, mib_Function, (void *)&macAssociationPermit, 0},
	{AutoRequest, Mac, MACAutoRequest, sizeof(macAutoRequest), true, mib_Function, (void *)&macAutoRequest, 0},
//	{BattLifeExt, Mac, MACBattLifeExt, sizeof(macBattLifeExt), true, mib_Function, (void *)&macBattLifeExt, 0},
//	{BattLifeExtPeriods,  Mac, MACBattLifeExtPeriods, sizeof(macBattLifeExtPeriods), true, mib_Function, (void *)&macBattLifeExtPeriods, 0},
	{BeaconPayload, Mac, MACBeaconPayload, sizeof(macBeaconPayload), true, mib_Function, (void *)&macBeaconPayload, 0},
	{BeaconPayloadLength,  Mac, MACBeaconPayloadLength, sizeof(macBeaconPayloadLength), true, mib_Function, (void *)&macBeaconPayloadLength, 0},
//	{BeaconOrder, Mac, MACBeaconOrder, sizeof(macBeaconOrder), true, mib_Function, (void *)&macBeaconOrder, 0},
//	{BeaconTxTime, Mac, MACBeaconTxTime, sizeof(macBeaconTxTime), true, mib_Function, (void *)&macBeaconTxTime, 0},
	{BSN, Mac, MACBSN, sizeof(macBSN), true, mib_Function, (void *)&macBSN, 0},
	{CoordExtendedAddress,  Mac, MACCoordExtendedAddress, sizeof(macCoordExtendedAddress), true, mib_Function, (void *)&macCoordExtendedAddress, 0},
	{CoordShortAddress,  Mac, MACCoordShortAddress, sizeof(macCoordShortAddress), true, mib_Function, (void *)&macCoordShortAddress, 0},
	{DSN, Mac, MACDSN, sizeof(macDSN), true, mib_Function, (void *)&macDSN, 0},
	{GTSPermit,  Mac, MACGTSPermit, sizeof(macGTSPermit), true, mib_Function, (void *)&macGTSPermit, 0},
	{MaxCSMAbackoffs,  Mac, MACMinBE, sizeof(macAckWaitDuration), true, mib_Function, (void *)&macAckWaitDuration, 0},
	{MinBE, Mac, MACMinBE, sizeof(macMinBE), true, mib_Function, (void *)&macMinBE, 0},
	{PANId, Mac, MACPANId, sizeof(macPANId), true, mib_Function, (void *)&macPANId, 0},
//	{PromiscuousMode,  Mac, MACPromiscuousMode, sizeof(macPromiscuousMode), true, mib_Function, (void *)&macPromiscuousMode, 0},
	{RxOnWhenIdle,  Mac, MACRxOnWhenIdle, sizeof(macRxOnWhenIdle), true, mib_Function, (void *)&macRxOnWhenIdle, 0},
	{ShortAddress,  Mac, MACShortAddress, sizeof(macShortAddress), true, mib_Function, (void *)&macShortAddress, 0},
//	{SuperframeOrder, Mac, MACSuperframeOrder, sizeof(macSuperframeOrder), true, mib_Function, (void *)&macSuperframeOrder, 0},
	{TransactionPersistenceTime,  Mac, MACTransactionPersistenceTime, sizeof(macTransactionPersistenceTime), true, mib_Function, (void *)&macTransactionPersistenceTime, 0},
//	{AssociatedPANCoord,  Mac, MACAssociatedPANCoord, sizeof(macAssociatedPANCoord), true, mib_Function, (void *)&macAssociatedPANCoord, 0},
	{MaxBE, Mac, MACMaxBE, sizeof(macMaxBE), true, mib_Function, (void *)&macMaxBE, 0},
//	{MaxFrameTotalWaitTime, Mac, MACMaxFrameTotalWaitTime, sizeof(macMaxFrameTotalWaitTime), true, mib_Function, (void *)&macMaxFrameTotalWaitTime, 0},
	{MaxFrameRetries, Mac, MACMaxFrameRetries, sizeof(macMaxFrameRetries), true, mib_Function, (void *)&macMaxFrameRetries, 0},
	{ResponseWaitTime, Mac, MACResponseWaitTime, sizeof(macResponseWaitTime), true, mib_Function, (void *)&macResponseWaitTime, 0},
//	{SyncSymbolOffset, Mac, MACSyncSymbolOffset, sizeof(macSyncSymbolOffset), true, mib_Function, (void *)&macSyncSymbolOffset, 0},
	{TimestampSupported, Mac, MACTimestampSupported, sizeof(macTimestampSupported), true, mib_Function, (void *)&macTimestampSupported, 0},
	{SecurityEnabled,  Mac, MACSecurityEnabled, sizeof(macSecurityEnabled), true, mib_Function, (void *)&macSecurityEnabled, 0},
//	{MinLIFSPeriod, Mac, MACMinLIFSPeriod, sizeof(macMinLIFSPeriod), true, mib_Function, (void *)&macMinLIFSPeriod, 0},
//	{MinSIFSPeriod, Mac, MACMinSIFSPeriod, sizeof(macMinSIFSPeriod), true, mib_Function, (void *)&macMinSIFSPeriod, 0}
};

int mib_entries = sizeof(mib) / sizeof(struct mib_info);
uint8_t *mibPhyIndex;
uint8_t *mibMacIndex;
int mibTableInitialized = 0;


/* FROM SICS: */
/*
 *
 *  Copyright (c) 2008, Swedish Institute of Computer Science
 *  All rights reserved.
 *
 *  Additional fixes for AVR contributed by:
 *
 *	Colin O'Flynn coflynn@newae.com
 *	Eric Gnoske egnoske@gmail.com
 *	Blake Leverett bleverett@gmail.com
 *	Mike Vidales mavida404@gmail.com
 *	Kevin Brown kbrown3@uccs.edu
 *	Nate Bohlmann nate@elfwerks.com
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of the copyright holders nor the names of
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */
/* SICS: FIRST PART OF frame_tx_create()
void
field_len(frame802154_t *p, field_length_t *flen)
{
	// init flen to zeros
	memset(flen, 0, sizeof(field_length_t));
	
	// Determine lengths of each field based on fcf and other args
	if (p->fcf.frameDestinationAddressMode) {
		flen->dest_pid_len = flen->dest_addr_len = 2;
	}
	if(p->fcf.frameSourceAddressMode) {
		flen->src_pid_len = flen->src_addr_len = 2;
	}
	
	// Set PAN ID compression bit if src pan id matches dest pan id.
	if(p->fcf.frameDestinationAddressMode && p->fcf.frameSourceAddressMode && p->src_pid == p->dest_pid) {
		p->fcf.panIDCompress = 1;
		// compressed header, only do dest pid
		flen->src_pid_len = 0;
	} else {
		p->fcf.panIDCompress = 0;
	}
	// determine address lengths
	if (p->fcf.frameDestinationAddressMode == FRAME_ADDRESS_MODE_EXTENDED)
		flen->dest_addr_len = 8;
	if (p->fcf.frameSourceAddressMode == FRAME_ADDRESS_MODE_EXTENDED)
		flen->src_addr_len = 8;
	
	// Aux security header
	if(p->fcf.frameSecurity) {
		switch(p->aux_hdr.security_control.key_id_mode) {
			case 0:
				flen->aux_sec_len = 5; // minimum value
				break;
			case 1:
				flen->aux_sec_len = 6;
				break;
			case 2:
				flen->aux_sec_len = 10;
				break;
			case 3:
				flen->aux_sec_len = 14;
				break;
			default:
				break;
		}
	}
}
*/

int mib_Function(void * mib_var)
{
	return 0;
}

/**
 * @brief Gets the MIB table entry
 *
 * @param Search mib-table for pib identifier.
 *
 * @return Pointer to the MIB record
 *
 *
 */
struct mib_info *
get_MibEntry(int pib_attribute_id)
{
	int index;
	
	if ( (index = getMibID(pib_attribute_id)) != SYSERR)	{
		return &mib[index];
	}
	return (struct mib_info *)0;
}

/*
 *	getMibID - get the MIB table entry index
 *
 */
int
getMibID(int pib_attribute_id)
{
	int i, index;
	
	if ( !mibTableInitialized )	{
		mibPhyIndex = (uint8_t *)malloc(MAX_PHY_IDENTIFIER-MIN_PHY_IDENTIFIER+1);
		mibMacIndex = (uint8_t *)malloc(MAX_MAC_IDENTIFIER-MIN_MAC_IDENTIFIER+1);
		memset(mibPhyIndex, 0xff, MAX_PHY_IDENTIFIER-MIN_PHY_IDENTIFIER+1);
		memset(mibMacIndex, 0xff, MAX_MAC_IDENTIFIER-MIN_MAC_IDENTIFIER+1);
		for (i = 0; i < mib_entries; i++)	{
			index = mib[i].mib_objectID;
			if (index < MIN_MAC_IDENTIFIER)	{
				mibPhyIndex[index] = i;
			}
			else	{
				index -= MIN_MAC_IDENTIFIER;
				mibMacIndex[index] = i;
			}
		}
		mibTableInitialized++;
	}
	if ( PHY_ATTRIBUTE_VALID(pib_attribute_id) || MAC_ATTRIBUTE_VALID(pib_attribute_id) )	{
		index = (pib_attribute_id<MIN_MAC_IDENTIFIER) ?
					mibPhyIndex[pib_attribute_id] : mibMacIndex[pib_attribute_id-MIN_MAC_IDENTIFIER];
		return (index==0xff ? SYSERR : index);
	}
	return SYSERR;
}


/* Implementation */
/* SICS: */
/** \brief Initializes the (quasi) 802.15.4 MAC.  This function should
 * be called only once on startup.
 */
void
mac_init(void)
{
    uint8_t *buf;
	
	macExtAddress = macLongAddr;		/* should come from EEPROM */
	buf = macLongAddrBuf;
    /* Buffer the uint64_t address for easy loading and debug. */
    IEEE_ADDR_0 = buf[0] = macLongAddr & 0xFF;
    IEEE_ADDR_1 = buf[1] = (macLongAddr >> 8) & 0xFF;
    IEEE_ADDR_2 = buf[2] = (macLongAddr >> 16) & 0xFF;
    IEEE_ADDR_3 = buf[3] = (macLongAddr >> 24) & 0xFF;
    IEEE_ADDR_4 = buf[4] = (macLongAddr >> 32) & 0xFF;
    IEEE_ADDR_5 = buf[5] = (macLongAddr >> 40) & 0xFF;
    IEEE_ADDR_6 = buf[6] = (macLongAddr >> 48) & 0xFF;
    IEEE_ADDR_7 = buf[7] = (macLongAddr >> 56) & 0xFF;
    /* Load the long address into the radio. This is required for auto mode */
    /* operation. */
//	radio_set_extended_address((uint8_t *)&macLongAddr);
}

/* Table 46—Summary of the primitives accessed through the MLME-SAP */

struct sap_info sapFuncTab[] = {
	{ ASSOCIATE_request,	ASSOCIATE_indication,		ASSOCIATE_response,		ASSOCIATE_confirm,		0 },
	{ DISASSOCIATE_request, DISASSOCIATE_indication,	NOFUNCTION_error,		DISASSOCIATE_confirm,	0 },
	{ NOFUNCTION_error,		BEACON_NOTIFY_indication,	NOFUNCTION_error,		NOFUNCTION_error,		0 },
	{ GET_request,			NOFUNCTION_error,			NOFUNCTION_error,		GET_confirm,			0 },
	{ GTS_request,			GTS_indication,				NOFUNCTION_error,		GTS_confirm,			0 },
	{ NOFUNCTION_error,		ORPHAN_indication,			ORPHAN_response,		NOFUNCTION_error,		0 },
	{ RESET_request,		NOFUNCTION_error,			NOFUNCTION_error,		RESET_confirm,			0 },
	{ RX_ENABLE_request,	NOFUNCTION_error,			NOFUNCTION_error,		RX_ENABLE_confirm,		0 },
	{ SCAN_request,			NOFUNCTION_error,			NOFUNCTION_error,		SCAN_confirm,			0 },
	{ NOFUNCTION_error,		COMM_STATUS_indication,		NOFUNCTION_error,		NOFUNCTION_error,		0 },
	{ SET_request,			NOFUNCTION_error,			NOFUNCTION_error,		SET_confirm,			0 },
	{ START_request,		NOFUNCTION_error,			NOFUNCTION_error,		START_confirm,			0 },
	{ SYNC_request,			NOFUNCTION_error,			NOFUNCTION_error,		NOFUNCTION_error,		0 },
	{ NOFUNCTION_error,		SYNC_LOSS_indication,		NOFUNCTION_error,		NOFUNCTION_error,		0 },
	{ POLL_request,			NOFUNCTION_error,			NOFUNCTION_error,		POLL_confirm,			0 }
};


/*
 *------------------------------------------------------------------------
 * SAP_function - call function to access primatives through the SAP
 *------------------------------------------------------------------------
 */

int
SAP_function(struct sap_info *bindl, struct mib_info *mip, int op, int func)
{
	struct sap_info	*psap = &sapFuncTab[func];
	
	//
	// mip->mi_param holds a pointer to an entry in tabtab that
	// contains the pointers to functions for each table
	//
	switch (op) {
		case SAP_REQUEST:
			return psap->sap_request(bindl, mip);
		case SAP_INDICATION:
			return psap->sap_indication(bindl, mip);
		case SAP_RESPONSE:
			return psap->sap_response(bindl, mip);
		case SAP_CONFIRM:
			return psap->sap_confirm(bindl, mip);
	}
	return SYSERR;
}


int ASSOCIATE_request(struct sap_info *bindl,  struct mib_info *mip) {return 1;}
int ASSOCIATE_indication(struct sap_info *bindl, struct mib_info *mip) { return 0; }
int ASSOCIATE_response(struct sap_info *bindl, struct mib_info *mip) { return 0; }
int ASSOCIATE_confirm(struct sap_info *bindl, struct mib_info *mip) { return 1; }
int DISASSOCIATE_request(struct sap_info *bindl,  struct mib_info *mip) { return 1; }
int DISASSOCIATE_indication(struct sap_info *bindl, struct mib_info *mip) { return 1; }
int DISASSOCIATE_confirm(struct sap_info *bindl, struct mib_info *mip) { return 1; }
int BEACON_NOTIFY_indication(struct sap_info *bindl, struct mib_info *mip) { return 1; }
int GET_request(struct sap_info *bindl,  struct mib_info *mip) { return 1; }
int GET_confirm(struct sap_info *bindl, struct mib_info *mip) { return 1; }
int GTS_request(struct sap_info *bindl,  struct mib_info *mip) { return 0; }
int GTS_indication(struct sap_info *bindl, struct mib_info *mip) { return 0; }
int GTS_confirm(struct sap_info *bindl, struct mib_info *mip) { return 0; }
int ORPHAN_indication(struct sap_info *bindl, struct mib_info *mip) { return 0; }
int ORPHAN_response(struct sap_info *bindl, struct mib_info *mip) { return 0; }
int RESET_request(struct sap_info *bindl,  struct mib_info *mip) { return 0; }
int RESET_confirm(struct sap_info *bindl, struct mib_info *mip) { return 0; }
int RX_ENABLE_request(struct sap_info *bindl,  struct mib_info *mip) { return 0; }
int RX_ENABLE_confirm(struct sap_info *bindl, struct mib_info *mip) { return 0; }
int SCAN_request(struct sap_info *bindl,  struct mib_info *mip) { return 0; }
int SCAN_confirm(struct sap_info *bindl, struct mib_info *mip) { return 0; }
int COMM_STATUS_indication(struct sap_info *bindl, struct mib_info *mip) { return 0; }
int SET_request(struct sap_info *bindl,  struct mib_info *mip) { return 0; }
int SET_confirm(struct sap_info *bindl, struct mib_info *mip) { return 0; }
int START_request(struct sap_info *bindl,  struct mib_info *mip) { return 0; }
int START_confirm(struct sap_info *bindl, struct mib_info *mip) { return 0; }
int SYNC_request(struct sap_info *bindl,  struct mib_info *mip) { return 0; }
int SYNC_LOSS_indication(struct sap_info *bindl, struct mib_info *mip) { return 0; }
int POLL_request(struct sap_info *bindl,  struct mib_info *mip) { return 0; }
int POLL_confirm(struct sap_info *bindl, struct mib_info *mip) { return 0; }


int
NOFUNCTION_error(struct sap_info *bindl, struct mib_info *mip)
{
	return SYSERR;
}


/* The MLME_GET.request primitive requests information about a given PIB attribute. */
/* Table 56 specifies the parameters for the MLME-GET.request primitive. */
int
MLME_GET_request(int pib, int pibIndex)
{
	int id;
	struct mib_info *a;
	char buffer[30];
	
	if ( (id = getMibID(pib)) == SYSERR )	{
		return SYSERR;
	}
	a = &mib[id];
	strcpy_P(buffer, a->mib_name);
	printf("GET: %s pib=0x%02X\n", buffer, pib);
	return OK;
}

/* 7.1.6.2 */
/* The MLME_GET.request primitive requests information about a given PIB attribute. */
/* Table 56 specifies the parameters for the MLME-GET.request primitive. */
//int
//GET_request(struct sap_info *bindl, struct mib_info *mip)
//{
//	memcpy(bindl->value, mip->mib_param, mip->mib_vartype);
//	mgc->PIBAttribute = ((mlme_get_req_t *)mgc)->PIBAttribute;
//	mgc->cmdcode      = MLME_GET_CONFIRM;
//	mgc->status       = status;
//	return MAC_SUCCESS;
//}
/* 7.1.6.1 */
/* The MLME-GET.confirm primitive reports the results of an information request from the PIB. */
/* Table 57 specifies the parameters for the MLME-GET.confirm primitive. */
//GET_confirm(ReturnStatus_t status, PIBAttribute_t pib, PIBAttributIndex_t pibIndex, PIBAttributeValue_t value)
//{

//}


//ASSOCIATE_request(requestParameter_t *req)
//{
//	frame_t *frame;
/*
 * On receipt of the ASSOCIATE.request primitive, the MLME of an unassociated device first updates
 * the appropriate PHY and MAC PIB attributes, as described in 5.1.3.1, and then generates an association
 * request command, as defined in 5.3.1.
 * The SecurityLevel parameter specifies the level of security to be applied to the association request command
 * frame. Typically, the association request command should not be implemented using security. However, if
 * the device requesting association shares a key with the coordinator, then security may be specified.
 */
//	if (isAssociated())	{
//		primativeError = BAD_ASSOCIATION_REQUEST;
//		return SYSERR;
//	}
//	if (!req->CoordPANId)	{
//		primativeError = BAD_PAN_ID;
//		return SYSERR;
//	}
//	phyCurrentChannel = req->ChannelNumber;
//	phyCurrentPage = req->ChannelPage;
//	macPANId = req->CoordPANId;

//	if ( (frame = (frame_t *)getbuf(MLME_PoolID) == SYSERR))	{
//		primativeError = BAD_POOL_ID;
//		return SYSERR;
//	}
//	makeCommand(frame);
//	return OK;
//}

