/*
 *  ttyint.c - replace with GENERATED file "confisr.c"
 *  AVR-Xinu
 *
 *  Created by Michael Minor on Tue Apr 21 2009.
 *  Copyright (c) 2009. All rights reserved.
 *
 */

#include <conf.h>
#include <kernel.h>
#include <USART.h>
#include <tty.h>
#include <avr/io.h>
#include <avr/interrupt.h>


/*--------------------------------------------------------------------------
 * USART0, Tx Buffer Empty interrupt -- call ttyoin(struct *tty)
 *--------------------------------------------------------------------------
 */

ISR(USART0_UDRE_vect)		/* USART0, Data Register Empty Interrupt */
{
    ttyoin(&tty[0]);
}


/*--------------------------------------------------------------------------
 * USART0, Rx Complete interrupt -- call ttyiin(struct *tty)
 *--------------------------------------------------------------------------
 */

ISR(USART0_RX_vect)		/* USART0, Rx Complete */
{
    ttyiin(&tty[0]);
}

/*--------------------------------------------------------------------------
 * USART1, Tx Buffer Empty interrupt -- call ttyoin(struct *tty)
 *--------------------------------------------------------------------------
 */

ISR(USART1_UDRE_vect)		/* USART0, Data Register Empty Interrupt */
{
    ttyoin(&tty[1]);
}


/*--------------------------------------------------------------------------
 * USART1, Rx Complete interrupt -- call ttyiin(struct *tty)
 *--------------------------------------------------------------------------
 */

ISR(USART1_RX_vect)		/* USART0, Rx Complete */
{
    ttyiin(&tty[1]);
}
