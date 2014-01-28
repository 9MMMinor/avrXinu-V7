#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include <owl/core/owl_err.h>
#include <owl/drivers/owl_rtc.h>

void (*cb)(void) = NULL;


static void alarm(int data)
{
        if (cb)
                cb();
}

int owlrtc_init(int hz, void (*tick_cb)(void))
{
        static struct timeval value;
        static struct timeval interval;
        static struct itimerval timer;

        if (hz == 0)
                return OWL_ERR_PARAM;

        cb = tick_cb;
        value.tv_sec = 1;
        value.tv_usec = 0;
        interval.tv_sec = 0;
        interval.tv_usec = 1000000 / hz;
        timer.it_interval = interval;
        timer.it_value = value;

        setitimer(ITIMER_REAL, &timer, 0);
        signal(SIGALRM, alarm);
        return 0;
}
