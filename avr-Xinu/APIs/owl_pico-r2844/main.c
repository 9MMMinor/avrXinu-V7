/*!
 *  \file main.c
 *  \brief oWL Pico reference design software.
 *
 *  Copyright (C) 2010 HD Wireless AB
 *
 *  You should have received a copy of the license along with this library.
 */


/*! \defgroup wlp_ref oWL Pico Reference Design
 *
 * The pclient application is built on top of the oWL pico API and uses the
 * owl toolkit, both provided by H&D Wireless.
 *
 * pclient provides a command line interface to directly invoke most of the
 * API functions in oWL Pico API, a ttcp throughput test tool and a small
 * web server.
 *
 * pclient can be built and used on a linux desktop PC together with the
 * SPB800 device. It can also be built for an ATMEL Xplain board, running on an
 * 8-bit CPU with 8Kb RAM. All the platform differences are handled in the
 * owl driver layer and in the main.c file.
 *
 * To build pclient on a linux system, make -C ports/linux should produce the
 * pclient binary in the ports/linux dir.
 *
 * To build pclient on for the ATMEL Xplain board, make -C ports/avr8 should
 * produce the pclient.hex binary in the ports/avr8 dir.
 *
 * The pclient reference design can be used as a starting point for using the
 * oWL Pico API for a custom application and/or on a custom platform.
 * 
 *  @{
 */
#define OWL_TRACE OWL_TRACE_INFO

#ifdef __avrXinu__
#include <avr-Xinu.h>
#include <tty.h>
#else
#include <stdio.h>
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifdef __linux__
# include <sys/time.h>
# include <fcntl.h>
# include <unistd.h>
# define TIMER_HZ 10
#endif /* __linux__ */

#ifdef __avr8__
# include <avr/io.h>
# define F_CPU 32000000UL
# include <util/delay.h>
# define UART_SHELL   USARTC0
# define UART_PICO    USARTD0
# define TIMER_HZ (1000 / 64) /* prescaler set to 1/64 on a 32KHz clock
                               * counting up to 32768 */
#endif /* __avr8__ */

#ifdef __avrXinu__

#include <file.h>
#include <USART.h>
#include <owl/ports/avrxinu/owl.h>
int Xinu_putc(char ch, FILE * stream);
int Xinu_getc(FILE * stream);
void USART_Flush(void);
void uart_Screen(struct devsw *devptr);
//int shell_Input( int argc, int *argv );
int tick_per_loopTimer(int argc, int *argv);
struct devsw OW_switch_tab;
# define UART_SHELL   CONSOLE
# define UART_PICO    1
//# define TIMER_HZ TICK /* number of TICKs per second */
#define TIMER_HZ 16
#endif /* __avrXinu__ */

#include <wlp_api.h>
#include <wlp_inet.h>

#include <owl/core/owl_err.h>
#include <owl/core/owl_debug.h>
#include <owl/core/owl_shell.h>
#include <owl/core/owl_timer.h>
#include <owl/drivers/owl_board.h>
#include <owl/drivers/owl_uart.h>
#include <owl/drivers/owl_rtc.h>
#include <owl/drivers/owl_led.h>
#include <owl/drivers/owl_temp.h>
#include "wlp_shell.h"
#include "wlp_ttcp.h"
#include "wlp_ntp.h"
#include "wlp_ping.h"
#include "httpd/wlp_httpd.h"

struct httpd *http_handle = NULL;
int owluart = -1;

#ifdef __linux__
static int kbhit(void)
{
        struct timeval tv;
        fd_set fds;
        tv.tv_sec = 0;
        tv.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        select(0 + 1, &fds, NULL, NULL, &tv);
        return FD_ISSET(0, &fds);
}


static void stdout_outb(void *ctx, int c)
{
        putc(c, stdout);
        fflush(stdout);
}


void owl_putc(char c)
{
        putc(c, stderr);
}
#endif /* __linux__ */

#ifdef __avrXinu__

/* These two calls take care of all shell related output */

/* normal stdout */
static void stdout_outb(void *ctx, int c)
{
	unsigned char ch = (unsigned char)c;
	
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
		;
	/* Put data into buffer, sends the data */
	UDR0 = ch;

//	Xinu_putc(ch, stdout);	/* hangs */
//	kprintf("%c", c);	/* This is CRAZY, but it works */
}

/* used in error output routines */
void owl_putc(char c)
{
//	Xinu_putc(c, stderr);
	kprintf("%c",c);
}

#endif /* __avrXinu__ */


#ifdef __avr8__
void owl_putc(char c)
{
        owluart_outb(&UART_SHELL, c);
}
#endif /* __avr8__ */


static void link_cb(void *ctx, int link)
{
        struct owlsh_ctx *sh = ctx;
        owlsh_printf(sh, "link_cb link:%d\n\r", link);
}


static void addr_cb(void *ctx, const struct ip_addr *ip)
{
        struct owlsh_ctx *sh = ctx;
        if (ip == NULL) {
                owlsh_printf(sh, "addr_cb ip:none\n\r");
                if (http_handle) {
                        httpd_destroy(http_handle);
                        http_handle = NULL;
                        owlsh_printf(sh, "httpd stopped\n\r");
                }

        } else {
                owlsh_printf(sh, "addr_cb ip:%s\n\r", inet_ntoa(ip));
                if ((http_handle = httpd_create(80)) != NULL)
                        owlsh_printf(sh, "httpd started\n\r");
        }
}


//static void tick_cb(void)
//{
//	owltmr_tick();
//}


int main(int argc, char *argv[])
{
        int err;
        struct owlsh_ctx *sh;
        int demo_mode = 0; /* we use an optional demo mode on xplain (avr8) */
        
        owlboard_init();
        owlrtc_init(TIMER_HZ, NULL);
        owltemp_init();

        if ((err = owltmr_init(TIMER_HZ)) < 0) {
                owl_err("owltmr_init failed err:%d(%s)", err, owl_error(err));
                return -1;
        }
        
        /* We will use the owl_* portability layer for console
         * input/output and error/debug functionality. This allows us
         * to build this code on (for instance) a Linux PC system
         * during development. To build the code for the target 
         * host platform you need to 
         *  - Write a owl_env.h file for the host
         *  - Write a owl_uart.c file for the target platform, implementing
         *    the functions prototyped in owl_uart.h
         *  - Write a owl_led.c file for the target platform, implementing
         *    the functions prototyped in owl_led.h
         *  - Write a owl_rtc.c file for the target platform, implementing
         *    the functions prototyped in owl_rtc.h
         */

#ifdef __avr8__

        /* On the xplain board, skip demo mode if user pressed any button
         * during startup
         */
        demo_mode = 1;
        if (PORTF_IN != 0xff) /* XXX should live in owl_button.c */
                demo_mode = 0;

        /* Initialize the console UART */
        if (owluart_init(&UART_SHELL, UART_BAUDRATE_9600,
                         UART_DATABITS_8, UART_PARITY_NONE,
                         UART_STOPBITS_1, 0) < 0)
                return -1;
        
        /* Initialize the host-WiFi UART */
        if ((err = owluart_init(&UART_PICO, UART_BAUDRATE_57600,
                                UART_DATABITS_8, UART_PARITY_NONE,
                                UART_STOPBITS_1, 0)) < 0) {
                owl_err("uart_init pico err:%d(%s)", err, owl_error(err));
                return -1;
        }

        /* Initialize the host console input/output system. We will
         * only use the console for status output in the example
         * (using owlsh_print()) so the console command handling will
         * be unused.
         */
        if ((sh = owlsh_create(NULL, owluart_outb, &UART_SHELL,
                               OWLSH_ESCAPECHARS)) == NULL) {
                owl_err("owlsh_create failed");
                return -1;
        }

        /* Wait for wifi to start */
        _delay_ms(200);

        /* Setup the owl pico library. We pass the functions
         * that the library will use to read from and write to
         * the WiFi module.
         */
        if ((err = wlp_init(owluart_read, owluart_write, &UART_PICO)) < 0) {
                owl_err("wlp_init failed err:%d(%s)", err, owl_error(err));
                return -1;
        }

        /* keep track of the uart fd to support the set_baudrate command */
        owluart = &UART_PICO;

        _delay_ms(100);
        
#endif /* __avr8__ */
        
#ifdef __linux__
        int fd;
        char *devname;
        
        if (argc > 1 && !strcmp(argv[1], "-h")) {
                printf("%s [port]\n", argv[0]);
                return 0;
        }

        if(argc > 1)
                devname = argv[1];
        else
                devname = "/dev/ttyUSB0";


        if ((fd = open(devname, O_RDWR | O_NOCTTY)) < 0) {
                owl_err("Failed to open device '%s'", devname);
                return -1;
        }
        
        if ((err = owluart_init(&fd, UART_BAUDRATE_57600,
                                UART_DATABITS_8, UART_PARITY_NONE,
                                UART_STOPBITS_1, 0)) < 0) {
                owl_err("uart_init failed err:%d(%s)", fd, owl_error(fd));
                return -1;
        }

        if ((sh = owlsh_create(NULL, stdout_outb, NULL,
                               OWLSH_NOECHO | OWLSH_ESCAPECHARS)) == NULL) {
                owl_err("owlsh_create failed");
                return -1;
        }

        if ((err = wlp_init(owluart_read, owluart_write, (void *) fd)) < 0) {
                owl_err("wlp_init failed err:%d(%s)", err, owl_error(err));
                return -1;
        }

        sleep(1);

        /* keep track of the uart fd to support the set_baudrate command */
        owluart = &fd;
        
        owlsh_addcmd(sh, &cmd_send_file, NULL);
        owlsh_addcmd(sh, &cmd_recv_file, NULL);
        owlsh_addcmd(sh, &cmd_recv, NULL);
#endif /* __linux__ */
	
#ifdef __avrXinu__
	/* This is an effective way of testing a Xinu driver */
	/* without configuring the driver.                   */
	/* You call directly the driver function - no system */
	/*              calls, like read(OWL, buff, len)     */

	struct devsw *devptr = &OW_switch_tab;
	devptr->dvminor = 0;
	devptr->dvname = "OWL";		/* none of these are used */
	devptr->dvnum = 1;
//	struct tty *iptr = &tty[0];
//	struct owlcblk *owptr = &owltab[0];
	
	/* Initialize the host-WiFi UART */
	if ( (err = owl_Init(devptr)) != OK)	{
		owl_err("uart_init failed err:%d(%s)", err, owl_error(err));
		return -1;
	}
	
	if ((sh = owlsh_create(NULL, stdout_outb, NULL, OWLSH_ESCAPECHARS)) == NULL) {
		owl_err("owlsh_create failed");
		return -1;
	}
	
	if ((err = wlp_init((void *)owl_Read, (void *)owl_Write, devptr)) < 0) {
		owl_err("wlp_init failed err:%d(%s)", err, owl_error(err));
		return -1;
	}
	sleep(1);
	demo_mode = 1;
	
	/* keep track of the uart fd to support the set_baudrate command */
//	owluart = fd;
	
#endif /* __avrXinu__ */

        /* We register callbacks so that we get notified when the
         * WiFi link is established (link up) or torn down (link down)
         * and when and IP address has been assigned to the interface
         * (since this can happen at some point after the WiFi link
         * comes up if DHCP is used).
         */

        wlp_set_link_cb(link_cb, sh);

        wlp_set_ipaddr_cb(addr_cb, sh);
        
        /* Add commands which can be used to control the WiFi link.
         * link_cb() will be called when the connection is up.
         */

        owlsh_addcmd(sh, &cmd_linkup, NULL);
        owlsh_addcmd(sh, &cmd_linkdown, NULL);
        owlsh_addcmd(sh, &cmd_get_hwaddr, NULL);
        owlsh_addcmd(sh, &cmd_get_network, NULL);

        /* Add commands which can be used to control the ip address settings */
        owlsh_addcmd(sh, &cmd_set_ipaddr, NULL);
        owlsh_addcmd(sh, &cmd_get_ipaddr, NULL);
        owlsh_addcmd(sh, &cmd_set_dhcp, NULL);
        owlsh_addcmd(sh, &cmd_get_dhcp, NULL);
        owlsh_addcmd(sh, &cmd_set_dhcpd, NULL);
        owlsh_addcmd(sh, &cmd_get_hostbyname, NULL);

        /* Add commands which can be used to play around with the socket
         * interface of owl pico
         */
//       owlsh_addcmd(sh, &cmd_socket, NULL);
//        owlsh_addcmd(sh, &cmd_bind, NULL);
//        owlsh_addcmd(sh, &cmd_listen, NULL);
//        owlsh_addcmd(sh, &cmd_connect, NULL);
//        owlsh_addcmd(sh, &cmd_close, NULL);
//        owlsh_addcmd(sh, &cmd_send, NULL);
//        owlsh_addcmd(sh, &cmd_sendto, NULL);
//        owlsh_addcmd(sh, &cmd_get_peeraddr, NULL);

        /* Add misc commands */
        owlsh_addcmd(sh, &cmd_get_version, NULL);
        owlsh_addcmd(sh, &cmd_get_fw_version, NULL);
        owlsh_addcmd(sh, &cmd_set_mode, NULL);
        owlsh_addcmd(sh, &cmd_get_mode, NULL);
		owlsh_addcmd(sh, &cmd_set_channel, NULL);
        owlsh_addcmd(sh, &cmd_reset, NULL);
        owlsh_addcmd(sh, &cmd_set_baudrate, NULL);
        
        /* Add protocol implementation examples */
        owlsh_addcmd(sh, &cmd_ttcp, NULL);
        owlsh_addcmd(sh, &cmd_ntp, NULL);
        owlsh_addcmd(sh, &cmd_ping, NULL);
        owlsh_addcmd(sh, &cmd_help, NULL);

        /* We use a default configuration on xplain for demo purposes */
        if (demo_mode) {
                struct ip_addr ip, netmask;
                struct wl_ssid_t ssid; /* default ssid */
				int ret;

                IP4_ADDR(&ip, 192, 168, 1, 110);
                IP4_ADDR(&netmask, 255, 255, 255, 0);

                strcpy(ssid.ssid, "HD-WiFi");
                ssid.len = strlen(ssid.ssid);

                /* connect to default AP */
                ret = wlp_linkup(&ssid, 0, 0);

                /* set default IP address */
                ret = wlp_set_ipaddr(&ip, &netmask, NULL, NULL);
        }
        
        /* Start the main poll loop to check the console input and making sure
         * that the owl pico API makes forward progress.
         */

#ifdef __avrXinu__

	resume( create(tick_per_loopTimer, 400, 100, "OT", 1, sh) );
	control(CONSOLE, TCMODER, NULL, NULL);	/* set mode to RAW */
	

#endif /* __avrXinu__ */
	
        for (;;) {

#ifdef __avr8__
			if (owluart_poll(&UART_SHELL))
				owlsh_key(sh, owluart_inb(&UART_SHELL, 0));
#endif /* __avr8__ */

#ifdef __linux__
			if (kbhit()) {
				char line[80];
                char *p;

				fgets(line, sizeof(line), stdin);
				p = line;
				while (*p) {
					owlsh_key(sh, *p);
					p++;
				}
			}
#endif /* __linux__ */

#ifdef __avrXinu__

			owlsh_key(sh, Xinu_getc(stdin));

#else /* __avrXinu__ */
			wlp_poll();
			owltmr_poll();
#endif /* __avrXinu__ */

		}                
        
        return 0;
}

#ifdef __avrXinu__

int tick_per_loopTimer(int argc, int *argv)
{
	int myTick = 0;
	struct owlsh_ctx *sh = (struct owlsh_ctx *)argv[0];
	
	owlsh_printf(sh, "Start timer loop\r\n$ ");
	for ( ;; )	{
		sleep10(1);
		owltmr_tick();
		if (myTick++ >= TICK)	{
			myTick = 0;
			wlp_poll();
			owltmr_poll();
		}
	}
	return 0;
}
#endif /* __avrXinu__ */

/*!  @} */
