/* ttyoin.c - ttyoin */

#include <conf.h>
#include <kernel.h>
#include <USART.h>
#include <tty.h>
#include <io.h>

#include <avr/io.h>

extern SYSCALL signal(int sem);
extern SYSCALL signaln(int sem, int count);

/*
 *------------------------------------------------------------------------
 *  ttyoin  --  lower-half tty device driver for output interrupts
 *------------------------------------------------------------------------
 */
 
void ttyoin(struct tty *iptr)
{
	int n;
	int	ct;

	n = iptr->unit;		/*  USART[n]  */

	if (iptr->ehead	!= iptr->etail)	{
		*USART[n].UDR = iptr->ebuff[iptr->etail++];
		if (iptr->etail	>= EBUFLEN)
			iptr->etail = 0;
		return;
	}
	if (iptr->oheld) {			/* honor flow control	*/
		*USART[n].UCSRB &= ~(1<<UDRIE0);		/* disable Data Ready Interrupt */
		return;
	}
	if ((ct=iptr->ocnt) > 0) {
		*USART[n].UDR = iptr->obuff[iptr->otail++];
		--iptr->ocnt;
		if (iptr->otail	>= OBUFLEN)
			iptr->otail = 0;
		if (ct > OBMINSP)
			signal(iptr->osem);
		else if	( ++(iptr->odsend) == OBMINSP) {
			iptr->odsend = 0;
			signaln(iptr->osem, OBMINSP);
		}
	} else
		*USART[n].UCSRB &= ~(1<<UDRIE0);		/* disable Data Ready Interrupt */
}
