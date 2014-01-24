/* rfdump.c - rfdump */

#include <avr-Xinu.h>
#include <fserver.h>
#include <rfile.h>
#include <avr/pgmspace.h>

/*------------------------------------------------------------------------
 *  rfdump  --  dump the contents of the remote file device blocks
 *------------------------------------------------------------------------
 */

void
rfdump(void)
{
	struct	rfblk	*rfptr;
	int	i;

	kprintf_P(PSTR("Remote files: server on dev=%d, server mutex=%d\n"),
			Rf.device, Rf.rmutex);
	for (i=0 ; i<Nrf; i++) {
		rfptr = &Rf.rftab[i];
		if (rfptr->rf_state == RFREE)
			continue;
		kprintf_P(PSTR(" %2d. "), rfptr->rf_dnum);
		kprintf_P(PSTR("mode=%03o, sem=%2d, pos=%6D, file name=%s\n"),
				rfptr->rf_mode,	rfptr->rf_mutex,
				rfptr->rf_pos, rfptr->rf_name);
	}
}
