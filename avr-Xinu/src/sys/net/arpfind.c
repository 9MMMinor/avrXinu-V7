/* arpfind.c - arpfind */

#include <stdio.h>
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <network.h>

extern int blkcopy(uint8_t *to, uint8_t *from, int16_t nbytes);
extern int blkequ(uint8_t *first, uint8_t *second, uint16_t nbytes);

/*
 *------------------------------------------------------------------------
 *  arpfind  -  find or insert entry in ARP cache and return its index
 *------------------------------------------------------------------------
 */

int arpfind(IPaddr faddr)
{
	int	i;
	int	arindex;
	struct	arpent	*atabptr;

	for (arindex=0; arindex<Arp.atabsiz; arindex++) {
		atabptr = &Arp.arptab[arindex];
		if (blkequ((uint8_t *)atabptr->arp_Iad, (uint8_t *)faddr, IPLEN)
			&& atabptr->arp_state != AR_FREE)
			return(arindex);
	}
	if (Arp.atabsiz < AR_TAB) {
		Arp.atabsiz++;
	}
	arindex = Arp.atabnxt++;
	if (Arp.atabnxt >= AR_TAB)
		Arp.atabnxt = 0;
	atabptr = &Arp.arptab[arindex];
	atabptr->arp_state = AR_ALLOC;
	blkcopy((uint8_t *)atabptr->arp_Iad, (uint8_t *)faddr, IPLEN);
	for(i=0 ; i<EPADLEN ; i++)
		atabptr->arp_Ead[i] = '\0';
	atabptr->arp_dev = -1;
	return(arindex);
}
