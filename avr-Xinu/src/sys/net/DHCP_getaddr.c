/* DHCP_getaddr.c - DHCP_getaddr */

#include <conf.h>
#include <kernel.h>
#include <network.h>

extern SYSCALL wait(), signal();
extern int dhcp_client_start();
extern SYSCALL getaddr(IPaddr);
extern struct dhcp_client_state state;

/*
 *------------------------------------------------------------------------
 *  DHCP_getaddr  -  obtain this system's complete address (IP address)
 *		via DHCP and validate Net.
 *------------------------------------------------------------------------
 */
 
SYSCALL	DHCP_getaddr(IPaddr address)
{
	wait (state.dhcpsem);
	if (!Net.mavalid)
		dhcp_client_start();
	signal(state.dhcpsem);
	
	return ( getaddr(address) );
}
