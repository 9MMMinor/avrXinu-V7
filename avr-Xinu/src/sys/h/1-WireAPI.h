//
//  1-WireAPI.h
//  1-Wire API
//
//  Created by Michael Minor on 11/3/14.
//
//

#ifndef __OW_1_WireAPI_h
#define __OW_1_WireAPI_h

// global search state
extern unsigned char ROM_NO[8];
extern int LastDiscrepancy;
extern int LastFamilyDiscrepancy;
extern int LastDeviceFlag;
extern unsigned char crc8;

// 1-Wire Devices
#ifndef DS18S20_h
#include <DS18S20.h>
#endif
#ifndef DS2438_h
#include <DS2438.h>
#endif

// 1-Wire API function prototypes
#ifdef USE_singleGPIO_1WIRE
void OW_Enable(void);
void OW_Disable(void);
#endif

// Hardware dependent (SingleGPIO, DS2482 bridge, etc)
int OWReset(void);
int OWSearch(void);
void OWWriteByte(unsigned char sendbyte);
unsigned char OWReadByte(void);
unsigned char OWReadBit(void);
void OWWriteBit(unsigned char sendbit);

unsigned char OWTouchBit(unsigned char sendbit);

// Hardware independent
unsigned char OWTouchByte(unsigned char sendbyte);
void OWBlock(unsigned char *tran_buf, int tran_len);
void OWFamilySkipSetup(void);
int OWFirst(void);
uint8_t OWFirstType(unsigned char type);
int OWNext(void);
uint8_t OWNextType(unsigned char type);
int  OWVerify(void);
int OWAccessSerialNumber(unsigned char *LaserROM);
int OWSerialNumber(unsigned char *LaserROM);
void OWTargetSetup(unsigned char family_code);
unsigned char calc_crc8(unsigned char data);

#endif
