//
//  frameIO.c
//  802_15_4_Mib
//
//  Created by Michael M Minor on 12/11/13.
//
//

/*
 *	This module is a simple 802.15.4 network layer which would be replaced by specific protcol layers (ZigBee or others).
 *	We implement this layer with two processes, the input process, and the output process. The output process is
 *	fairly simple, it accepts frames from the local output port and sends them. The input process receives frames and
 *  decides whether to enqueue them for upper layers of the network software or to forward them. The user layer
 *	(main ...) does output by sending frames to Radio.foport and input by receiving frames from
 *	the input port (Radio.fiport).
 */

#include <avr-Xinu.h>
#include "frame802154.h"
#include "mib.h"
#include "frameIO.h"
#include "macSymbolCounter.h"
#include "radio.h"

extern Bool macPromiscuousMode;
struct radioinfo Radio;
extern struct rfDeviceControlBlock radio[];
void dropFrame(void);
PROCESS frameInput(int, int *);
PROCESS frameOutput(int, int *);
void doMACCommand(frame802154_t *);

/*
 *------------------------------------------------------------------------
 *  frameInit  -  initialize radio (IEEE 802.15.4) network data structures
 *------------------------------------------------------------------------
 */

int frameInit(void)
{
	/* Initialize pool of network buffers and rest of Radio structure	*/
	
	Radio.radiopool = mkpool(sizeof(frame802154_t), RADIOBUFS);
	Radio.nmutex = screate(1);
	Radio.fiport = pcreate(RADIO_QUEUE_LEN);		/* input */
	Radio.foport = pcreate(RADIO_QUEUE_LEN);		/* output */
//	Radio.faport = pcreate(RADIO_QUEUE_LEN);		/* separate ack port? */
	Radio.npacket = Radio.ndrop = Radio.nover = Radio.nmiss = Radio.nerror = 0;
	return(OK);
}

/*
 *-------------------------------------------------------------------------------------------------------------------
 *	frameInput -- read frames and send them to the proper port
 *-------------------------------------------------------------------------------------------------------------------
 */

PROCESS frameInput(int argc, int *argv)
{
	int len;
	frame802154_t *fptr;
	int userpid = 0;

	if (argc != 1)
		panic("frameInput: Bad argument\n");
	userpid = argv[0];
	
	/* initialze other parts of radio */
	frameInit();
	macSymbolCounterInit();
	radioTimerEventInit();
	radio_init();				/* initializes low-level 802.15.4 PHY + MAC */
	
	resume( create(RADIOOUT, RADIOOSTK, RADIOIPRI-1, RADIOONAM, 1, userpid) );
	
	for (fptr = (frame802154_t *)getbuf(Radio.radiopool); TRUE; )	{			/* FOREVER */
		/* MAC command reception shall abide by the procedure described in 5.1.6.2. */
		len = read(RADIO, (unsigned char *)fptr, sizeof(frame802154_t));	/* includes 1st level (FCS) filtering */
		if ( len < 0)	{
			kprintf("frameInput: TIME-OUT (%d)\n", len);
			dropFrame();
		}
		else if (macPromiscuousMode == TRUE)	{
//			kprintf("Promiscuous\n");
			psend(Radio.fiport, (int)fptr);					/* just queue the frame */
			fptr = (frame802154_t *)getbuf(Radio.radiopool);	/* and get a new one */
		}
		else	{			/* passes 3rd level filtering */
			switch (fptr->fcf.frameType)	{
				case FRAME_TYPE_BEACON:
					break;
				case FRAME_TYPE_DATA:
					psend(Radio.fiport, (int)fptr);					/* queue the frame for upper levels*/
//					kprintf("frameInput(data): f=%p\n", fptr);
					fptr = (frame802154_t *)getbuf(Radio.radiopool);	/* and get a new one */
					break;
				case FRAME_TYPE_ACK:
//					kprintf("frameInput(ack): f=%p\n", fptr);
					break;
				case FRAME_TYPE_MAC_COMMAND:
					doMACCommand(fptr);
					fptr = (frame802154_t *)getbuf(Radio.radiopool);
				default:
					Radio.nerror++;
			}
		}
	}
}
	
void dropFrame(void)
{
	Radio.ndrop++;
}
	
/*
 *-------------------------------------------------------------------------------------------------------------------
 *	frameOutput -- select a frame from local port or from forwarding port and send it
 *-------------------------------------------------------------------------------------------------------------------
 */
	
PROCESS frameOutput(int argc, int *argv)
{
	frame802154_t *fptr;
//	int loop = 0;
	
	if (argc != 1)
		panic("frameOutput: Bad arguments\n");
	resume( argv[0] );							/* main!! */
	kprintf("Network output process runs\n");	/* don't ask - this needs to be here??? */
	
	while ( TRUE )	{
		fptr = (frame802154_t *)preceive(Radio.foport);			/* blocks */
		int len = fptr->header_len + fptr->data_len + FTR_LEN;
		write(RADIO, (unsigned char *)fptr, len);
		freebuf((int *)fptr);
//		printf("frameOutput %d\n", loop++);
	}
}

void doMACCommand(frame802154_t *frame)
{
	freebuf((int *)frame);
	return;
}



