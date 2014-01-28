//
//  STAmain.c
//  owl_pico on STK500 ATmega1284p platform.
//
//  Created by Michael Minor on 4/4/12.
//  Copyright 2012. All rights reserved.
//


/* avr-xinu */
#include <avr-Xinu.h>	/* includes avr-libc <stdio.h>	*/
#include <tty.h>
#include <date.h>
#include <sleep.h>
#include <network.h>

#include <assert.h>
#include <avr/pgmspace.h>

#define OWL_TRACE OWL_TRACE_INFO

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <file.h>
#include <USART.h>
#include <owl/ports/avrxinu/owl.h>
#include <util/delay.h>
int Xinu_putc(char ch, FILE * stream);
int Xinu_getc(FILE * stream);
void USART_Flush(void);
int tick_per_loopTimer(int argc, int *argv);
int getutim(long *timvar);
int fileTest(void);
void printMem(void);
void My_free(void *p);
struct devsw OW_switch_tab;

#define TIMER_HZ 16

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

#define HTTP_PORT 80
struct httpd *http_handle = NULL;
int link_Up_semaphore;
void * owluart;

/* These two calls take care of all shell related output */

/* normal stdout */
static void stdout_outb(void *ctx, int c)
{
	unsigned char ch = (unsigned char)c;
	
///	/* Wait for empty transmit buffer */
///	while ( !( UCSR0A & (1<<UDRE0)) )
///		;
///	/* Put data into buffer, sends the data */
///	UDR0 = ch;

	Xinu_putc(ch, stdout);
}

/* used in error output routines */
void owl_putc(char c)
{
//	Xinu_putc(c, stderr);
	kprintf("%c",c);
}

static void link_cb(void *ctx, int link)
{

	if (link == 1)	{
		kprintf_P(PSTR("Link is up\n"));
		signal(link_Up_semaphore);
	}
	else	{
		kprintf_P(PSTR("Link is down\n"));
		sreset(link_Up_semaphore, 0);
	}
}

static void addr_cb(void *ctx, const struct ip_addr *ip)
{
        struct owlsh_ctx *sh = ctx;
        if (ip == NULL) {
			owlsh_printf(sh, "addr_cb ip:none\n\r");
                if (http_handle) {
                        httpd_destroy(http_handle);
                        http_handle = NULL;
                        owlsh_printf(sh, "httpd stopped\n\r$ ");
                }

        } else {
			owlsh_printf(sh, "addr_cb ip:%s\n\r", inet_ntoa(ip));
                if ((http_handle = httpd_create(HTTP_PORT)) != NULL)
                        owlsh_printf(sh, "httpd started\n\r$ ");
        }
}

int main(void)
{
	int err;
	struct owlsh_ctx *sh;
	
	/* This is an effective way of testing a Xinu driver 	*/
	/* without configuring the driver.                   	*/
	/* You call directly the driver function - no system 	*/
	/*              calls, like read(OWL, buff, len)     	*/
	struct devsw *devptr = &OW_switch_tab;				/*	*/
	devptr->dvminor = 0;								/*	*/
	devptr->dvname = "OWL";		/*   none of these are used	*/
	devptr->dvnum = 1;									/*	*/
    
    printf("Owl Pico on avr-Xinu\n");
	control(CONSOLE, TCMODER, NULL, NULL); /* set mode to RAW */
	
	owlboard_init();
	owlrtc_init(TIMER_HZ, NULL);
	owltemp_init();
	link_Up_semaphore = screate(0);	/* no link until signalled */

	if ((err = owltmr_init(TIMER_HZ)) < 0) {
		owl_err("owltmr_init failed err:%d(%s)", err, owl_error(err));
		return -1;
	}
	
	/* Initialize the host-WiFi UART */
	if ( (err = owl_Init(devptr)) != OK)	{
		owl_err("uart_init failed err:%d(%s)", err, owl_error(err));
		return -1;
	}

	if ((sh = owlsh_create(NULL, stdout_outb, NULL, OWLSH_ESCAPECHARS)) == NULL) {
		owl_err("owlsh_create failed");
		return -1;
	}
	
	if ((err = wlp_init((void *)owl_BlockRead, (void *)owl_BlockWrite, (void *)&owltab[0])) < 0) {
		owl_err("wlp_init failed err:%d(%s)", err, owl_error(err));
		return -1;
	}
	
	uint8_t mode;
//	kprintf("wlp_get_mode: ");
	wlp_get_mode(&mode);
	if (mode == WL_MODE_AP)	{
		owlsh_printf(sh, "resetting mode to STA\n\r");
		sleep10(2);
		wlp_set_mode(WL_MODE_STA);
		err = wlp_reset();
		owlsh_printf(sh, "%s %d (%s)\n\r",
                     err >= 0 ? "ok" : "err", err, owl_error(err));
		sleep(1);
		panic("Please reboot\n");
	}
//	kprintf("mode=%d\n",mode);
	
	sleep(1);

		
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
	owlsh_addcmd(sh, &cmd_socket, NULL);
	owlsh_addcmd(sh, &cmd_bind, NULL);
	owlsh_addcmd(sh, &cmd_listen, NULL);
	owlsh_addcmd(sh, &cmd_connect, NULL);
	owlsh_addcmd(sh, &cmd_close, NULL);
	owlsh_addcmd(sh, &cmd_send, NULL);
	owlsh_addcmd(sh, &cmd_sendto, NULL);
	owlsh_addcmd(sh, &cmd_get_peeraddr, NULL);

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

	struct ip_addr ip, netmask, gateway;
	struct wl_ssid_t ssid; /* default ssid */
	const char *wpa_key = "Lutetium";
	int ret;
	IP4_ADDR(&ip, 192, 168, 1, 110);
	IP4_ADDR(&netmask, 255, 255, 255, 0);
	IP4_ADDR(&gateway, 192, 168, 1, 1);

	strcpy(ssid.ssid, "linksys");
	ssid.len = strlen(ssid.ssid);
	
	// connect to default AP
//	kprintf("linkup: ");
	ret = wlp_linkup(&ssid, wpa_key, 0);
//	kprintf("ret=%d\n",ret);

	// set default IP address
//	kprintf("wlp_set_ipaddr: ");
	ret = wlp_set_ipaddr(&ip, &netmask, &gateway, NULL);
//	kprintf("ret=%d\n",ret);
        
	// Start the main poll loop to check the console input and making sure
	// that the owl pico API makes forward progress.

	resume( create(tick_per_loopTimer, 400, 100, "OT", 1, sh) );
	
	long now;
	char atime[26];
	
	wait(link_Up_semaphore);
	
	sleep(3);
	getutim(&now);
	ascdate(ut2ltim(now), atime);
	printf("Time: %s\n", atime);
	

	
	resume( create(fileTest, 600, 22, "RF", 0) );
		
	for (;;) {
		owlsh_key(sh, Xinu_getc(stdin));
	}                
	return 0;
}

int tick_per_loopTimer(int argc, int *argv)
{
	int myTick = 0;
	
	kprintf("Start timer loop\n$ ");
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

/*
 *------------------------------------------------------------------------
 * getutim  --  obtain time in seconds past Jan 1, 1970, ut (gmt)
 *				owl_pico version
 *------------------------------------------------------------------------
 */

static int nist_response;
static void recv_cb(void *ctx, int sockid, int len);
static Bool clkset = FALSE;

#define NIST_PORT 37

int getutim(long *timvar)
{
	STATWORD ps;
	int	sockid;
	uint32_t utnow;
	struct ip_addr nist_server;
	int resp;

	wait(clmutex);
	if ( !clkset )	{
		IP4_ADDR(&nist_server, 64, 147, 116, 229);	/* nist server */
		nist_response = screate(0);	/* done semaphore */
		/* create a UDP socket */
		sockid = wlp_socket(WLP_SOCK_DGRAM, 0);	/* protocol is ignored for DGRAM */
		wlp_connect(sockid, &nist_server, NIST_PORT);
		wlp_set_recv_cb(sockid, recv_cb, NULL);
		
		/* send junk packet to prompt */
		wlp_send(sockid, (const char *)"Xinu", 4);
		wait(nist_response);
		resp =  wlp_recv(sockid, (char *)&utnow, sizeof(utnow));
		if ( resp == sizeof(utnow) )	{
			disable(ps);
			clktime = net2xt( net2hl(utnow) );
			restore(ps);
		}
		clkset = TRUE;		/* right or wrong */
		wlp_close(sockid);
		sdelete(nist_response);
	}
	disable(ps);
	*timvar = clktime;
	restore(ps);
	signal(clmutex);

	return(0);
}

static void
recv_cb(void *ctx, int sockid, int len)
{
	
	signal(nist_response);
}


int 
fileTest( void )
{
	char atime[26];
	char *file1 = "temp1";
	char *file2 = "temp2";
	char *file3 = "mytemp";
	char *format = "Cannot open remote file %s\n";
	FILE *in, *out;
	int	from, to;
	int len;
	char *buf;

//	kprintf("fileTest\n");
//	printMem();
	if ( (out = fopen(file3, "nw")) == (FILE *)SYSERR ) {
		printf(format, file3);
//		return (SYSERR);
		goto sleeps;
	}
	ascdate(ut2ltim(clktime), atime);
	len = strlen(atime);
	atime[len]= '\n';
	write(fileno(out), (unsigned char *)atime, len+1);
	fclose(out);
	
//	copy file3 to file2 
	if ( (int)(in=fopen(file3, "ro")) == SYSERR) {
		printf(format, file3);
		return(SYSERR);
	}
	if ( (int)(out=fopen(file2, "nw")) == SYSERR) {
		fclose(in);
		printf(format, file2);
		return(SYSERR);
	}
	if ( ((int) (buf = (char *)getmem(512)) ) == SYSERR) {
		printf("no memory\n");
		return (SYSERR);
	} else {
		from = fileno(in);
		to = fileno(out);
		while ( (len = read(from, (unsigned char *)buf, 512)) > 0 )
			write(to, (unsigned char *)buf, len);
		freemem(buf, 512);
	}
	fclose(in);
	fclose(out);
	
	
	if ( rename(file2, file1) == SYSERR ) {
		printf("Cannot move %s\n", file2);
		return(SYSERR);
	}
	if ( remove(file3, 0) == SYSERR ) {
		printf("Cannot remove %s\n", file3);
		return(SYSERR);
	}
	
sleeps:	
	printf("Done.\n");
	
	for ( ;; )
		sleep(60);
	
	return 0 ;
}

#include <mem.h>
#include <avr/io.h>

extern char *_etext;
extern char *__bss_start, *__bss_end;
extern char *__data_start, *__data_end;
extern char *_end;

/*
 *------------------------------------------------------------------------
 *  printMem  -  print memory use and free list information
 *------------------------------------------------------------------------
 */

void printMem(void)
{
	int	i;
	struct	mblock	*mptr;
	unsigned free;
	unsigned avail;
	unsigned stkmem;
	
	/* calculate current size of free memory and stack memory */
	
	for( free=0,mptr=memlist.mnext ; mptr!=(struct mblock *)NULL ;
		mptr=mptr->mnext)
		free += mptr->mlen;
	for (stkmem=0,i=0 ; i<NPROC ; i++) {
		if (proctab[i].pstate != PRFREE)
			stkmem += (unsigned)proctab[i].pstklen;
	}
	kprintf("Flash memory: %ul bytes total, %u text\n",
			1L + (unsigned long)FLASHEND, (unsigned) &_etext);
	kprintf("RAM memory: %u bytes RAM, %u registers %u data, %u bss\n",
			1 + (unsigned)RAMEND, (unsigned) &__data_start,
			(unsigned) &__data_end - (unsigned) &__data_start,
			(unsigned) &__bss_end - (unsigned) &__bss_start);
	avail = (unsigned)RAMEND - (unsigned) &_end + 1;
	kprintf(" initially: %5u avail\n", avail);
	kprintf(" presently: %5u avail, %5u stack, %5u heap\n",
			free, stkmem, avail - stkmem - free);
	kprintf(" free list:\n");
	for( mptr=memlist.mnext ; mptr!=(struct mblock *)NULL ;
		mptr=mptr->mnext) {
		kprintf("   block at 0x%4x, length %5u (0x%x)\n",
				mptr, mptr->mlen, mptr->mlen);
	}
}
