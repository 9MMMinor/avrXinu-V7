/* kprintf.c - kprintf, kputc, USART_Init, USART_Transmit */

#include <stdio.h>

#include <conf.h>
#include <kernel.h>
#include <USART.h>
#include <tty.h>
#include <avr/io.h>


static void USART_Transmit(char c);
static int kputc(char c, FILE * stream);

FILE kprint_out = FDEV_SETUP_STREAM(kputc,(void *)0,_FDEV_SETUP_WRITE); /*udata=CONSOLE=0*/

/*
 *------------------------------------------------------------------------
 *  kprintf  --  kernel printf: formatted, unbuffered output to kprint_out
 *------------------------------------------------------------------------
 */
 
int kprintf(const char *fmt, ...)
{
    STATWORD ps;
    va_list args;
    int ret;

    disable(ps);

    va_start(args, fmt);
    ret = vfprintf(&kprint_out, fmt, args);
    va_end(args);

    restore(ps);
	return ret;
}

/*
 *------------------------------------------------------------------------
 *  kprintf_P  --  printf_P: formatted, unbuffered output to kprint_out
 *------------------------------------------------------------------------
 */

int kprintf_P(const char *fmt, ...)
{
    STATWORD ps;
    va_list args;
	int ret;
	
    disable(ps);
	
    va_start(args, fmt);
    ret = vfprintf_P(&kprint_out, fmt, args);
    va_end(args);
	
    restore(ps);
	return ret;
}


/*
 *------------------------------------------------------------------------
 *  kputc  --  write a character on the console using polled I/O
 *------------------------------------------------------------------------
 */
 
static int kputc(char c, FILE * stream)
{

    if ( c == 0 )
		return EOF;
    if (c=='\n')
		USART_Transmit('\r');
    USART_Transmit(c);
    return 0;
}

static void USART_Transmit( char data )
{
    /* Wait for empty transmit buffer */
//	while ( !( UCSR0A & (1<<UDRE0)) )
	while ( !( *USART[CONSOLE].UCSRA & (1<<UDRE0)) )
		;
    /* Put data into buffer, sends the data */
    *USART[CONSOLE].UDR = data;
}


/*
 *-----------------------------------------------------------------------
 *	USART initialization sequence.
 *-----------------------------------------------------------------------
 */

void USART_Init( int dev_minor, unsigned int ubrr )
{
	
		/* Set baud rate */
		*USART[dev_minor].UBRR_H = (unsigned char)(ubrr>>8);
		*USART[dev_minor].UBRR_L = (unsigned char)ubrr;
	
		/* set 2X transmit and receive clock */
		*USART[dev_minor].UCSRA = (1<<U2X0);
		/* Enable receiver and transmitter */
		*USART[dev_minor].UCSRB = (1<<RXEN0)|(1<<TXEN0);
		/* Set frame format: 8data, 1 stop bit (default) */
		*USART[dev_minor].UCSRC =  (1<<USBS0) | (3<<UCSZ00);
}

