/*
 *  USART.h
 *  AVR_kprintf
 *
 *  Created by Michael Minor on 2/22/10.
 *  Copyright 2010. All rights reserved.
 *
 */
 
#include <avr/io.h>
 
/**********************************************************************************/
/* NOTE: while this works well for the ATmega1284P, it is a total disaster for    */
/* the much more irregular atmega128                                              */
/* Use the usart_register structure, USART[], and initialize it.				  */
/**********************************************************************************/
 

/*
 *  The ATmega1284P has two USART’s, USART0 and USART1
 *
 *  (0xCE) UDR1		USART1 I/O Data Register									189
 *  (0xCD) UBRR1H - - - - USART1 Baud Rate Register High Byte					193/206
 *  (0xCC) UBRR1L USART1 Baud Rate Register Low Byte							193/206
 *  (0xCB) Reserved - - - - - - - -
 *  (0xCA) UCSR1C UMSEL11 UMSEL10 UPM11 UPM10 USBS1 UCSZ11 UCSZ10 UCPOL1		191/205
 *  (0xC9) UCSR1B RXCIE1 TXCIE1 UDRIE1 RXEN1 TXEN1 UCSZ12 RXB81 TXB81			190/204
 *  (0xC8) UCSR1A RXC1 TXC1 UDRE1 FE1 DOR1 UPE1 U2X1 MPCM1						189/204
 *
 *  (0xC6) UDR0		USART0 I/O Data Register									189
 *  (0xC5) UBRR0H - - - - USART0 Baud Rate Register High Byte					193/206
 *  (0xC4) UBRR0L USART0 Baud Rate Register Low Byte							193/206
 *  (0xC3) Reserved - - - - - - - -
 *  (0xC2) UCSR0C UMSEL01 UMSEL00 UPM01 UPM00 USBS0 UCSZ01 UCSZ00 UCPOL0		191/205
 *  (0xC1) UCSR0B RXCIE0 TXCIE0 UDRIE0 RXEN0 TXEN0 UCSZ02 RXB80 TXB80			190/204
 *  (0xC0) UCSR0A RXC0 TXC0 UDRE0 FE0 DOR0 UPE0 U2X0 MPCM0						189/204
 */

typedef struct usart_csr
{
	volatile uint8_t UCSRA;			/*  Control and Status Register A		*/
	volatile uint8_t UCSRB;			/*  Control and Status Register B		*/
	volatile uint8_t UCSRC;			/*  Control and Status Register C		*/
	volatile uint8_t RES;			/*  Reserved							*/
	volatile uint8_t UBRR_L;		/*  Baud Rate Register Low Byte			*/
	volatile uint8_t UBRR_H;		/*  Baud Rate Register High Byte		*/
	volatile uint8_t UDR;			/*  I/O Data Register					*/
}USART_t;

#define USART0		(*(USART_t *) 0xC0)
#define USART0_ptr	((USART_t *) 0xC0)
#define USART1		(*(USART_t *) 0xC8)
#define USART1_ptr	((USART_t *) 0xC8)


/*
 *	The ATmega128 has two USART
 *
 *	(0x9D)			UCSR1C	– UMSEL1 UPM11 UPM10 USBS1 UCSZ11 UCSZ10 UCPOL1			191
 *	(0x9C)			UDR1	USART1 I/O Data Register								189
 *	(0x9B)			UCSR1A	RXC1 TXC1 UDRE1 FE1 DOR1 UPE1 U2X1 MPCM1 189
 *	(0x9A)			UCSR1B	RXCIE1 TXCIE1 UDRIE1 RXEN1 TXEN1 UCSZ12 RXB81 TXB81		190
 *	(0x99)			UBRR1L	USART1 Baud Rate Register Low							192
 *	(0x98)			UBRR1H	– – – – USART1 Baud Rate Register High					192
 
 *	(0x95)			UCSR0C	– UMSEL0 UPM01 UPM00 USBS0 UCSZ01 UCSZ00 UCPOL0			191
 *	(0x90)			UBRR0H	– – – – USART0 Baud Rate Register High					192
 *	0x0C (0x2C)		UDR0	USART0 I/O Data Register								189
 *	0x0B (0x2B)		UCSR0A	RXC0 TXC0 UDRE0 FE0 DOR0 UPE0 U2X0 MPCM0				189
 *	0x0A (0x2A)		UCSR0B	RXCIE0 TXCIE0 UDRIE0 RXEN0 TXEN0 UCSZ02 RXB80 TXB80		190
 *	0x09 (0x29)		UBRR0L	USART0 Baud Rate Register Low							192
 */
 
typedef volatile uint8_t v_reg;
typedef struct usart_registers {
	v_reg * UCSRA;
	v_reg * UCSRB;
	v_reg * UCSRC;
	v_reg * UBRR_L;
	v_reg * UBRR_H;
	v_reg * UDR;
}USART_ptr;

extern USART_ptr USART[];
