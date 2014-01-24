/* dgmopen.c - dgmopen */

#include <avr-Xinu.h>
#include <network.h>

int dgalloc(void);
int dgparse(struct dgblk *dgptr, char *fspec);

/*
 *------------------------------------------------------------------------
 *  dgmopen  -  open a fresh datagram pseudo device and return descriptor
 *------------------------------------------------------------------------
 */
 
int dgmopen(struct devsw *devptr, int forport, int locport)
{
	STATWORD ps;    
	struct	dgblk	*dgptr;
	struct	netq	*nqptr;
	int	slot;
	int	nq=0;
	int	i;

	disable(ps);
	if ( (slot=dgalloc()) == SYSERR) {
		restore(ps);
		kprintf("dgalloc: SYSERR\n");
		return(SYSERR);
	}
	dgptr = &dgtab[slot];
	if (locport == ANYLPORT)
		locport = udpnxtp();
	else {
		for (i=0 ; i<NETQS ; i++)
			if (Net.netqs[i].valid && 
				Net.netqs[i].uport == locport) {
				dgptr->dg_state = DG_FREE;
				restore(ps);
				kprintf("dgmopen: invalid netq %d\n",i);
				return(SYSERR);
			}
	}
	if (dgparse(dgptr,(char *)forport)==SYSERR || (nq=nqalloc())==SYSERR ) {
		dgptr->dg_state = DG_FREE;
		kprintf("dgmopen: bad dgparse or nq=%d\n",nq);
		restore(ps);
		return(SYSERR);
	}
	nqptr = &Net.netqs[nq];
	nqptr->uport = dgptr->dg_lport = locport;
	dgptr->dg_xport = nqptr->xport;
	dgptr->dg_netq = nq;
	dgptr->dg_mode = DG_NMODE;
	restore(ps);
	return(dgptr->dg_dnum);
}
