
/*
 * Copyright (c) 2006-2008 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdint.h>
#include <avr/io.h>

/**
 * \addtogroup net
 *
 * @{
 */
/**
 * \addtogroup net_driver
 *
 * @{
 */
/**
 * \addtogroup net_driver_enc28j60
 *
 * @{
 */
/**
 * \file
 * Microchip ENC28J60 I/O header (license: GPLv2)
 *
 * \author Roland Riegel
 */
 
 
/*
 *  enc28j60.h
 *  Xinu/src/sys/h
 *
 *  modified by Michael Minor on 7/10/10 for avr-Xinu.
 *
 */
 
 
#define WriteControlRegister(register,data) WriteControlRegisterCommandSequence(ENC_WCR|(register),(data))
#define ReadEthRegister(register) ReadEthRegisterCommandSequence(ENC_RCR|(register))
#define ReadMacRegister(register) ReadMacRegisterCommandSequence(ENC_RCR|(register))
#define ReadMIIRegister(register) ReadMacRegisterCommandSequence(ENC_RCR|(register))
#define BitFieldClear(register, data) WriteControlRegisterCommandSequence(ENC_BFC|(register),(data))
#define BitFieldSet(register, data) WriteControlRegisterCommandSequence(ENC_BFS|(register),(data))
#define ReadBufferMemory(buf, len) ReadBufferMemoryCommandSequence((buf),(len))
#define WriteBufferMemory(buf, len) WriteBufferMemoryCommandSequence((buf),(len))
#define waitspi() while(!(SPSR&(1<<SPIF)))


/* avr-Xinu ENC28J60 Configuration	*/
/* RST (Pin-4 J9): System Reset		*/
/* NET-INT (Pin-2 J9: INT2/PB2 for ATmega1284P */
#define NET_INT PB2
#define NET_INT_PORT PORTB


#define ENC28J60_FULL_DUPLEX 1

/* SPI Opcode Set for the ENC28J60 <<5 for below */
#define RCR 0x00	/* Read Control Register */
#define RBM 0x01	/* Read Buffer Memory */
#define WCR 0x02	/* Write Control Memory */
#define WBM 0x03	/* Write Buffer Memory */
#define BFS 0x04	/* Bit Field Set */
#define BFC 0x05	/* Bit Field Clear */
#define SRC 0x07	/* System Reset Command */

/* SPI Instruction Set for the ENC28J60	*/
#define ENC_RCR	0x00	/* Read Control Register | (address) */
#define ENC_RBM	0x3A	/* Read Buffer Memory 1<<5|0x1A */
#define ENC_WCR	0x40	/* Write Control Memory | (address) */
#define ENC_WBM	0x7A	/* Write Buffer Memory 3<<5|0x1A */
#define ENC_BFS	0x80	/* Bit Field Set | (address) */
#define ENC_BFC	0xA0	/* Bit Field Clear | (address) */
#define ENC_SRC	0xFF	/* System Reset Command */
/* SPI operation codes */
#define ENC28J60_READ_CTRL_REG       ENC_RCR
#define ENC28J60_READ_BUF_MEM        ENC_RBM
#define ENC28J60_WRITE_CTRL_REG      ENC_WCR
#define ENC28J60_WRITE_BUF_MEM       ENC_WBM
#define ENC28J60_BIT_FIELD_SET       ENC_BFS
#define ENC28J60_BIT_FIELD_CLR       ENC_BFC
#define ENC28J60_SOFT_RESET          ENC_SRC




/* partitioning of the internal 8kB tx/rx buffers  */
#define RX_START 0x0000     /* do not change this! */
#define TX_END (0x2000 - 1) /* do not change this! */
#define TX_START 0x1a00
#define RX_END (TX_START - 1)

#define RSV_SIZE 6


/* bank independent */
#define EIE			0x1b
#define EIR			0x1c
#define ESTAT		0x1d
#define ECON2		0x1e
#define ECON1		0x1f

/* bank 0 */
#define ERDPTL		0x00
#define ERDPTH		0x01
#define EWRPTL		0x02
#define EWRPTH		0x03
#define ETXSTL		0x04
#define ETXSTH		0x05
#define ETXNDL		0x06
#define ETXNDH		0x07
#define ERXSTL		0x08
#define ERXSTH		0x09
#define ERXNDL		0x0a
#define ERXNDH		0x0b
#define ERXRDPTL	0x0c
#define ERXRDPTH	0x0d
#define ERXWRPTL	0x0e
#define ERXWRPTH	0x0f
#define EDMASTL		0x10
#define EDMASTH		0x11
#define EDMANDL		0x12
#define EDMANDH		0x13
#define EDMADSTL	0x14
#define EDMADSTH	0x15
#define EDMACSL		0x16
#define EDMACSH		0x17

/* bank 1 */
#define EHT0		0x00
#define EHT1		0x01
#define EHT2		0x02
#define EHT3		0x03
#define EHT4		0x04
#define EHT5		0x05
#define EHT6		0x06
#define EHT7		0x07
#define EPMM0		0x08
#define EPMM1		0x09
#define EPMM2		0x0a
#define EPMM3		0x0b
#define EPMM4		0x0c
#define EPMM5		0x0d
#define EPMM6		0x0e
#define EPMM7		0x0f
#define EPMCSL		0x10
#define EPMCSH		0x11
#define EPMOL		0x14
#define EPMOH		0x15
#define EWOLIE		0x16
#define EWOLIR		0x17
#define ERXFCON		0x18
#define EPKTCNT		0x19

/* bank 2 */
#define MACON1		0x00
#define MACON2		0x01
#define MACON3		0x02
#define MACON4		0x03
#define MABBIPG		0x04
#define MAIPGL		0x06
#define MAIPGH		0x07
#define MACLCON1	0x08
#define MACLCON2	0x09
#define MAMXFLL		0x0a
#define MAMXFLH		0x0b
#define MAPHSUP		0x0d
#define MICON		0x11
#define MICMD		0x12
#define MIREGADR	0x14
#define MIWRL		0x16
#define MIWRH		0x17
#define MIRDL		0x18
#define MIRDH		0x19
                
/* bank 3 */
#define MAADR1		0x00
#define MAADR0		0x01
#define MAADR3		0x02
#define MAADR2		0x03
#define MAADR5		0x04
#define MAADR4		0x05
#define EBSTSD		0x06
#define EBSTCON		0x07
#define EBSTCSL		0x08
#define EBSTCSH		0x09
#define MISTAT		0x0a
#define EREVID		0x12
#define ECOCON		0x15
#define EFLOCON		0x17
#define EPAUSL		0x18
#define EPAUSH		0x19

/* phy */
#define PHCON1		0x00
#define PHSTAT1		0x01
#define PHID1		0x02
#define PHID2		0x03
#define PHCON2		0x10
#define PHSTAT2		0x11
#define PHIE		0x12
#define PHIR		0x13
#define PHLCON		0x14

/* EIE */
#define EIE_INTIE       7
#define EIE_PKTIE       6
#define EIE_DMAIE       5
#define EIE_LINKIE      4
#define EIE_TXIE        3
#define EIE_WOLIE       2
#define EIE_TXERIE      1
#define EIE_RXERIE      0

/* EIR */
#define EIR_PKTIF       6
#define EIR_DMAIF       5
#define EIR_LINKIF      4
#define EIR_TXIF        3
#define EIR_WOLIF       2
#define EIR_TXERIF      1
#define EIR_RXERIF      0

/* ESTAT */
#define ESTAT_INT       7
#define ESTAT_LATECOL   4
#define ESTAT_RXBUSY    2
#define ESTAT_TXABRT    1
#define ESTAT_CLKRDY    0

/* ECON2 */
#define ECON2_AUTOINC   7
#define ECON2_PKTDEC    6
#define ECON2_PWRSV     5
#define ECON2_VRPS      4

/* ECON1 */
#define ECON1_TXRST     7
#define ECON1_RXRST     6
#define ECON1_DMAST     5
#define ECON1_CSUMEN    4
#define ECON1_TXRTS     3
#define ECON1_RXEN      2
#define ECON1_BSEL1     1
#define ECON1_BSEL0     0

/* EWOLIE */
#define EWOLIE_UCWOLIE  7
#define EWOLIE_AWOLIE   6
#define EWOLIE_PMWOLIE  4
#define EWOLIE_MPWOLIE  3
#define EWOLIE_HTWOLIE  2
#define EWOLIE_MCWOLIE  1
#define EWOLIE_BCWOLIE  0

/* EWOLIR */
#define EWOLIR_UCWOLIF  7
#define EWOLIR_AWOLIF   6
#define EWOLIR_PMWOLIF  4
#define EWOLIR_MPWOLIF  3
#define EWOLIR_HTWOLIF  2
#define EWOLIR_MCWOLIF  1
#define EWOLIR_BCWOLIF  0

/* ERXFCON */
#define ERXFCON_UCEN    7
#define ERXFCON_ANDOR   6
#define ERXFCON_CRCEN   5
#define ERXFCON_PMEN    4
#define ERXFCON_MPEN    3
#define ERXFCON_HTEN    2
#define ERXFCON_MCEN    1
#define ERXFCON_BCEN    0

/* MACON1 */
#define MACON1_LOOPBK   4
#define MACON1_TXPAUS   3
#define MACON1_RXPAUS   2
#define MACON1_PASSALL  1
#define MACON1_MARXEN   0

/* MACON2 */
#define MACON2_MARST    7
#define MACON2_RNDRST   6
#define MACON2_MARXRST  3
#define MACON2_RFUNRST  2
#define MACON2_MATXRST  1
#define MACON2_TFUNRST  0

/* MACON3 */
#define MACON3_PADCFG2  7
#define MACON3_PADCFG1  6
#define MACON3_PADCFG0  5
#define MACON3_TXCRCEN  4
#define MACON3_PHDRLEN  3
#define MACON3_HFRMEN   2
#define MACON3_FRMLNEN  1
#define MACON3_FULDPX   0

/* MACON4 */
#define MACON4_DEFER    6
#define MACON4_BPEN     5
#define MACON4_NOBKOFF  4
#define MACON4_LONGPRE  1
#define MACON4_PUREPRE  0

/* MAPHSUP */
#define MAPHSUP_RSTINTFC 7
#define MAPHSUP_RSTRMII  3

/* MICON */
#define MICON_RSTMII    7

/* MICMD */
#define MICMD_MIISCAN   1
#define MICMD_MIIRD     0

/* EBSTCON */
#define EBSTCON_PSV2    7
#define EBSTCON_PSV1    6
#define EBSTCON_PSV0    5
#define EBSTCON_PSEL    4
#define EBSTCON_TMSEL1  3
#define EBSTCON_TMSEL0  2
#define EBSTCON_TME     1
#define EBSTCON_BISTST  0

/* MISTAT */
#define MISTAT_NVALID   2
#define MISTAT_SCAN     1
#define MISTAT_BUSY     0

/* ECOCON */
#define ECOCON_COCON2   2
#define ECOCON_COCON1   1
#define ECOCON_COCON0   0

/* EFLOCON */
#define EFLOCON_FULDPXS 2
#define EFLOCON_FCEN1   1
#define EFLOCON_FCEN0   0

/* PHCON1 */
#define PHCON1_PRST     15
#define PHCON1_PLOOPBK  14
#define PHCON1_PPWRSV   11
#define PHCON1_PDPXMD   8

/* PHSTAT1 */
#define PHSTAT1_PFDPX   12
#define PHSTAT1_PHDPX   11
#define PHSTAT1_LLSTAT  2
#define PHSTAT1_JBSTAT  1

/* PHCON2 */
#define PHCON2_FRCLNK   14
#define PHCON2_TXDIS    13
#define PHCON2_JABBER   10
#define PHCON2_HDLDIS   8

/* PHSTAT2 */
#define PHSTAT2_TXSTAT  13
#define PHSTAT2_RXSTAT  12
#define PHSTAT2_COLSTAT 11
#define PHSTAT2_LSTAT   10
#define PHSTAT2_DPXSTAT 9
#define PHSTAT2_PLRITY  4

/* PHIE */
#define PHIE_PLINKIE    4
#define PHIE_PGEIE      1

/* PHIR */
#define PHIR_LINKIF     4
#define PHIR_PGIF       2

/* PHLCON */
#define PHLCON_LACFG3   11
#define PHLCON_LACFG2   10
#define PHLCON_LACFG1   9
#define PHLCON_LACFG0   8
#define PHLCON_LBCFG3   7
#define PHLCON_LBCFG2   6
#define PHLCON_LBCFG1   5
#define PHLCON_LBCFG0   4
#define PHLCON_LFRQ1    3
#define PHLCON_LFRQ0    2
#define PHLCON_STRCH    1

/* Receive Status Vector bits 16-31 in a single uint16_t */
#define RSV_LongEvent			0
#define RSV_CarrierEvent		2
#define RSV_CRCError			4
#define RSV_LengthCheck			5
#define RSV_LengthOut			6
#define RSV_ReceivedOk			7
#define RSV_ReceiveMulticast	8
#define RSV_ReceiveBroadcast	9
#define RSV_DribbleNibble		10
#define RSV_ReceiveControl		11
#define RSV_ReceivePause		12
#define RSV_ReceiveUnknown		13
#define RSV_ReceiveVLAN			14
#define RSV_Zero				15


int enc28j60_init(const uint8_t *);
char ReadEthRegisterCommandSequence(char);
char ReadMacRegisterCommandSequence(char);
void WriteControlRegisterCommandSequence(char, char);
void WriteBufferMemoryCommandSequence(char *, int);
char ReadBufferByte(void);
void WriteBufferByte(uint8_t data);
void ReadBufferMemoryCommandSequence(char *, int);
uint16_t ReadPhyRegister(uint8_t);
void WritePhyRegister(uint8_t, uint16_t);
void select_Bank(uint8_t);
int link_Up(void);

