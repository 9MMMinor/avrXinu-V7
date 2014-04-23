//
//  frameIO.h
//  802_15_4_Mib
//
//  Created by Michael Minor on 2/26/14.
//
//

#ifndef _02_15_4_Mib_frameIO_h
#define _02_15_4_Mib_frameIO_h

/* High-level radio definitions and constants */
#ifndef RADIOBUFS				/* override in avr-Configuration for small or large Targets */
#define	RADIOBUFS		10		/* number of radio buffers	*/
#endif
#define RADIO_QUEUE_LEN	5		/* input and output port lengths */

/* Network input and output processes: procedure name and parameters */

//#define	RADIOIN		frameInput	/* radio input daemon process	*/
//#define	RADIOOUT	frameOutput	/* radio output process	*/
//#define	RADIOISTK		600		/* stack size for radio input	*/
//#define	RADIOOSTK		600		/* stack size for radio output*/
//#define	RADIOIPRI		100		/* radio runs at high priority*/
//#define	RADIOOPRI		 99		/* radio output priority	*/
//#define	RADIOINAM	"radioIn"	/* name of radio input process*/
//#define	RADIOONAM	"radioOut"	/* name of radio output   "	*/
//#define	RADIOIARGC	1			/* count of args to radio input	*/
//#define	RADIOOARGC	2			/* count of args to radio output	*/


struct	radioinfo	{			/* info and parms. for radio	*/
	int	radiopool;		/* radio packet buffer pool	*/
	int fiport;			/* frame input port */
	int foport;			/* frame output port */
//	int faport;			/* frame ack port */
	int	nmutex;			/* output mutual excl. semaphore*/
	int	npacket;		/* # of packets processed	*/
	int	ndrop;			/* # of packets discarded	*/
	int	nover;			/* # dropped because queue full	*/
	int	nmiss;			/* # dropped	*/
	int	nerror;			/* # dropped	*/
};

/* PROYOTYPES */
PROCESS frameInput(int, int *);
PROCESS frameOutput(int, int *);

/* GLOBALS */
extern struct radioinfo Radio;


#endif
