/* ttyiin.c ttyiin, ttybreak, erase1, eputc, echoch */

#include <conf.h>
#include <kernel.h>
#include <USART.h>
#include <tty.h>

#include <avr/io.h>

static void erase1();
static void echoch();
static void eputc();
extern SYSCALL send(int pid, WORD msg);
extern SYSCALL signal(int sem);
extern SYSCALL signaln(int sem, int count);

/*
 *------------------------------------------------------------------------
 *  ttyiin  --  lower-half tty device driver for input interrupts
 *------------------------------------------------------------------------
 */
 
void ttyiin(struct tty *iptr)
{
	int n;
	unsigned register int ch;
	int	err;
	int	cerr;
	int	ct;
	unsigned char status;
	
	n = iptr->unit;		/* tty unit */
	status = *USART[n].UCSRA;
	ch = *USART[n].UDR;

	err = status & ((1<<FE0) | (1<<DOR0) | (1<<UPE0));

	cerr = 0;
	if (err != 0) {
#ifdef PRINTERRORS	    
	    kprintf("recv error, ch: 0x%x ", (unsigned int) ch);
	    if (err & (1<<PE0))
			kprintf(", Parity Error");
	    if (err & (1<<DOR0))
			kprintf(", Data Overrun");
	    if (err & (1<<FE0))
			kprintf(", Framing Error");
	    kprintf("\n");
#endif	    
	    cerr = IOCHERR;
	}
	
//#ifdef DEBUG
//	kprintf("\nttyiin:<%c,%x,%x,%x>	",
//		ch,(unsigned int) ch, (unsigned int) err,
//		(unsigned int) cerr);
//#endif	
	
	if (iptr->imode == IMRAW) {
		if (iptr->icnt >= IBUFLEN){
			return;
		}
		iptr->ibuff[iptr->ihead++] = ch | cerr;
		++iptr->icnt;
		if (iptr->ihead	>= IBUFLEN)	/* wrap buffer pointer	*/
			iptr->ihead = 0;
	        signal(iptr->isem);
	} else {				/* cbreak | cooked mode	*/
		if ( ch	== RETURN && iptr->icrlf )
			ch = NEWLINE;
		if (iptr->iintr && ch == iptr->iintrc) {
			send(iptr->iintpid, INTRMSG);
			eputc(ch, iptr);
			return;
		}
		if (iptr->oflow) {
			if (ch == iptr->ostart)	{
				iptr->oheld = FALSE;
				*USART[n].UCSRB |= (1<<UDRIE0);	/*ttyostart(iptr);*/
				return;
			}
			if (ch == iptr->ostop) {
				iptr->oheld = TRUE;
				return;
			}
		} else
		    iptr->oheld = FALSE;
		if (iptr->imode	== IMCBREAK) {		/* cbreak mode	*/
			if (iptr->icnt >= IBUFLEN) {
				eputc(iptr->ifullc,iptr);
				return;
			}
			iptr->ibuff[iptr->ihead++] = ch	| cerr;
			if (iptr->ihead	>= IBUFLEN)
				iptr->ihead = 0;
			if (iptr->iecho)
				echoch(ch,iptr);
			if (iptr->icnt < IBUFLEN) {
				++iptr->icnt;
				signal(iptr->isem);
			}
		} else {				/* cooked mode	*/
			if (ch == iptr->ikillc && iptr->ikill) {
				iptr->ihead -= iptr->icursor;
				if (iptr->ihead	< 0)
					iptr->ihead += IBUFLEN;
				iptr->icursor =	0;
				eputc(RETURN,iptr);
				eputc(NEWLINE,iptr);
				return;
			}
			if (ch == iptr->ierasec	&& iptr->ierase) {
				if (iptr->icursor > 0) {
					iptr->icursor--;
					erase1(iptr);
				}
				return;
			}
			if (ch == NEWLINE || ch == RETURN ||
				(iptr->ieof && ch == iptr->ieofc)) {
				if (iptr->iecho) {
					echoch(ch,iptr);
					if (ch == iptr->ieofc)
						echoch(NEWLINE,iptr);
				}
				iptr->ibuff[iptr->ihead++] = ch | cerr;
				if (iptr->ihead	>= IBUFLEN)
					iptr->ihead = 0;
				ct = iptr->icursor+1; /* +1 for \n or \r*/
				iptr->icursor =	0;
				iptr->icnt += ct;
				signaln(iptr->isem,ct);
				return;
			}
			ct = iptr->icnt;
			ct = ct	< 0 ? 0	: ct;
			if ((ct	+ iptr->icursor) >= IBUFLEN-1) {
				eputc(iptr->ifullc,iptr);
				return;
			}
			if (iptr->iecho)
			        echoch(ch,iptr);
			iptr->icursor++;
			iptr->ibuff[iptr->ihead++] = ch	| cerr;
			if (iptr->ihead	>= IBUFLEN)
				iptr->ihead = 0;
		}
	}
}

/*------------------------------------------------------------------------
 *  erase1  --  erase one character honoring erasing backspace
 *------------------------------------------------------------------------
 */
static void erase1(struct tty *iptr)
{
	char	ch;

	if (--(iptr->ihead) < 0)
		iptr->ihead += IBUFLEN;
	ch = iptr->ibuff[iptr->ihead];
	if (iptr->iecho) {
		if (ch < BLANK || ch == 0177) {
			if (iptr->evis)	{
				eputc(BACKSP,iptr);
				if (iptr->ieback) {
					eputc(BLANK,iptr);
					eputc(BACKSP,iptr);
				}
			}
			eputc(BACKSP,iptr);
			if (iptr->ieback) {
				eputc(BLANK,iptr);
				eputc(BACKSP,iptr);
			}
		} else {
			eputc(BACKSP,iptr);
			if (iptr->ieback) {
				eputc(BLANK,iptr);
				eputc(BACKSP,iptr);
			}
		}
	} 
        else
			*USART[iptr->unit].UCSRB |= (1<<UDRIE0);			/*ttyostart(iptr);*/
}




/*------------------------------------------------------------------------
 *  echoch  --  echo a character with visual and ocrlf options
 *------------------------------------------------------------------------
 */
static void echoch(ch, iptr)
	char	ch;		/* character to	echo			*/
	struct	tty   *iptr;	/* pointer to I/O block for this devptr	*/
{
	if (ch==NEWLINE||ch==RETURN) {
		if (iptr->ecrlf) {
			eputc(RETURN,iptr);
			eputc(NEWLINE,iptr);
		}
	} else if ((ch<BLANK||ch==0177) && iptr->evis) {
		eputc(UPARROW,iptr);
		eputc(ch+0100,iptr);	/* make it printable	*/
	} else {
		eputc(ch,iptr);
	}
}

/*------------------------------------------------------------------------
 *  eputc - put one character in the echo queue
 *------------------------------------------------------------------------
 */
static void eputc(ch,iptr)
	char	ch;
	struct	tty   *iptr;
{
	iptr->ebuff[iptr->ehead++] = ch;
	if (iptr->ehead	>= EBUFLEN)
		iptr->ehead = 0;
	*USART[iptr->unit].UCSRB |= (1<<UDRIE0);			/*ttyostart(iptr);*/
}


/*-----------------------------------------------------------------------
 *  ttybreak -- handle a break received on the serial line
 *-----------------------------------------------------------------------
 */
//static void ttybreak()
//{
//    STATWORD ps;
    
//    disable(ps);
//    kprintf("\n\nSerial line BREAK detected.\n");
//	pause();
//    restore(ps);
//    return;
//}

