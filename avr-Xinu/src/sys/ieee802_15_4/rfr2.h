//
//  rfr2.h
//  radio-15-4
//
//  Created by Michael M Minor on 10/10/13.
//
//

#ifndef radio_15_4_rfr2_h
#define radio_15_4_rfr2_h

// TRX_STATUS4:0



/******************************************************************************
 Types section
 ******************************************************************************/
//! PHY PIB attributes.
typedef struct
{
	uint8_t         phyCurrentChannel;		// Register: PHY_CC_CCA
	uint32_t        phyChannelsSupported;	// Just one row for current page.
//	uint8_t			phyTXPowerTolerence;	// 1_dB, 3_dB, or 6_dB
	int8_t          phyTXPower;				// Register: PHY_TX_PWR
	uint8_t			phyCCAMode;
	uint8_t         phyCurrentPage;			// This is the current PHY channel page.
	uint16_t        phyMaxFrameDuration;
	uint8_t         phySHRDuration;			// The duration of the synchronization header (SHR)
//	uint8_t         phySymbolsPerOctet;
} PhyPibAttr_t;

//! MAC PIB attributes.
typedef struct
{
	uint32_t        ackWaitDuration;
	Bool            associatedPanCoord;
#ifdef _FFD_
	bool            associationPermit;
#endif // _FFD_
	Bool            autoRequest;
	//bool          battLifeExt;
	//uint8_t       battLifeExtPeriods;
#ifdef _FFD_
	uint8_t         *beaconPayload;
	uint8_t         beaconPayloadLength;
	//uint8_t       beaconOrder;
	//uint32_t      beaconTxTime;
	uint8_t         bsn;
#endif // _FFD_
	ExtAddr_t       coordExtAddr;
	ShortAddr_t     coordShortAddr;
	uint8_t         dsn;
	//bool          gtsPermit;
	uint8_t         maxBe;
	uint8_t         maxCsmaBackoffs;
	//uint8_t       maxFrameTotalWaitTime;
	uint8_t         maxFrameRetries;
	uint8_t         minBe;
	//uint8_t       minLifsPeriod;
	//uint8_t       minSifsPeriod;
	PanId_t         panId;
#ifdef _FFD_
	//bool          promiscuousMode;
#endif // _FFD_
	uint8_t         responseWaitTime;
	Bool            rxOnWhenIdle;
	Bool            securityEnabled;
	ShortAddr_t     shortAddr;
#ifdef _FFD_
	//uint8_t       superframeOrder;
	//uint8_t       syncSymbolOffset;
#endif // _FFD_
	//bool          timestampSupported;
#ifdef _FFD_
	//uint16_t        transactionPersistenceTime;
#endif // _FFD_
	/************************************************************/
	// Additional attributes. NOT described in the standard.
	ExtAddr_t       extAddr;  // Device own extended address.
	Bool            panCoordinator;
	// For PanServer needs.
#ifdef _FFD_
	Bool            beaconTxEnable;
#endif //_FFD_
	/** Duration in milliseconds of maximum length frame transferring. */
	uint8_t         maxFrameTransmissionTime;
} MacPibAttr_t;

//! MAC and PHY PIB attributes.
typedef struct
{
	MacPibAttr_t macAttr;
	PhyPibAttr_t phyAttr;
} PIB_t;

#endif /*_MACENVMEM_H*/

