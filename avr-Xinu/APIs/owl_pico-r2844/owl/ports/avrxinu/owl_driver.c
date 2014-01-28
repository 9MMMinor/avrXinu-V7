//
//  owl_driver.c
//  owl_pico_avrxinu
//
//  Created by Michael Minor on 2/20/12.
//  Copyright (c) 2012. All rights reserved.
//

#include <avr-Xinu.h>
#include <avr/interrupt.h>
#include <USART.h>
#include "owl.h"

#include <owl/drivers/owl_uart.h>
#include <owl/core/owl_err.h>
#include <wlp_api.h>
#include <wlp_proto.h>
#include <wlp_inet.h>

int	insert(int proc, int head, int key);
void stopclk(void);
int dequeue(int item);
int ready(int pid, int resched);
int resched(void);
void strtclk(void);
 
struct	owlcblk	owltab[Nowl];
extern void *owluart;

devcall owl_Init(
		struct devsw *devptr	/* entry in device switch table	*/
	)
{
	struct owlcblk *owptr;
	
	owluart = owptr = &owltab[ 0 ];
	owluart_init(owptr, UART_BAUDRATE_57600, UART_DATABITS_8,
				 UART_PARITY_NONE, UART_STOPBITS_1, 0); /* normally 57600:8N1 */
	return (OK);
}

int
owluart_init(void *uart,
             uint32_t baudrate, enum uart_databits databits,
             enum uart_parity parity, enum uart_stopbits stopbits,
             int rtscts)
{	
	uint8_t ctrlc = 0;
    uint8_t ctrlb = (1<<RXEN1) | (1<<TXEN1);
    uint16_t ubrr;
	
	if (rtscts)
		return OWL_ERR_NOTSUPP;
	
	switch(databits) {
        case UART_DATABITS_5:
			ctrlc |= (0<<UCSZ10)|(0<<UCSZ11);
			break;
        case UART_DATABITS_6:
			ctrlc |= (1<<UCSZ10)|(0<<UCSZ11);
			break;
        case UART_DATABITS_7:
			ctrlc |= (0<<UCSZ10)|(1<<UCSZ11);
			break;
        case UART_DATABITS_8:
			ctrlc |= (1<<UCSZ10)|(1<<UCSZ11);
			break;
        case UART_DATABITS_9:
			ctrlc |= (1<<UCSZ10)|(1<<UCSZ11);
			ctrlb |= (1<<UCSZ12);
			break;
	};
	
	switch (parity) {
        case UART_PARITY_NONE:
			break;
        case UART_PARITY_EVEN:
			ctrlc |= (1<<UPM11) | (0<<UPM10);
			break;
        case UART_PARITY_ODD:
			ctrlc |= (1<<UPM11) | (1<<UPM10);
			break;
        default:
			return OWL_ERR_NOTSUPP;
	}
	
	switch (stopbits) {
        case UART_STOPBITS_1:
			break;
        case UART_STOPBITS_2:
			ctrlc |= (1<<USBS1);
			break;
	}
	
//	if (F_CPU < (16UL * baudrate))	{
		UCSR1A |= (1<<U2X1);	/* double USART clock */
		ubrr = F_CPU / (8UL * baudrate) - 1;

//	}
//	else	{
//		ubrr = F_CPU / (16UL * baudrate) -1;
//	}
	
	UCSR1B = 0;				/* disable transceiver while configuring */
	UCSR1C = ctrlc;
	UBRR1L = ubrr & 0xff;
	UBRR1H = ubrr >> 8;
	UCSR1B = ctrlb;
	USART_Flush();
	UCSR1B |= (1<<RXCIE1);	/* enable Receive Complete Interrupt */
	return 0;
}

void USART_Flush( void )
{
	unsigned char dummy;
	
	while ( UCSR1A & (1<<RXC1) ) dummy = UDR1;
}

/*
 ***************************NOT USED IN XINU*****************************************
 * Read len bytes from the UART. This function should block until len bytes
 * have been written.
 * This function may be passed to wlp_init().
 *
 * NOTE: This function must be modified for the specific platform used.
 ***POLLED READ ATMega1284p******POLLED READ ATMega1284p******POLLED READ ATMega1284p
 *
 ***************************NOT USED IN XINU*****************************************
 */
int board_uart_read_f(void *ctx, void *buf, int len)
{
	int i;
	char *in = buf;
	
	for (i = 0; i < len; i++) { // loop until all bytes are read
		// wait for a char
		while ( !(UCSR1A & (1<<RXC1)) )	{
			;
		}
		// read the char into the provided buffer
		in[i] = UDR1;
	}	
	return len;
}

/*
 ***************************NOT USED IN XINU*****************************************
 * Write len bytes to the UART. This function blocks until len bytes are written.
 * This function may be passed to wlp_init().
 *
 * NOTE: This function must be modified for the specific platform used.
 ***POLLED WRITE ATMega1284p******POLLED WRITE ATMega1284p******POLLED WRITE ATMega12
 *
 ***************************NOT USED IN XINU*****************************************
 */
int board_uart_write_f(void *ctx, const void *buf, int len)
{
	int i;
	const char *out = buf;
    
	for (i = 0; i < len; i++) { // loop until all bytes are written
		
		// wait until we are ready to transmit next char
		while ( !(UCSR1A & (1<<UDRE1)) )
			; 
		// transmit the char
		UDR1 = out[i];
	} 
	return len;
}

/*
 *	owl_BlockRead(void *ctx, void *buf, int len)
 *	owl_BlockWrite(void *ctx, const void *buf, int len)
 *
 *	Avr-Xinu interrupt driven i/o mode driver. Block read
 *	and block write can only be called following a call to
 *	sendRequest() with a wlp_api WLP_SOCK_SEND, WLP_SOCK_SENDTO,
 *	or WLP_SOCK_RECV request id (see wlp_proto.h).
 *
 *	Read/Write 'len' bytes to SPB800 to/from 'buf'.
 *
 */

int owl_BlockRead(void *ctx, void *buf, int len)
{
	return 0;// ( owl_RW(ctx, (uint8_t *)buf, len) );
}

int owl_BlockWrite(void *ctx, const void *buf, int len)
{
	return 0;// ( owl_RW(ctx, (uint8_t *)buf, len) );
}
/*
int owl_RW(struct owlcblk *owptr, uint8_t *buf, int len)
{
	int retv;
	
	owptr->rwbuf = buf;
	owptr->rwlen = len;
	switch (owptr->userLevel_state)	{
		case BLOCK_WRITE:
			UCSR1B |= (1<<UDRIE1);		// turn ON output interrupt
			// Fall through
		case BLOCK_READ:
			retv = receive();			// could be receive or time-out:
			//	rvalue = recvtim(10);	// wait 10 TICKS
			//	if (rvalue == TIMEOUT)
			//		{
			//		something else;
			//		signal(some_semaphore);
			//		return (SOMEERROR);
			//		}
			break;
		default:
			retv = WLP_ERR_PROTO;
			break;
	}
	restore(ps);
	signal(owptr->mutex_Semaphore);
	return (retv);
}
*/

int sendRequest(struct wlp_req *req, int reqlen,
			 struct wlp_cfm *cfm, int cfmlen,
			 uint8_t *buf, int buflen)
{
	STATWORD ps;
	struct owlcblk *owptr;
	int rvalue;

	owptr = &owltab[ 0 ];
	wait(owptr->mutex_Semaphore);
	disable(ps);
	owptr->pid = getpid();
	owptr->req = req;
	owptr->reqlen = reqlen;
	owptr->cfm = cfm;
	owptr->cfmlen = cfmlen;
	switch (owptr->req->id)	{
		case WLP_SOCK_SEND:
		case WLP_SOCK_SENDTO:
		case WLP_SOCK_RECV:
			owptr->rwbuf = buf;
			owptr->rwlen = buflen;
			break;
		default:
			owptr->rwbuf = NULL;
			owptr->rwlen = 0;
			break;
	}
	UCSR1B |= (1<<UDRIE1);		// turn ON output interrupt
	rvalue = receive();			// could be receive or time-out:
	//	rvalue = recvtim(10);	// wait 10 TICKS
	//	if (rvalue == TIMEOUT)
	//		{
	//		something else;
	//		signal(some_semaphore);
	//		return (SOMEERROR);
	//		}
	restore(ps);
	signal(owptr->mutex_Semaphore);
	return (rvalue);
}

int owluart_Startup(struct wlp_req *wlpRequest)
{
	STATWORD ps;
	struct owlcblk *owptr = &owltab[0];
	int rvalue;
	
	if ( wlpRequest->id == WLP_RESET )	{
        struct wlp_reset_cfm cfm;
        int err;
        
		owptr->interruptLevel_state = READY;
        if ((err = sendRequest(wlpRequest, sizeof(wlpRequest),
                            (struct wlp_cfm *) &cfm, sizeof(cfm),
							NULL, 0)) < 0)
			return err;
	}
	else	{
		owptr->mutex_Semaphore = screate(1);
	}
	owptr->pid = getpid();
	disable(ps);
	owptr->interruptLevel_state = INITIALIZE;
	UCSR1B |= (1<<UDRIE1);		// turn ON output interrupt
	rvalue = receive();			// could be receive or time-out:
	//	rvalue = recvtim(10);	// wait 10 TICKS
	//	if (rvalue == TIMEOUT)
	//		{
	//		restore(ps);
	//		something else;
	//		return (SOMEERROR);
	//		}
	restore(ps);
	return (rvalue);
}

/*
 *------------------------------------------------------------------------
 *  owl_RX - USART RX complete interrupt (interrupts disabled)
 *------------------------------------------------------------------------
 */

ISR(USART1_RX_vect)		// USART1 RX complete
{
    owl_ISR( &owltab[0] );
}

/*
 *------------------------------------------------------------------------
 *  owl_UDRE - USART Data Register Empty interrupt
 *------------------------------------------------------------------------
 */

ISR(USART1_UDRE_vect)
{
	owl_ISR( &owltab[0] );
}

static void done(int returnMessage, struct owlcblk *owptr)
{
	owptr->interruptLevel_state = READY;
	UCSR1B &= ~(1<<UDRIE1);	/* turn off output interrupt */
	if ( owptr->pid )	{
		send(owptr->pid, returnMessage);
	}
}

static volatile uint8_t *bufNext;
static volatile uint8_t bufCount;
static uint8_t totlen = 0;

void owl_ISR(struct owlcblk *owptr)
{
	uint8_t acklen;
	char ch;
	
	switch ( owptr->interruptLevel_state )
	{
		case READY:					/* UDRE interrupt */
			totlen = sizeof(totlen) + owptr->reqlen;
			UDR1 = totlen;
			owptr->interruptLevel_state = WACK;
			UCSR1B &= ~(1<<UDRIE1);	/* turn off output interrupt */
			break;
		case WACK:					/* RX interrupt */
			acklen = UDR1;
			if (acklen != totlen)	{
				done(WLP_ERR_PROTO, owptr);
				return;
			}
			bufNext = (uint8_t *)owptr->req;
			bufCount = owptr->reqlen;
			owptr->interruptLevel_state = TRANSMITTING_REQ;
			UCSR1B |= (1<<UDRIE1);	/* turn ON output interrupt */
			break;
		case TRANSMITTING_REQ:		/* UDRE */
			if (bufCount-- > 0)	{
				UDR1 = *bufNext++;
			}
 			else	{
				owptr->interruptLevel_state = RECEIVING_CFM;
				UCSR1B &= ~(1<<UDRIE1);	/* turn off output interrupt */
				bufNext = (uint8_t *)owptr->cfm;
				bufCount = owptr->cfmlen;
			}
			break;
		case RECEIVING_CFM:			/* RX */
			*bufNext++ = UDR1;
			if ( --bufCount <= 0 )	{
				owptr->cfm->res = ntohs(owptr->cfm->res);
				if (owptr->cfm->res < 0)	{
					done(owptr->cfm->res, owptr);
					return;
				}
				if ( owptr->userLevel_state == CONTROL_STATUS_REQ ){
					done(0, owptr);
					return;
				}
				bufNext = owptr->rwbuf;
				bufCount = owptr->cfm->res;
				switch (owptr->req->id)	{
					case WLP_SOCK_RECV:
						owptr->interruptLevel_state = RECEIVING_DATA;
						break;
					case WLP_SOCK_SEND:
					case WLP_SOCK_SENDTO:
						owptr->interruptLevel_state = TRANSMITTING_DATA;
						UCSR1B |= (1<<UDRIE1);	/* turn ON output interrupt */
						break;
					default:
						done(owptr->cfm->res, owptr);
						return;
				}
			}
			break;
		case RECEIVING_DATA:		/* RX */
			*bufNext++ = UDR1;
			if ( --bufCount <= 0 )	{
				done(owptr->cfm->res, owptr);
				return;
			}
			break;
		case TRANSMITTING_DATA:		/* UDRE */
			if (bufCount-- > 0)	{
				UDR1 = *bufNext++;
			}
			else	{
				done(owptr->cfm->res, owptr);
				return;
			}
			break;
		/* implementation of wlp_init() */
		case INITIALIZE:			/* UDRE */
			UDR1 = 'w';
			UCSR1B &= ~(1<<UDRIE1);	/* turn off output interrupt */
			owptr->interruptLevel_state = INITIALIZE_ACK;
			break;
		case INITIALIZE_ACK:		/* RX */
			ch = UDR1;
			if (ch == '.')
				return;
			owptr->interruptLevel_state = READY;
			done(0, owptr);
			break;
		default:
			done(SYSERR, owptr);
	}
}
