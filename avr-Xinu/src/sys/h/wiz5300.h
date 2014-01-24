/*
 *  wiz5300.h
 *
 *
 * This file defines the memory map and values of W5300 registers
 * and defines the basic I/O functions to access W5300 registers. 
 *
 */

#include <stdint.h>
#include <avr/io.h>
typedef volatile uint8_t vuint8_t;

#define __DEF_W5300_MAP_BASE__ 0x8000
#define COMMON_REG_BASE   __DEF_W5300_MAP_BASE__          /**< The base address of COMMON_REG */
#define SOCKET_REG_BASE   __DEF_W5300_MAP_BASE__ + 0x0200 /**< The base address of SOCKET_REG */
#define SOCKET_REG_SIZE    0x40     // SOCKET Regsiter Count per Channel
#define	MAX_SOCK_NUM		8 
 
/*
 *  Mode register. 
 */
#define MR		_SFR_MEM16(__DEF_W5300_MAP_BASE__)
#define MRH		_SFR_MEM8(__DEF_W5300_MAP_BASE__)
#define MRL		_SFR_MEM8(__DEF_W5300_MAP_BASE__+1)

/*
 *  Indirect mode address register.
 */ 
#define IDM_AR	_SFR_MEM16(__DEF_W5300_MAP_BASE__ + 0x02)
#define IDM_ARH	_SFR_MEM8(__DEF_W5300_MAP_BASE__ + 0x02)
#define IDM_ARL	_SFR_MEM8(__DEF_W5300_MAP_BASE__ + 0x03)

/*
 *  Indirect mode data register.
 */ 
#define IDM_DR	_SFR_MEM16(__DEF_W5300_MAP_BASE__ + 0x04)
#define IDM_DRH	_SFR_MEM8(__DEF_W5300_MAP_BASE__ + 0x04)
#define IDM_DRL	_SFR_MEM8(__DEF_W5300_MAP_BASE__ + 0x05)

/*
 *  Interrupt register
 */
#define IR	_SFR_MEM16(COMMON_REG_BASE + 0x02)
#define IRH	_SFR_MEM8(COMMON_REG_BASE + 0x02)
#define IRL	_SFR_MEM8(COMMON_REG_BASE + 0x03)

/*
 *  Interrupt mask register
 */
#define IMR		_SFR_MEM16(COMMON_REG_BASE + 0x04)
#define IMRH	_SFR_MEM8(COMMON_REG_BASE + 0x04)
#define IMRL	_SFR_MEM8(COMMON_REG_BASE + 0x05)


//#define ICFGR		_SFR_MEM16(COMMON_REG_BASE + 0x06)
//#define ICFGRH	_SFR_MEM8(COMMON_REG_BASE + 0x06)
//#define ICFGRL	_SFR_MEM8(COMMON_REG_BASE + 0x07)

/*
 *  Source hardware address register
 */
#define SHAR	_SFR_MEM16(COMMON_REG_BASE + 0x08)
#define SHAR0	_SFR_MEM8(COMMON_REG_BASE + 0x08 + 0)
#define SHAR1	_SFR_MEM8(COMMON_REG_BASE + 0x08 + 1)
#define SHAR2	_SFR_MEM8(COMMON_REG_BASE + 0x08 + 2)
#define SHAR3	_SFR_MEM8(COMMON_REG_BASE + 0x08 + 3)
#define SHAR4	_SFR_MEM8(COMMON_REG_BASE + 0x08 + 4)
#define SHAR5	_SFR_MEM8(COMMON_REG_BASE + 0x08 + 5)


/*
 *  Gateway IP address register
 */
#define GAR		_SFR_MEM16(COMMON_REG_BASE + 0x10)
#define GAR0	_SFR_MEM8(COMMON_REG_BASE + 0x10 + 0)
#define GAR1	_SFR_MEM8(COMMON_REG_BASE + 0x10 + 1)
#define GAR2	_SFR_MEM8(COMMON_REG_BASE + 0x10 + 2)
#define GAR3	_SFR_MEM8(COMMON_REG_BASE + 0x10 + 3)

/*
 *  Subnet mask register
 */
#define SUBR	_SFR_MEM16(COMMON_REG_BASE + 0x14)
#define SUBR0	_SFR_MEM8(COMMON_REG_BASE + 0x14 + 0)
#define SUBR1	_SFR_MEM8(COMMON_REG_BASE + 0x14 + 1)
#define SUBR2	_SFR_MEM8(COMMON_REG_BASE + 0x14 + 2)
#define SUBR3	_SFR_MEM8(COMMON_REG_BASE + 0x14 + 3)

/*
 *  Source IP address register
 */
#define SIPR	_SFR_MEM16(COMMON_REG_BASE + 0x18)
#define SIPR0	_SFR_MEM8(COMMON_REG_BASE + 0x18 + 0)
#define SIPR1	_SFR_MEM8(COMMON_REG_BASE + 0x18 + 1)
#define SIPR2	_SFR_MEM8(COMMON_REG_BASE + 0x18 + 2)
#define SIPR3	_SFR_MEM8(COMMON_REG_BASE + 0x18 + 3)

/*
 *  Retransmittion timeout-period register 
 */
#define RTR		_SFR_MEM16(COMMON_REG_BASE + 0x1C)
#define RTRH	_SFR_MEM8(COMMON_REG_BASE + 0x1C + 0)
#define RTRL	_SFR_MEM8(COMMON_REG_BASE + 0x1C + 1)

/*
 *  Retransmittion retry count reigster
 */
#define RCR		_SFR_MEM16(COMMON_REG_BASE + 0x1E)
#define RCRH	_SFR_MEM8(COMMON_REG_BASE + 0x1E + 0)
#define RCRL	_SFR_MEM8(COMMON_REG_BASE + 0x1E + 1)

/*
 *  Tx memory size of each SOCKET reigster
 */
#define TMSR0		_SFR_MEM8(COMMON_REG_BASE + 0x20)
#define TMSR1		_SFR_MEM8(COMMON_REG_BASE + 0x20 + 1)
#define TMSR2		_SFR_MEM8(COMMON_REG_BASE + 0x20 + 2)
#define TMSR3		_SFR_MEM8(COMMON_REG_BASE + 0x20 + 3)
#define TMSR4		_SFR_MEM8(COMMON_REG_BASE + 0x20 + 4)
#define TMSR5		_SFR_MEM8(COMMON_REG_BASE + 0x20 + 5)
#define TMSR6		_SFR_MEM8(COMMON_REG_BASE + 0x20 + 6)
#define TMSR7		_SFR_MEM8(COMMON_REG_BASE + 0x20 + 7)


/*
 *  Rx memory size of each SOCKET reigster
 */
#define RMSR0		_SFR_MEM8(COMMON_REG_BASE + 0x28)
#define RMSR1		_SFR_MEM8(COMMON_REG_BASE + 0x28 + 1)
#define RMSR2		_SFR_MEM8(COMMON_REG_BASE + 0x28 + 2)
#define RMSR3		_SFR_MEM8(COMMON_REG_BASE + 0x28 + 3)
#define RMSR4		_SFR_MEM8(COMMON_REG_BASE + 0x28 + 4)
#define RMSR5		_SFR_MEM8(COMMON_REG_BASE + 0x28 + 5)
#define RMSR6		_SFR_MEM8(COMMON_REG_BASE + 0x28 + 6)
#define RMSR7		_SFR_MEM8(COMMON_REG_BASE + 0x28 + 7)



/*
 *  Memory type register
 */
#define MTYPER		_SFR_MEM16(COMMON_REG_BASE + 0x30)
#define MTYPERH		_SFR_MEM8(COMMON_REG_BASE + 0x30)
#define MTYPERL		_SFR_MEM8(COMMON_REG_BASE + 0x30 + 1)

/*
 *  Authentication type register
 */
#define PATR		_SFR_MEM16(COMMON_REG_BASE + 0x32)
#define PATRH		_SFR_MEM8(COMMON_REG_BASE + 0x32)
#define PATRL		_SFR_MEM8(COMMON_REG_BASE + 0x32 + 1)

//#define PPPALGOR      _SFR_MEM16(COMMON_REG_BASE + 0x34)
//#define PPPALGORH     _SFR_MEM8(COMMON_REG_BASE + 0x34)
//#define PPPALGORL     _SFR_MEM8(COMMON_REG_BASE + 0x34 + 1)

/*
 *  PPP link control protocol request timer register
 */
#define PTIMER		_SFR_MEM16(COMMON_REG_BASE + 0x36)
#define PTIMERH		_SFR_MEM8(COMMON_REG_BASE + 0x36)
#define PTIMERL		_SFR_MEM8(COMMON_REG_BASE + 0x36 + 1)

/*
 *  PPP LCP magic number register
 */
#define PMAGICR		_SFR_MEM16(COMMON_REG_BASE + 0x38)
#define PMAGICRH	_SFR_MEM8(COMMON_REG_BASE + 0x38)
#define PMAGICRL	_SFR_MEM8(COMMON_REG_BASE + 0x38 + 1)

//#define PSTATER		_SFR_MEM16(COMMON_REG_BASE + 0x3A)
//#define PSTATERH		_SFR_MEM8(COMMON_REG_BASE + 0x3A)
//#define PSTATERL		_SFR_MEM8(COMMON_REG_BASE + 0x3A + 1)

/*
 *  PPPoE session ID register
 */
#define PSIDR		_SFR_MEM16(COMMON_REG_BASE + 0x3C)
#define PSIDRH		_SFR_MEM8(COMMON_REG_BASE + 0x3C)
#define PSIDRL		_SFR_MEM8(COMMON_REG_BASE + 0x3C + 1)

/*
 *  PPPoE destination hardware address register
 */
#define PDHAR		_SFR_MEM16(COMMON_REG_BASE + 0x40)
#define PDHAR0		_SFR_MEM8(COMMON_REG_BASE + 0x40 + 0)
#define PDHAR1		_SFR_MEM8(COMMON_REG_BASE + 0x40 + 1)
#define PDHAR2		_SFR_MEM8(COMMON_REG_BASE + 0x40 + 2)
#define PDHAR3		_SFR_MEM8(COMMON_REG_BASE + 0x40 + 3)
#define PDHAR4		_SFR_MEM8(COMMON_REG_BASE + 0x40 + 4)
#define PDHAR5		_SFR_MEM8(COMMON_REG_BASE + 0x40 + 5)

/*
 *  Unreachable IP address register 
 *
 * RESET : 0.0.0.0 \n 
 * When trying to transmit UDP data to destination port number which is not open, 
 * W5300 can receive ICMP (Destination port unreachable) packet.
 * In this case, IR(IR_DPUR) becomes '1'. 
 * And destination IP address and unreachable port number of ICMP packet
 * can be acquired through UIPR and UPORTR.
 */
#define UIPR		_SFR_MEM16(COMMON_REG_BASE + 0x48)
#define UIPR0		_SFR_MEM8(COMMON_REG_BASE + 0x48)
#define UIPR1		_SFR_MEM8(COMMON_REG_BASE + 0x48 + 1)
#define UIPR2		_SFR_MEM8(COMMON_REG_BASE + 0x48 + 2)
#define UIPR3		_SFR_MEM8(COMMON_REG_BASE + 0x48 + 3)

/*
 *  Unreachable port number register
 */
#define UPORTR		_SFR_MEM16(COMMON_REG_BASE + 0x4C)
#define UPORTRH		_SFR_MEM8(COMMON_REG_BASE + 0x4C)
#define UPORTRL		_SFR_MEM8(COMMON_REG_BASE + 0x4C + 1)

/*
 *  Fragment MTU register
 */
#define FMTUR		_SFR_MEM16(COMMON_REG_BASE + 0x4E)
#define FMTURH		_SFR_MEM8(COMMON_REG_BASE + 0x4E)
#define FMTURL		_SFR_MEM8(COMMON_REG_BASE + 0x4E + 1)

/*
 *  PIN 'BRDYn' configure register
 */
#define Pn_BRDYR(n)		_SFR_MEM16(COMMON_REG_BASE + 0x60 + (n)*4)
#define Pn_BRDYRH(n)    _SFR_MEM8(COMMON_REG_BASE + 0x60 + (n)*4)
#define Pn_BRDYRL(n)    _SFR_MEM8(COMMON_REG_BASE + 0x60 + (n)*4 + 1)

/*
 *  PIN 'BRDYn' buffer depth Register
 */
#define Pn_BDPTHR(n)		_SFR_MEM16(COMMON_REG_BASE + 0x60 + (n)*4 + 2)
#define Pn_BDPTHRH(n)		_SFR_MEM8(COMMON_REG_BASE + 0x60 + (n)*4 + 2)
#define Pn_BDPTHRL(n)		_SFR_MEM8(COMMON_REG_BASE + 0x60 + (n)*4 + 3)

/*
 *  W5300 identification register
 */
#define IDR		_SFR_MEM16(COMMON_REG_BASE + 0xFE)
#define IDRH	_SFR_MEM8(COMMON_REG_BASE + 0xFE)
#define IDRL	_SFR_MEM8(COMMON_REG_BASE + 0xFE + 1)


/************************************/
/* The bit of MR regsiter defintion */
/************************************/
/* MRH */
#define MR_DBW		7		/**< Data bus width bit of MR. */
#define MR_MPF		6		/**< Mac layer pause frame bit of MR. */
#define MR_WDF		3		/**< Write data fetch time bit of  MR. */
#define MR_RDH		2		/**< Read data hold time bit of MR. */
#define MR_FS		0		/**< FIFO swap bit of MR. */
/* MRL */
#define MR_RST		7		/**< S/W reset bit of MR. */
#define MR_MT		5		/**< Memory test bit of MR. */
#define MR_PB		4		/**< Ping block bit of MR. */
#define MR_PPPoE	3		/**< PPPoE bit of MR. */
#define MR_DBS		2		/**< Data bus swap of MR. */
#define MR_IND		0		/**< Indirect mode bit of MR. */


/****************************************/ 
/* The bit of IR IMR regsiter defintion */ 
/****************************************/ 
#define IR_IPCF			7		/**< IP conflict bit of IR. */
#define IR_DPUR			6		/**< Destination port unreachable bit of IR. */
#define IR_PPPT			5		/**< PPPoE terminate bit of IR. */
#define IR_FMTU			4		/**< Fragment MTU bit of IR. */
#define IR_SnINT(n)	   (n)		/**< SOCKETn interrupt occurrence bit of IR. */

/*****************************************/ 
/* The bit of Pn_BRDYR regsiter defintion*/ 
/*****************************************/ 
#define Pn_PEN		7		/**< PIN 'BRDYn' enable bit of Pn_BRDYR. */
#define Pn_MT		6		/**< PIN memory type bit of Pn_BRDYR. */
#define Pn_PPL		5		/**< PIN Polarity bit of Pn_BRDYR. */
#define Pn_SN(n) (n & 0x07)	/**< PIN Polarity bit of Pn_BRDYR. */


/***************************************/ 
/* The bit of Sn_MR regsiter defintion */ 
/***************************************/
/* Sn_MRH */
#define Sn_MR_ALIGN		0		/**< Alignment bit of Sn_MRH */
/* Sn_MRL */
#define Sn_MR_MULTI		7		/**< Multicasting bit of Sn_MRL */
#define Sn_MR_MF		6		/**< MAC filter bit of Sn_MRL */
#define Sn_MR_IGMPv		5		/**< IGMP version bit of Sn_MRL */
#define Sn_MR_ND		5		/**< No delayed ack bit of Sn_MRL */
#define Sn_MR_CLOSE		0x00		/**< Protocol bits of Sn_MRL */
#define Sn_MR_TCP		0x01		/**< Protocol bits of Sn_MRL */
#define Sn_MR_UDP		0x02		/**< Protocol bits of Sn_MRL */
#define Sn_MR_IPRAW		0x03		/**< Protocol bits of Sn_MRL */
#define Sn_MR_MACRAW	0x04		/**< Protocol bits of Sn_MRL */
#define Sn_MR_PPPoE		0x05		/**< Protocol bits of Sn_MRL */


/******************************/ 
/* The values of CR defintion */ 
/******************************/
#define Sn_CR_OPEN         0x01                 /**< OPEN command value of Sn_CR. */
#define Sn_CR_LISTEN       0x02                 /**< LISTEN command value of Sn_CR. */
#define Sn_CR_CONNECT      0x04                 /**< CONNECT command value of Sn_CR. */
#define Sn_CR_DISCON       0x08                 /**< DISCONNECT command value of Sn_CR. */
#define Sn_CR_CLOSE        0x10                 /**< CLOSE command value of Sn_CR. */
#define Sn_CR_SEND         0x20                 /**< SEND command value of Sn_CR. */
#define Sn_CR_SEND_MAC     0x21                 /**< SEND_MAC command value of Sn_CR. */ 
#define Sn_CR_SEND_KEEP    0x22                 /**< SEND_KEEP command value of Sn_CR */
#define Sn_CR_RECV         0x40                 /**< RECV command value of Sn_CR */
#define Sn_CR_PCON         0x23                 /**< PCON command value of Sn_CR */
#define Sn_CR_PDISCON      0x24                 /**< PDISCON command value of Sn_CR */ 
#define Sn_CR_PCR          0x25                 /**< PCR command value of Sn_CR */
#define Sn_CR_PCN          0x26                 /**< PCN command value of Sn_CR */
#define Sn_CR_PCJ          0x27                 /**< PCJ command value of Sn_CR */


/****************************************************************/ 
/* The values of Sn_IR defintion								*/ 
/****************************************************************/
#define Sn_IR_PRECV		7		/**< PPP receive bit of Sn_IR	*/
#define Sn_IR_PFAIL		6		/**< PPP fail bit of Sn_IR		*/
#define Sn_IR_PNEXT		5		/**< PPP next phase bit of Sn_IR*/
#define Sn_IR_SENDOK	4		/**< Send OK bit of Sn_IR		*/
#define Sn_IR_TIMEOUT	3		/**< Timout bit of Sn_IR		*/
#define Sn_IR_RECV		2		/**< Receive bit of Sn_IR		*/
#define Sn_IR_DISCON	1		/**< Disconnect bit of Sn_IR	*/
#define Sn_IR_CON		0		/**< Connect bit of Sn_IR		*/

/**********************************/ 
/* The values of Sn_SSR defintion */ 
/**********************************/
#define SOCK_CLOSED        0x00			/**< SOCKETn is released */
#define SOCK_ARP           0x01			/**< ARP-request is transmitted to acquire destina */
#define SOCK_INIT          0x13			/**< SOCKETn is open as TCP mode. */
#define SOCK_LISTEN        0x14			/**< SOCKETn operates as "TCP SERVER" and waits */
#define SOCK_SYNSENT       0x15			/**< Connect-request(SYN packet) is transmitted to "TCP SERVER" */
#define SOCK_SYNRECV       0x16			/**< Connect-request(SYN packet) is received from "TCP CLIENT". */
#define SOCK_ESTABLISHED   0x17			/**< TCP connection is established. */
#define SOCK_FIN_WAIT      0x18			/**< SOCKETn is closing. */
#define SOCK_CLOSING       0x1A			/**< SOCKETn is closing. */
#define SOCK_TIME_WAIT     0x1B			/**< SOCKETn is closing. */
#define SOCK_CLOSE_WAIT    0x1C			/**< Disconnect-request(FIN packet) is received from the peer. */
#define SOCK_LAST_ACK      0x1D			/**< SOCKETn is closing. */
#define SOCK_UDP           0x22			/**< SOCKETn is open as UDP mode. */
#define SOCK_IPRAW         0x32			/**< SOCKETn is open as IPRAW mode. */
#define SOCK_MACRAW        0x42			/**< SOCKET0 is open as MACRAW mode. */
#define SOCK_PPPoE         0x5F			/**< SOCKET0 is open as PPPoE mode. */

/* IP PROTOCOL */
#define IPPROTO_IP         0   /* Dummy for IP */
#define IPPROTO_ICMP       1   /* Control message protocol */
#define IPPROTO_IGMP       2   /* Internet group management protocol */
#define IPPROTO_GGP        3   /* Gateway^2 (deprecated) */
#define IPPROTO_TCP        6   /* TCP */
#define IPPROTO_PUP        12  /* PUP */
#define IPPROTO_UDP        17  /* UDP */
#define IPPROTO_IDP        22  /* XNS idp */
#define IPPROTO_ND         77  /* UNOFFICIAL net disk protocol */
#define IPPROTO_RAW        255 /* Raw IP packet */


typedef struct wiz5300_SOC_reg  {	/* SOCKET		BigEndian:				*/
	vuint8_t SR_MRH;		/* Mode Register High:	+0=>WordAddr/hiByteAddr	*/
	vuint8_t SR_MRL;		/* Mode Register Low:	+1=>lowByteAddr			*/
	vuint8_t SR_CRH;		/* Command Register			*/
	vuint8_t SR_CRL;
	vuint8_t SR_IMRH;		/* Interrupt Mask Register	*/
	vuint8_t SR_IMRL;
	vuint8_t SR_IRH;		/* Interrupt Register		*/
	vuint8_t SR_IRL;
	vuint8_t SR_SSRH;		/* Socket Status Register	*/
	vuint8_t SR_SSRL;
	vuint8_t SR_PORTRH;		/* Source Port Register		*/
	vuint8_t SR_PORTRL;
	vuint8_t SR_DHAR0;		/* Destination Hardware Address Register	*/
	vuint8_t SR_DHAR1;
	vuint8_t SR_DHAR2;
	vuint8_t SR_DHAR3;
	vuint8_t SR_DHAR4;
	vuint8_t SR_DHAR5;
	vuint8_t SR_DPORTRH;	/* Destination Port Register	*/
	vuint8_t SR_DPORTRL;
	vuint8_t SR_DIPR0;		/* Destination IP Address Register	*/
	vuint8_t SR_DIPR1;
	vuint8_t SR_DIPR2;
	vuint8_t SR_DIPR3;
	vuint8_t SR_MSSRH;		/* Maximum Segment Size Register	*/
	vuint8_t SR_MSSRL;
	vuint8_t SR_KPALVTR;	/* Keep Alive Time Register	*/
	vuint8_t SR_PROTOR;		/* Protocol Number Register	*/
	vuint8_t SR_TOSRH;		/* TOS Register (type-of-service)	*/
	vuint8_t SR_TOSRL;
	vuint8_t SR_TTLRH;		/* TTL (time-to-live) Register	*/
	vuint8_t SR_TTLRL;
	vuint8_t SR_TX_WRSR0;	/* Tx Write Size Register	*/
	vuint8_t SR_TX_WRSR1;
	vuint8_t SR_TX_WRSR2;
	vuint8_t SR_TX_WRSR3;
	vuint8_t SR_TX_FSR0;	/* Tx Free Size Register	*/
	vuint8_t SR_TX_FSR1;
	vuint8_t SR_TX_FSR2;
	vuint8_t SR_TX_FSR3;
	vuint8_t SR_RX_RSR0;	/* Rx Received Size Register	*/
	vuint8_t SR_RX_RSR1;
	vuint8_t SR_RX_RSR2;
	vuint8_t SR_RX_RSR3;
	vuint8_t SR_FRAGRH;		/* Fragment Register	*/
	vuint8_t SR_FRAGRL;
	vuint8_t SR_TX_FIFORH;	/* Tx FIFO Register	*/
	vuint8_t SR_TX_FIFORL;
	vuint8_t SR_RX_FIFORH;	/* Rx FIFO Register	*/
	vuint8_t SR_RX_FIFORL;
	vuint8_t SR_TX_RDH;
	vuint8_t SR_TX_RDL;
	vuint8_t SR_TX_WRH;
	vuint8_t SR_TX_WRL;
	vuint8_t SR_TX_ACKH;
	vuint8_t SR_TX_ACKL;
	vuint8_t SR_RX_RDH;
	vuint8_t SR_RX_RDL;
	vuint8_t SR_RX_WRH;
	vuint8_t SR_RX_WRL;
}SOC_t;


#define SOC0 (*(SOC_t *) (SOCKET_REG_BASE + 0x00))				/* SOC0.SR_MRL for example */
#define SOC1 (*(SOC_t *) (SOCKET_REG_BASE + 0x40))
#define SOC2 (*(SOC_t *) (SOCKET_REG_BASE + 0x80))
#define SOC3 (*(SOC_t *) (SOCKET_REG_BASE + 0xC0))
#define SOC4 (*(SOC_t *) (SOCKET_REG_BASE + 0x100))
#define SOC5 (*(SOC_t *) (SOCKET_REG_BASE + 0x140))
#define SOC6 (*(SOC_t *) (SOCKET_REG_BASE + 0x180))
#define SOC7 (*(SOC_t *) (SOCKET_REG_BASE + 0x1C0))
#define SOC(n) (*(SOC_t *) (SOCKET_REG_BASE + (n)*0x40))		/* SOC(0).SR_PORTL for example */
#define SOCPTR(n) ((SOC_t *) (SOCKET_REG_BASE + (n)*0x40))		/* e.g. SOCPTR(0)->SR_PORTH */


