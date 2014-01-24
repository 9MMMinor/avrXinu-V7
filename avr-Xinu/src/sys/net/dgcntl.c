/* dgcntl.c - dgcntl */

#include <avr-Xinu.h>
#include <network.h>

/*
 *------------------------------------------------------------------------
 *  dgcntl  -  control function for datagram pseudo-devices
 *------------------------------------------------------------------------
 */
 
int dgcntl(struct devsw *devptr, int func, void * arg, void * arg2)
{
	STATWORD ps;    
	struct	dgblk	*dgptr;
	int	freebuf();
	int	ret;

	disable(ps);
	dgptr = (struct dgblk *)devptr->dvioblk;
	ret = OK;
	switch (func) {

		case DG_SETMODE:	/* set mode bits */
			dgptr->dg_mode = (int)arg;
			break;

		case DG_CLEAR:		/* clear queued packets */
			preset(dgptr->dg_xport, freebuf);
			break;

		default:
			ret = SYSERR;
	}
	restore(ps);
	return(ret);
}
