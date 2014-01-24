/* getname.c - getname */

#include <avr-Xinu.h>
#include <network.h>

/*
 *------------------------------------------------------------------------
 *  getname  -  get name of this host and place it where specified
 *------------------------------------------------------------------------
 */
 
SYSCALL	getname(char *nam)
{
	IPaddr myaddr;
	char *p;

	*nam = NULLCH;
	if (!Net.mnvalid) {
		getaddr(myaddr);
		if (ip2name(myaddr, Net.myname) == SYSERR)
			return(SYSERR);
		Net.mnvalid = TRUE;
	}
	for (p=Net.myname ; *p != NULLCH && *p != '.' ; )
		*nam++ = *p++;
	*nam = NULLCH;
//	kprintf("getname: OK\n");
	return(OK);
}
