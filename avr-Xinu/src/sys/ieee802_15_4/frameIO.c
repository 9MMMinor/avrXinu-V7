//
//  frameIO.c
//  802_15_4_Mib
//
//  Created by Michael M Minor on 12/11/13.
//
//

/*
 *	This module is a simple management layer which would be replaced by specific protcol layers (ZigBee or others).
 *	We implement this layer with two processes, the input process, and the output process. The output process is
 *	fairly simple, it accepts frames from the local output port or from a forwarding port, and sends them. The input
 *	process receives frames and decides whether to enqueue them for upper layers of the network software or to forward
 *	them.
 */

#include <stdio.h>
#include "frame802154.h"
#include "mib.h"

static void dropFrame(struct fglob *, frame_t *);

/*
 *-------------------------------------------------------------------------------------------------------------------
 *	frameInput -- read frames and send them to the proper port
 *-------------------------------------------------------------------------------------------------------------------
 */

PROCESS frameInput(int netID)
{
	int len;
	octet_t *data;
	frame802154_t *p
	frame_t *fptr;
	struct fglob *fgptr;		/* global data for this layer: ports, sequence no's, failure counts */

	p = (frame802154_t *)getmem(sizeof frame802154_t);
	fptr = (frame_t *)getbuf(frameBufferPool);
	fgptr = &fdata[netID];
	for (fgptr->fiseq = fgptr->fifails = 0; TRUE; )	{			/* FOREVER */
		/* MAC command reception shall abide by the procedure described in 5.1.6.2. */
		len = read(fgptr->findev, fptr, FRAMEMAXLENGTH);	/* includes 1st level (FCS) filtering */
		if ( len < 0)	{	
			dropFrame(fptr);
		}
		else if (macPromiscuousMode == TRUE)	{
			psend(fgptr->fiport, fptr);					/* just queue the frame */
			fptr = (frame_t *)getbuf(frameBufferPool);	/* and get a new one */
		}
		else if (parseFrame(p, fptr) == OK)	{			/* passes 3rd level filtering */
			switch (p->fcf.frameType)	{
				case FRAME_TYPE_DATA:
					psend(fgptr->fiport, fptr);					/* queue the frame */
					fptr = (frame_t *)getbuf(frameBufferPool);	/* and get a new one */
					break;
				case FRAME_TYPE_ACK:
					psend(fgptr->foport, fptr);
					fptr = (frame_t *)getbuf(frameBufferPool);
					break;
				case FRAME_TYPE_MAC_COMMAND:
					payload = makeMACCommandHdr(p, (octet_t *)frame);
			}
		else	{
			dropFrame(fptr);	
		}
	}
}
	
static void dropFrame(struct fglob *fgptr, frame_t *fptr)
	{
		++fgptr->fifails;
	}
	
/*
 *-------------------------------------------------------------------------------------------------------------------
 *	frameOutput -- select a frame from local port or from forwarding port and send it
 *-------------------------------------------------------------------------------------------------------------------
 */
	
PROCESS frameOutput(int netID)
{
	frame_t *fptr;
	struct fglob *fgptr;
	
	fgptr = &fdata[netID];
	for (fgptr->foseq = fgptr->fofails = 0; TRUE; )	{			/* FOREVER */
		wait(fgptr->fosem);										/*  block until there is output */
		if (pcount(fgptr->ffport) > 0)
			fptr = (frame_t)preceive(fgptr->ffport);
		else
			fptr = (frame_t)preceive(fgptr->foport);
		frameSend(fptr, fgptr);
		freebuf(fptr);
	}
}





