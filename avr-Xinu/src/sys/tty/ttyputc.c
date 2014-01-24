/* ttyputc.c - ttyputc */

#include <conf.h>
#include <kernel.h>
#include <USART.h>
#include <tty.h>
#include <io.h>

extern SYSCALL wait();

/*------------------------------------------------------------------------
 *  ttyputc - write one character to a tty device
 *------------------------------------------------------------------------
 */
int ttyputc(struct devsw *devptr, unsigned char ch)
{
	STATWORD ps;    
	struct tty *iptr;

	iptr = &tty[devptr->dvminor];
	if ( ch==NEWLINE && iptr->ocrlf )
		ttyputc(devptr,RETURN);
	disable(ps);
	wait(iptr->osem);		/* wait	for space in queue	*/
	iptr->obuff[iptr->ohead++] = ch;
	++iptr->ocnt;
	if (iptr->ohead	>= OBUFLEN)
		iptr->ohead = 0;
	*USART[iptr->unit].UCSRB |= (1<<UDRIE0);			/*ttyostart(iptr);*/
	restore(ps);
	return(OK);
}



