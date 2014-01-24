/* getaddr.c - getaddr */

#include <conf.h>
#include <kernel.h>
#include <network.h>
#include <string.h>

extern int blkcopy();
extern SYSCALL wait(), signal();
extern int dhcp_client_start();
extern SYSCALL getaddr(IPaddr);
extern struct dhcp_client_state state;

/*
 *----------------------------------------------------------------
 *  getaddr  -  obtain this system's complete address (IP address)
 *----------------------------------------------------------------
 */

SYSCALL	getaddr(IPaddr address)
{
#ifdef RARP
	wait (Arp.rarpsem);
	if (!Net.mavalid)
		sndrarp();
	signal(Arp.rarpsem);
#endif
	if (!Net.mavalid)	{
		memset(address, 0, IPLEN);
		return(SYSERR);
	}
	blkcopy(address, Net.myaddr, IPLEN);
	return(OK);
}
