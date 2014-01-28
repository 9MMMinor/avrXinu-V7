//
//  DS2482.h
//  TWIDriver
//
//  Created by Michael Minor on 12/16/11.
//  Copyright (c) 2011 Michael M Minor. All rights reserved.
//

//	im1WP 1-Wire Module
//	1-Wire to I2C Bridge with DS2482-100 chip.
//	www.modtronix.com/products/im1wp
//	1-Wire Interface Module with I2C bus, 3.0V-5.5V, Lead Free.

#ifndef TWIDriver_DS2482_h
#define TWIDriver_DS2482_h

// Search state
extern unsigned char ROM_NO[8];
extern int LastDiscrepancy;
extern int LastFamilyDiscrepancy;
extern int LastDeviceFlag;
extern unsigned char crc8;

// DS2482 state
extern unsigned char I2C_address;
extern int short_detected;
extern int c1WS, cSPU, cPPM, cAPU;

#define DS2482_SLAVEADDRESS	0x30>>1
#define POLL_LIMIT 200

// DS2482 commands
#define DS2482_CMD_DRST		0xF0	// Command: Device Reset
#define DS2482_CMD_WCFG		0xD2	// Command: Write Configuration
#define DS2482_CMD_CHSL   	0xC3	// Command: Channel Select (-800 only)
#define DS2482_CMD_SRP		0xE1	// Command: Set Read Pointer
#define DS2482_CMD_1WRS		0xB4	// Command: 1-Wire Bus Reset
#define DS2482_CMD_1WWB		0xA5	// Command: 1-Wire Write Byte
#define DS2482_CMD_1WRB		0x96	// Command: 1-Wire Read Byte
#define DS2482_CMD_1WSB		0x87	// Command: 1-Wire Single Bit
#define DS2482_CMD_1WT		0x78	// Command: 1-Wire Triplet

// DS2482 config bits
#define DS2482_CONFIG_APU	0x01
#define DS2482_CONFIG_PPM	0x02
#define DS2482_CONFIG_SPU	0x04
#define DS2482_CONFIG_1WS	0x08

// DS2482 status bits 
#define DS2482_STATUS_1WB	0x01
#define DS2482_STATUS_PPD	0x02
#define DS2482_STATUS_SD	0x04
#define DS2482_STATUS_LL	0x08
#define DS2482_STATUS_RST	0x10
#define DS2482_STATUS_SBR	0x20
#define DS2482_STATUS_TSB	0x40
#define DS2482_STATUS_DIR	0x80

// DS2482 register select
#define DS2482_SELECT_STATUSREGISTER 		0xF0
#define DS2482_SELECT_READDATAREGISTER		0xE1
#define DS2482_SELECT_CONFIGURATIONREGISTER	0xC3

// API mode bit flags
#define MODE_STANDARD                  0x00
#define MODE_OVERDRIVE                 0x01
#define MODE_STRONG                    0x02

// DS2482 Basic Function prototypes
int DS2482_detect(unsigned char addr);
unsigned char DS2482_search_triplet(int search_direction);
int DS2482_write_config(unsigned char config);
int DS2482_reset();
int DS2482_channel_select(int channel);

// 1-Wire API for DS2482 function prototypes
int OWReset(void);
void OWWriteByte(unsigned char sendbyte);
unsigned char OWReadByte(void);
unsigned char OWTouchByte(unsigned char sendbyte);
unsigned char OWTouchBit(unsigned char sendbit);
void OWWriteBit(unsigned char sendbit);
unsigned char OWReadBit(void);
void OWBlock(unsigned char *tran_buf, int tran_len);
int OWFirst();
int OWNext();
int  OWVerify();
void OWTargetSetup(unsigned char family_code);
void OWFamilySkipSetup();
int OWSearch();

// Extended 1-Wire functions
int OWSpeed(int new_speed);
int OWLevel(int level);
int OWWriteBytePower(int sendbyte);
int OWReadBitPower(int applyPowerResponse);

// Helper functions
unsigned char calc_crc8(unsigned char data);

#endif
