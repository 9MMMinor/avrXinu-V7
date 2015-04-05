/* confisr.c (GENERATED FILE; Edits are overwritten by config) */


/* Interrupt Service routines referenced */

ISR(USART0_RX_vect)
{
	ttyiin(&tty[0]);
}
ISR(USART0_UDRE_vect)
{
	ttyoin(&tty[0]);
}

