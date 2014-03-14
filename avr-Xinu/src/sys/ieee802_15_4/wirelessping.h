//
//  wirelessping.h
//  802_15_4_Mib
//
//  Created by Michael Minor on 3/11/14.
//
//

#ifndef _02_15_4_Mib_wirelessping_h
#define _02_15_4_Mib_wirelessping_h

#define TIME 10
#define PINGAPP 0xab

typedef struct pingControlField	{
	uint8_t replyRequested:1;
	uint8_t includeTimeVal:1;
	uint8_t res:6;
} pingControlField_t;

typedef struct pingPacket	{
	uint8_t	hdr;
	pingControlField_t pcf;
	uint8_t data[8];
} pingPacket_t;


#endif
