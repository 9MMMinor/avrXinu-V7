/* sndrarp.c - sndrarp */

#include <avr-Xinu.h>
#include <network.h>

/*
 *------------------------------------------------------------------------
 *  sndrarp  -  broadcast a RARP packet to obtain my IP address
 *------------------------------------------------------------------------
 */

int sndrarp(void)
{
	STATWORD ps;    
	struct	epacket	*mkarp();
	struct	epacket	*packet;
	int	i, mypid, resp;
	IPaddr	junk; /* needed for argument to mkarp; not ever used */

	mypid = getpid();
	for (i=0 ; i<AR_RTRY ; i++) {
		packet = mkarp(EP_RARP, AR_RREQ, junk, junk);
		if ( ((int)packet) == SYSERR)
			break;
		disable(ps);
		Arp.rarppid = mypid;
		recvclr();
		write(ETHER, (uint8_t *)packet, EMINPAK);
		resp = recvtim(AR_TIME);
		restore(ps);
		if (resp != TIMEOUT)
			return(OK);
	}
	panic("No response to RARP");
	return(SYSERR);
}
