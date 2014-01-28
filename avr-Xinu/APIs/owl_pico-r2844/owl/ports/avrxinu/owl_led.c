#include <owl/core/owl_debug.h>
#include <owl/drivers/owl_led.h>
#include <avr/io.h>

void owlled_toggle(int led_id)
{
	PORTB ^= (1<<led_id);
	owl_info("led_id:%d", led_id);
}
