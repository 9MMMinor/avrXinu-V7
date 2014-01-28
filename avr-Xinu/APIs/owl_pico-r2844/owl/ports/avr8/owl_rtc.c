#include <board.h>
#include <sysclk.h>
#include <sleepmgr.h>
#include <rtc.h>

#include <owl/drivers/owl_rtc.h>

void (*cb)(void) = NULL;

static void alarm(uint32_t time)
{
        if (cb)
                cb();

        rtc_set_alarm_relative(0);
}

int owlrtc_init(int hz, void (*tick_cb)(void))
{
        cb = tick_cb;
        rtc_init();
        rtc_set_callback(alarm);
        rtc_set_alarm_relative(0);
        return 0;
}
