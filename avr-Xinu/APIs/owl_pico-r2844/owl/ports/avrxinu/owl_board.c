#include <avr-Xinu.h>
#include <owl/drivers/owl_board.h>
#include <avr/io.h>

int owlboard_init(void)
{

	PORTB = 0xff;		/* lights off */
	DDRB = 0xff;		/* outputs all */
	return 0;
}

