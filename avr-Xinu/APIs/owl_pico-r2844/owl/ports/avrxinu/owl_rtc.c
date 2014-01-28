/* avr-Xinu:  owlrtc_init() */

#include <avr-Xinu.h>

#include <owl/core/owl_err.h>
#include <owl/drivers/owl_rtc.h>

int owlrtc_init(int hz, void (*tick_cb)(void))
{

	if (hz == 0)
		return OWL_ERR_PARAM;
	return 0;
}
