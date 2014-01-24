/* ttygetc.c - ttygetc */

#include <conf.h>
#include <kernel.h>
#include <USART.h>
#include <tty.h>
#include <io.h>

extern SYSCALL wait();

/*------------------------------------------------------------------------
 *  ttygetc - read one character from a tty device
 *------------------------------------------------------------------------
 */
int ttygetc(struct devsw *devptr)
{
	STATWORD ps;    
	int	ch;
	struct	tty   *iptr;

	disable(ps);
	iptr = &tty[devptr->dvminor];
	wait(iptr->isem);		/* wait	for a character	in buff	*/
	ch = 0xff & iptr->ibuff[iptr->itail++];
	--iptr->icnt;
	if (iptr->itail	>= IBUFLEN)
		iptr->itail = 0;
	if (iptr->ieof && (iptr->ieofc == ch) )
		ch = EOF;
	restore(ps);
	return(ch);
}
