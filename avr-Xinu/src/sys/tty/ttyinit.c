/* ttyinit.c - ttyinit */

#include <avr-Xinu.h>
#include <USART.h>
#include <tty.h>
#include <io.h>
#include <avr/io.h>

#define BAUD 9600L
#define UBRR (F_CPU)/(BAUD*8L)-1L		// U2X0 set for doubling

extern int screate();
extern void USART_Init();
extern FILE kprint_out;

/* because some of the Atmega USART registers are all over the place:				*/
USART_ptr USART[] = {
					{&UCSR0A,&UCSR0B,&UCSR0C,&UBRR0L,&UBRR0H,&UDR0},	/*	USART0	*/
					{&UCSR1A,&UCSR1B,&UCSR1C,&UBRR1L,&UBRR1H,&UDR1}		/*	USART1	*/
					};

/*
 *------------------------------------------------------------------------
 *  ttyinit - initialize buffers and modes for a tty line
 *------------------------------------------------------------------------
 */
 
DEVCALL ttyinit(struct devsw *devptr)
{
	register struct	tty *iptr;
	int	isconsole;
	int ubrr = UBRR;

	/* set up tty parameters */
	iptr = &tty[devptr->dvminor];
	devptr->dvioblk = (char *) iptr;		/* fill tty control blk	*/
	isconsole = (devptr->dvnum == CONSOLE);	/* make console cooked	*/
	iptr->unit = devptr->dvminor;			/* USART_ptr index		*/
	iptr->ihead = iptr->itail = 0;			/* empty input queue	*/
	iptr->isem = screate(0);				/* chars. read so far=0	*/
	iptr->icnt = 0;							/* "					*/
	iptr->osem = screate(OBUFLEN);			/* buffer available=all	*/
	iptr->ocnt = 0;							/* "					*/
	iptr->odsend = 0;						/* sends delayed so far	*/
	iptr->ohead = iptr->otail = 0;			/* output queue empty	*/
	iptr->ehead = iptr->etail = 0;			/* echo queue empty  	*/
	iptr->imode = (isconsole ? IMCOOKED : IMRAW);
	iptr->iecho = iptr->evis = isconsole;	/* echo console input	*/
	iptr->ierase = iptr->ieback = isconsole;/* console honors erase	*/
//	iptr->ierasec = BACKSP;					/*  delete character ^H	*/
	iptr->ierasec = 0x7f;					/*  delete character    */
	iptr->ecrlf = iptr->icrlf = isconsole;	/* map RETURN on input	*/
	iptr->ocrlf = iptr->oflow = isconsole;
	iptr->ieof  = iptr->ikill = isconsole;	/* set line kill == @	*/
	iptr->iintr = FALSE;
	iptr->iintrc = INTRCH;
	iptr->iintpid = BADPID;
	iptr->ikillc = KILLCH;
	iptr->ieofc = EOFC;
	iptr->oheld = FALSE;
	iptr->ostart = STRTCH;
	iptr->ostop = STOPCH;
	iptr->icursor = 0;
	iptr->ifullc = TFULLC;
	
	/* initialize libc stdin, stdout, and stderr */
//	if (isconsole)
//		{
//		stdin = stdout = stderr = &kprint_out; /* unbuffered, polled output */		
//		}

	/* set up the USART */
	USART_Init(devptr->dvminor, ubrr);			/* found in kprintf.c */
	
	*USART[devptr->dvminor].UCSRB |= (1<<RXCIE0);		/* set RX Complete Interrupt Enable */
	*USART[devptr->dvminor].UCSRB &= ~(1<<UDRIE0);		/* clear USART Data Register empty Interrupt Enable */
	
	return (OK);
}
