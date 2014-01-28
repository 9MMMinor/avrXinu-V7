#include <avr/io.h>

#include <owl/core/owl_debug.h>
#include <owl/drivers/owl_led.h>

/*
 * Toggle led on xplain board
 *
 */ 
void owlled_toggle(int led)
{ 
        owl_info("led:%d", led);
        
#if BOARD == XPLAIN
        if (led <= 7)
                PORTE_OUTTGL = (1 << led);
#endif
}
