/*
 *  ttyopen.c
 *  avr-Xinu
 *
 *  Created by Michael Minor on 3/12/10.
 *  Copyright 2010. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <USART.h>
#include <tty.h>

extern SYSCALL wait(int);


int stdPut(char, FILE *);
int stdGet(FILE *);

void *
ttyopen(struct devsw *devptr)
{
	struct tty *iptr = &tty[devptr->dvminor];
	FILE *streamPtr;
	
	if ( (streamPtr = (FILE *)calloc(1,sizeof(FILE)) ) == (FILE *)NULL )
		return 0;
	fdev_setup_stream(streamPtr, stdPut, stdGet, _FDEV_SETUP_RW);
	fdev_set_udata(streamPtr, (void *)iptr);	/* Set user defined and accessible pointer */
	
	stdout = streamPtr;
	
//	if (strcmp(name, "stdout") == 0)
//		stdout = streamPtr;				/* stdout is a macro */
//	else if (strcmp(name, "stdin") == 0)
//		stdin = streamPtr;
//	else if (strcmp(name, "stderr") == 0)
//		stderr = streamPtr;
	
	return (void *)streamPtr;
}

/* prototype for a pseudo driver BUT ONLY FOR tty */
int
stdPut(char ch, FILE * stream)
{
	STATWORD ps;    
	struct tty *iptr;
	
	iptr = (struct tty *)fdev_get_udata(stream);
	if ( ch==NEWLINE && iptr->ocrlf )
		stdPut(RETURN, stream);
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

int
stdGet(FILE * stream)
{
	STATWORD ps;    
	int	ch;
	struct	tty   *iptr;
	
	disable(ps);
	iptr = (struct tty *)fdev_get_udata(stream);
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

