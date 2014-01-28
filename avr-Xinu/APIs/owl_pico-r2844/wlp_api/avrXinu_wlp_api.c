//
//  avrXinu_wlp_api.c
//  owl_pico_avrxinu
//
//  Created by Michael Minor on 5/4/12.
//  Copyright (c) 2012. All rights reserved.
//

/*
 * Programming interface for oWL Pico API, wlp_api.
 * Copyright (C) 2011 HD Wireless AB
 *
 */

/*
 * The struct ip_addr type is based on work from the lwIP network stack.
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification,  are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//*********************************************************************************************
// avr-Xinu // avr-Xinu // avr-Xinu // avr-Xinu // avr-Xinu // avr-Xinu // avr-Xinu 
//*********************************************************************************************
// (mmm) May 4, 2012:
// This file is modified from its original (owl_pico-r2844) because of the difficulty
// in making a clean interface to an admittedly slow host microprocessor (ATMega1284p,
// with 3.68 MHz clock). Inter-machine communications can always be a problem when there
// is no synchronization between machines. Here we have two machines linked to their
// respective UARTs both capable of sending and receiving data at mutually agreed baud-
// rates. So problems of the receiving process not being able to consume data as quickly
// as the sending process transmits it tend not to be the problem. The problem
// is one of getting set up to send or receive. For example, every communication with
// the SPB800 calls the function send_req(). If the calling function is a control function,
// that's it. If the calling function is a block read (ie wlp_recv()), then we have send_req()
// followed by block_read(). Similarly, for a block write function (eg wlp_send()), we
// have send_req() followed by a block_write(). The owl Pico API approach is to initialize
// the API by passing it the function addresses for the host UART read function, uart_read_f(),
// and write function, uart_write_f(). Simplifying the names to R() and W(), and their arguments
// to only the number of bytes to read or write, we have W(1)-R(1)-W(m)-R(n) for a simple
// call to send_req(), and W(1)-R(1)-W(m)-R(n)-W(nbytes) for a block write of nbytes. m is the
// number of bytes in the request command block, and n is the number of bytes in the acknowledge
// control block.
//
// It is the rapid transition from write to read that can be a problem for Xinu especially when
// attempting to use existing tty drivers. Xinu I/O is synchronous -- most drivers set up read/
// write buffers and transfer counts, enable device interrupts, and then go to sleep in some
// fashion until interrupt service awakens the calling process and notifies it that the read or
// write is done.  It has to be expected that there are context switches between the start and
// finish of the transfer. Treating the W()-R()-W()-R()-R()/W() as a sequence of independent i/o
// operations, therefore, is very problematic. One way to make this work is to use a very fast
// processor as with typical Linux system; a second way is to force the entire sequence to be
// atomic by using LOCKS (running with interrupts disabled) and using polled read/write functions
// (synchronous low-level i/o).  The following polled i/o functions for the ATmega1284p
// microprocessor work fine with the unmodified oWL API:

/*
 * Read len bytes from the UART. This function should block until len bytes
 * have been read.
 *
 * NOTE: This function must be modified for the specific platform used.
 *
 */
/*
int board_uart_read_f(void *ctx, void *buf, int len)
{
	int i;
	char *in = buf;
	
	for (i = 0; i < len; i++) { // loop until all bytes are read
		// wait for a char
		while ( !(UCSR1A & (1<<RXC1)) )	{
			;
		}
		
		// read the char into the provided buffer
		in[i] = UDR1;
	}
	
	return len;
}
 */

/*
 * Write len bytes to the UART. This function should block until len bytes
 * have been written.
 *
 * NOTE: This function must be modified for the specific platform used.
 *
 */
/*
int board_uart_write_f(void *ctx, const void *buf, int len)
{
	int i;
	const char *out = buf;
  
	for (i = 0; i < len; i++) { // loop until all bytes are written
		
		// wait until we are ready to transmit next char
		while ( !(UCSR1A & (1<<UDRE1)) )
			; 
		
		// transmit the char
		UDR1 = out[i];
	} 
    
	return len;
}
*/

// All of our previous attempts using interrupt-driven I/O have been with mixed success. We
// tried input and output circular buffers ...

// Here's what we've done that is significant:
//		Modify send_req() to include two more arguments -- a pointer to a block buffer and the
// length of the block read or write. These are the buffer pointer and length arguments
// to wlp_recv() or wlp_send() and wlp_sendto().  The arguments are NULL
// and zero for any but the block read/write functions.
// All of our changes are enabled by defining INTERRUPT_DRIVEN_IO. Whether you are using
// blocking I/O or not is immaterial, because you can use this method without an OS like
// Xinu by using a flag mechanism in place of Xinu's send(message)/receive(message). You set
// a flag and fill a message in send() and loop until the flag is set in receive().
// These changes (INTERRUPT_DRIVEN_IO) allow us to write a UART driver which responds to
// read/write transitions entirely in interrupt service. It is possible that we may need
// to utilize deferred processing - a mechanism built into Xinu that prohibits context switches
// for short periods of time even though interrupts remain enabled.
#define INTERRUPT_DRIVEN_IO

#ifdef	INTERRUPT_DRIVEN_IO
#define send_req(a,b,c,d) sendRequest((a), (b), (c), (d), NULL, 0)
#endif
//*********************************************************************************************
// avr-Xinu // avr-Xinu // avr-Xinu // avr-Xinu // avr-Xinu // avr-Xinu // avr-Xinu 
//*********************************************************************************************



#include <string.h>
#include <stdlib.h>

#include "wlp_api.h"
#include "wlp_proto.h"
#include "wlp_inet.h"

#define IP_ADDR_ANY_VALUE 0x00000000UL
#define IP_ADDR_BROADCAST_VALUE 0xffffffffUL
#define WLP_SOCK_DYNAMIC /* use malloc/free to allocate new sockets */
//#define WLP_ENABLE_UART_LOCK

#ifdef WLP_ENABLE_UART_LOCK
# define WLP_LOCK() priv->write(priv->rw_ctx, NULL, 1);
# define WLP_UNLOCK() priv->write(priv->rw_ctx, NULL, 0);
#else
# define WLP_LOCK()
# define WLP_UNLOCK()
#endif

/* used by IP_ADDR_ANY and IP_ADDR_BROADCAST in ip_addr.h */
const struct ip_addr ip_addr_any = { IP_ADDR_ANY_VALUE };
const struct ip_addr ip_addr_broadcast = { IP_ADDR_BROADCAST_VALUE };


struct wlp_sock_t {
        int16_t id;
#define WLP_SOCK_FLAG_CONNECTED    (1 << 0) /* last known state for cb's*/
#define WLP_SOCK_FLAG_ERROR        (1 << 1) /* last known state for cb's*/
#define WLP_SOCK_FLAG_DELETED      (1 << 2)
        uint8_t flags;
        struct wlp_sock_t *next;
        void (*listen)(void *ctx, int sockid);
        void (*conn)(void *ctx, int sockid, int connected);
        void (*rx)(void *ctx, int sockid, int len);
        void *ctx; /* callback context for listen and conn */
        void *rx_ctx; /* callback context for rx */
};

struct wlp_ctx {
        int (*read)(void *ctx, void *data, int len);
        int (*write)(void *ctx, const void *data, int len);
        void *rw_ctx;
#define WLP_FLAG_LINKUP   (1 << 0)
#define WLP_FLAG_GOT_ADDR (1 << 1)
#define WLP_FLAG_POLL     (1 << 2)
        uint8_t flags;

        void (*link)(void *ctx, int link);
        void *link_ctx;

        void (*addr)(void *ctx, const struct ip_addr *ip);
        void *addr_ctx;

        void (*lookup)(void *ctx, const struct ip_addr *ip);
        void *lookup_ctx;

#ifndef WLP_SOCK_DYNAMIC
#define WLP_SOCK_MAX 8
        struct wlp_sock_t sock[WLP_SOCK_MAX];
#endif /* !WLP_SOCK_DYNAMIC */

} wlp_ctx;

struct wlp_ctx *priv = NULL;
struct wlp_sock_t *sock_head = NULL;


static int link_status(void);
static int net_config(const struct ip_addr *ip, const struct ip_addr *netmask,
                      const struct ip_addr *gw, const struct ip_addr *dns,
                      int dhcp);
static int net_info(struct ip_addr *ip, struct ip_addr *netmask,
                    struct ip_addr *gw, struct ip_addr *dns, int *dhcp,
                    struct ip_addr *dns_lookup, int *dns_result);
static int sock_status(int16_t sockid, int8_t *state);

#ifdef INTERRUPT_DRIVEN_IO
extern int owluart_Startup(void *ctx);
extern int sendRequest(	struct wlp_req *req, int reqlen,
						struct wlp_cfm *cfm, int cfmlen,
						uint8_t *buf, int buflen);
#else
static int send_req(struct wlp_req *req, int reqlen,
					struct wlp_cfm *cfm, int cfmlen);
#endif
static struct wlp_sock_t *sock_alloc(void);
static struct wlp_sock_t *sock_find(int id);
static void sock_free(struct wlp_sock_t *sock);

#ifdef WLP_ENABLE_UART_LOCK
static int send_req_nolock(struct wlp_req *req, int reqlen, struct wlp_cfm *cfm,
                           int cfmlen);
#else
# define send_req_nolock send_req
#endif

#ifdef INTERRUPT_DRIVEN_IO
int wlp_init(int (*readf)(void *ctx, void *data, int len),
             int (*writef)(void *ctx, const void *data, int len),
             void *ctx)
{
	struct wlp_reset_req req;
	
	req.id = 0;
	priv = &wlp_ctx;
	memset(priv, 0, sizeof(*priv));
	priv->read = readf;
	priv->write = writef;
	priv->rw_ctx = ctx;
	owluart_Startup((struct wlp_req *)&req);
	return (0);
}
#else
int wlp_init(int (*readf)(void *ctx, void *data, int len),
             int (*writef)(void *ctx, const void *data, int len),
             void *ctx)
{
        char c = 'w';

        priv = &wlp_ctx;
        memset(priv, 0, sizeof(*priv));

        priv->read = readf;
        priv->write = writef;
        priv->rw_ctx = ctx;

        priv->write(priv->rw_ctx, &c, 1);
        priv->read(priv->rw_ctx, &c, 1);
		while (c == '.')	{
			priv->read(priv->rw_ctx, &c, 1);
		}

        return 0;
}
#endif /*INTERRUPT_DRIVEN_IO*/


int wlp_set_baudrate(uint32_t baudrate, uint8_t rtscts)
{
        struct wlp_uart_config_req req;
        struct wlp_uart_config_cfm cfm;
        
        int err;
        req.id = WLP_UART_CONFIG;
        req.baudrate = htonl(baudrate);
        req.rtscts = rtscts;

        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0) {
                return err;
        }

        return cfm.res;
}

#ifdef INTERRUPT_DRIVEN_IO
int wlp_reset(void)
{
	struct wlp_reset_req req;
	
	req.id = WLP_RESET;
	owluart_Startup((struct wlp_req *)&req);
	return (0);
}

#else
int wlp_reset(void)
{
        struct wlp_reset_req req;
        struct wlp_reset_cfm cfm;
        int err;
        char w = 'w';
        char c;
        
        req.id = WLP_RESET;
        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        /* Only 57600 supported */
        WLP_LOCK();
        priv->write(priv->rw_ctx, &w, 1);
        priv->read(priv->rw_ctx, &c, 1);
        while (c == '.') {
                priv->write(priv->rw_ctx, &w, 1);
                priv->read(priv->rw_ctx, &c, 1);
        }
        WLP_UNLOCK();
        return cfm.res;
}
#endif /*INTERRUPT_DRIVEN_IO*/


int wlp_get_fw_version(uint32_t *code, char *build, int len)
{
        struct wlp_fw_version_req req;
        struct wlp_fw_version_cfm cfm;
        int err;
        
        memset(&req, 0, sizeof(req));

        req.id = WLP_FW_VERSION;
        
        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        if (cfm.res < 0)
                return cfm.res;

        *code = cfm.code;
        strncpy(build, cfm.build, len);
        return cfm.res;
}


void wlp_set_link_cb(void (*link_cb)(void *ctx, int link), void *ctx)
{
        priv->link = link_cb;
        priv->link_ctx = ctx;
}


int wlp_linkup(struct wl_ssid_t *ssid, const char *key, int wep)
{
        struct wlp_link_up_req req;
        struct wlp_link_up_cfm cfm;
        int err;
        memset(&req, 0, sizeof(req));

        req.id = WLP_LINK_UP;

        if (ssid)
                memcpy(&req.ssid, ssid, sizeof(req.ssid));
        
        if (key)
                strncpy(req.key, key, sizeof(req.key) - 1);
        
        req.wep = (uint8_t) wep;
        
        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        return cfm.res;
}


int wlp_linkdown(void)
{
        struct wlp_link_down_req req;
        struct wlp_link_down_cfm cfm;
        int err;

        req.id = WLP_LINK_DOWN;

        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        return cfm.res;
}


int wlp_set_ipaddr(const struct ip_addr *ip, const struct ip_addr *netmask,
                   const struct ip_addr *gw, const struct ip_addr *dns)
{
        return net_config(ip, netmask, gw, dns, 0);
}


int wlp_get_ipaddr(struct ip_addr *ip, struct ip_addr *netmask,
                   struct ip_addr *gw, struct ip_addr *dns)
{
        return net_info(ip, netmask, gw, dns, NULL, NULL, NULL);
}

int wlp_set_dhcp(int enable)
{
        return net_config(NULL, NULL, NULL, NULL, enable);
}


int wlp_get_dhcp(int *enabled)
{
        return net_info(NULL, NULL, NULL, NULL, enabled, NULL, NULL);
}

int wlp_set_dhcpd(int enable)
{
        struct wlp_net_dhcpd_req req;
        struct wlp_net_dhcpd_cfm cfm;
        int err;

        req.id = WLP_NET_DHCPD;
        req.enable = enable;
        
        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        return cfm.res; 
}

void wlp_set_ipaddr_cb(void (*addr_cb)(void *ctx, const struct ip_addr *addr),
                       void *ctx)
{
        priv->addr = addr_cb;
        priv->addr_ctx = ctx;
}


int wlp_get_hwaddr(struct wl_mac_addr_t *hwaddr)
{
        struct wlp_link_info_req req;
        struct wlp_link_info_cfm cfm;
        int err;

        req.id = WLP_LINK_INFO;

        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        if (hwaddr)
                memcpy(hwaddr, &cfm.hwaddr, sizeof(*hwaddr));
        
        return 0; /* always return 0 */        
}


int wlp_get_network(struct wlp_network_t *net)
{
        struct wlp_link_info_req req;
        struct wlp_link_info_cfm cfm;
        int err;

        req.id = WLP_LINK_INFO;

        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        if (net == NULL)
                return cfm.res;
        
        memcpy(&net->bssid, &cfm.ap, sizeof(net->bssid));
        memcpy(&net->ssid, &cfm.ssid, sizeof(net->ssid));
        
        return cfm.res;
}


int wlp_socket(int type, int proto)
{
        struct wlp_sock_open_req req;
        struct wlp_sock_open_cfm cfm;
        struct wlp_sock_t *sock;
        int err;

        if ((sock = sock_alloc()) == NULL)
                return WLP_ERR_MEM;
        
        req.id = WLP_SOCK_OPEN;
        req.type = type;
        req.protocol = proto;
        
        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0) {
                sock_free(sock);
                return err;
        }
        
        if (cfm.res < 0) {
                sock_free(sock);
                return cfm.res;
        }
        
        sock->id = cfm.res;
        sock->next = NULL;

        /* add socket to list */
        if (sock_head == NULL) {
                sock_head = sock;

        } else {
                struct wlp_sock_t *iter;
                for (iter = sock_head; iter->next; iter = iter->next);
                iter->next = sock;
        }
        
        return sock->id;
}


int wlp_set_conn_cb(int sockid,
                    void (*conn_cb)(void *ctx, int sockid, int connected),
                    void *ctx)
{
        struct wlp_sock_t *sock;
        if ((sock = sock_find(sockid)) == NULL)
                return WLP_ERR_PARAM;

        sock->conn = conn_cb;
        sock->ctx = ctx;
        return 0;
}


int wlp_set_recv_cb(int sockid,
                    void (*recv_cb)(void *ctx, int sockid, int len),
                    void *ctx)
{
        struct wlp_sock_t *sock;
        if ((sock = sock_find(sockid)) == NULL)
                return WLP_ERR_PARAM;

        sock->rx = recv_cb;
        sock->rx_ctx = ctx;
        return 0;
}

int wlp_bind(int sockid, const struct ip_addr *ip, uint16_t port)
{
        struct wlp_sock_bind_req req;
        struct wlp_sock_bind_cfm cfm;
        int err;

        req.id = WLP_SOCK_BIND;
        req.sockid = htons(sockid);
        req.ip = *ip;
        req.port = htons(port);
        
        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        return cfm.res;
}


int wlp_listen(int sockid, int backlog,
               void (*listen_cb)(void *ctx, int sockid),
               void *ctx)
{
        struct wlp_sock_listen_req req;
        struct wlp_sock_listen_cfm cfm;
        struct wlp_sock_t *sock;
        int err;

        if ((sock = sock_find(sockid)) == NULL)
                return WLP_ERR_PARAM;

        sock->listen = listen_cb;
        sock->ctx = ctx;
        sock->flags = 0; /* clear any error flag */
        
        req.id = WLP_SOCK_LISTEN;
        req.sockid = htons(sockid);
        req.backlog = backlog;
        
        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        return cfm.res;
}


int wlp_connect(int sockid, const struct ip_addr *ip, uint16_t port)
{
        struct wlp_sock_connect_req req;
        struct wlp_sock_connect_cfm cfm;
        struct wlp_sock_t *sock;
        int err;

        if ((sock = sock_find(sockid)) == NULL)
                return WLP_ERR_PARAM;

        sock->flags = 0; /* clear any error flag */
        
        req.id = WLP_SOCK_CONNECT;
        req.sockid = htons(sockid);
        req.ip = *ip;
        req.port = htons(port);
        
        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        return cfm.res;
}


int wlp_accept(int sockid)
{
        struct wlp_sock_accept_req req;
        struct wlp_sock_accept_cfm cfm;
        struct wlp_sock_t *sock;
        struct wlp_sock_t *iter;
        int err;

        if ((sock = sock_alloc()) == NULL)
                return WLP_ERR_MEM;

        req.id = WLP_SOCK_ACCEPT;
        req.sockid = htons(sockid);
        
        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0) {
                sock_free(sock);
                return err;
        }

        if (cfm.res < 0) {
                sock_free(sock);
                return cfm.res;
        }
        
        sock->id = cfm.res;
        sock->next = NULL;
        sock->flags |= WLP_SOCK_FLAG_CONNECTED;
        
        /* sock_head always != NULL, since we have at least our listening
         * sock
         */
        for (iter = sock_head; iter->next; iter = iter->next);
        iter->next = sock;
        
        return sock->id;
}


int wlp_close(int sockid)
{
        struct wlp_sock_close_req req;
        struct wlp_sock_close_cfm cfm;
        struct wlp_sock_t *iter;
        struct wlp_sock_t *prev = NULL;
        
        req.id = WLP_SOCK_CLOSE;
        req.sockid = htons(sockid);

        /* make sure that we delete the socket from our local list even though
         * the request fails. This might happen if the device was restarted
         * with wlp_reset() while we still had sockets open
         */
        (void) send_req((struct wlp_req *) &req, sizeof(req),
                        (struct wlp_cfm *) &cfm, sizeof(cfm));

        for (iter = sock_head; iter; prev = iter, iter = iter->next) {
                if (iter->id != sockid)
                        continue;

                /*
                 * Already a pending close for this socket? Maybe we are trying
                 * to close another socket with the same id. Note that there
                 * is a possibility that we have two or more sockets in our
                 * sock_head list that share the same id's. This will happen
                 * if wlp_close() is closed from any callbacks invoked from
                 * wlp_poll() _and_ wlp_connect() or wlp_accept() is called
                 * before wlp_poll() has returned.
                 *
                 */
                if (iter->flags & WLP_SOCK_FLAG_DELETED)
                        continue;
                
                /* invoked from a wlp callback? then just mark the socket
                 * as deleted, and it will be removed when polling is complete,
                 * from wlp_poll()
                 */
                if (priv->flags & WLP_FLAG_POLL) {
                        iter->flags |= WLP_SOCK_FLAG_DELETED;
                        return 0;
                }

                if (prev == NULL)
                        sock_head = sock_head->next;
                else
                        prev->next = iter->next;
                
                sock_free(iter);
                return 0;
        }

        return 0;
}
 
int wlp_recv(int sockid, char *buf, int16_t len)
{
        struct wlp_sock_recv_req req;
        struct wlp_sock_recv_cfm cfm;

        req.id = WLP_SOCK_RECV;
        req.sockid = htons(sockid);
        req.len = htons(len);

#ifdef	INTERRUPT_DRIVEN_IO
	return ( sendRequest((struct wlp_req *) &req, sizeof(req),
						 (struct wlp_cfm *) &cfm, sizeof(cfm),
						 (uint8_t *)buf, len) );
#else
		int err;
        WLP_LOCK();
        
        if ((err = send_req_nolock((struct wlp_req *) &req, sizeof(req),
                                   (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0) {
                WLP_UNLOCK();
                return err;
        }

        if (cfm.res < 0) {
                WLP_UNLOCK();
                return cfm.res;
        }

        /* should never happen */
        if (cfm.res > len) {
                WLP_UNLOCK();
                return WLP_ERR_PROTO;
        }
                
        /* read result number of bytes into buf */
        err = priv->read(priv->rw_ctx, buf, cfm.res);
        WLP_UNLOCK();
        return err;
#endif
}


int wlp_send(int sockid, const char *buf, int16_t len)
{
        struct wlp_sock_send_req req;
        struct wlp_sock_send_cfm cfm;

        req.id = WLP_SOCK_SEND;
        req.sockid = htons(sockid);
        req.len = htons(len);
	
#ifdef	INTERRUPT_DRIVEN_IO
		return ( sendRequest((struct wlp_req *) &req, sizeof(req),
							 (struct wlp_cfm *) &cfm, sizeof(cfm),
							 (uint8_t *)buf, len) );
#else

		int err;
        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        if (cfm.res < 0)
                return cfm.res;

        /* should never happen */
        if (cfm.res > len)
                return WLP_ERR_PROTO;

        /* write result number of bytes from buf */
        return priv->write(priv->rw_ctx, buf, cfm.res);
#endif
}


int wlp_sendto(int sockid, const char *buf, int16_t len,
               const struct ip_addr *ip, uint16_t port)
{
        struct wlp_sock_sendto_req req;
        struct wlp_sock_sendto_cfm cfm;
        
        req.id = WLP_SOCK_SENDTO;
        req.sockid = htons(sockid);
        req.len = htons(len);
        req.addr = *ip;
        req.port = htons(port);
	
#ifdef	INTERRUPT_DRIVEN_IO
		return ( sendRequest((struct wlp_req *) &req, sizeof(req),
							 (struct wlp_cfm *) &cfm, sizeof(cfm),
							 (uint8_t *)buf, len) );
#else
		int err;

        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        if (cfm.res < 0)
                return cfm.res;

        /* should never happen */
        if (cfm.res > len)
                return WLP_ERR_PROTO;

        /* write result number of bytes from buf */
        return priv->write(priv->rw_ctx, buf, cfm.res);
#endif
}


int wlp_get_peeraddr(int sockid, struct ip_addr *peer)
{
        struct wlp_sock_info_req req;
        struct wlp_sock_info_cfm cfm;
        int err;

        req.id = WLP_SOCK_INFO;
        req.sockid = htons(sockid);
        
        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        if (cfm.res < 0)
                return cfm.res;
        
        *peer = cfm.peer;
        return cfm.res;              
}


int wlp_get_hostbyname(const char *host,
                       void (*lookup_cb)(void *ctx, const struct ip_addr *ip),
                       void *ctx)
{
        struct wlp_net_dnslookup_req req;
        struct wlp_net_dnslookup_cfm cfm;
        int err;

        if (host == NULL)
                return WLP_ERR_PARAM;
        
        if (strlen(host) > WLP_HOSTNAME_MAX_LENGTH)
                return WLP_ERR_PARAM;

        if (priv->lookup)
                return WLP_ERR_BUSY;
        
        req.id = WLP_NET_DNSLOOKUP;
        strcpy(req.host, host);
        
        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        if (cfm.res == 0) {
                priv->lookup = lookup_cb;
                priv->lookup_ctx = ctx;
        }
        
        return cfm.res;              
}



void wlp_poll(void)
{
        int res;
        struct ip_addr ip;
        struct ip_addr dns;
        int dns_result;
        struct wlp_sock_t *prev = NULL;
        struct wlp_sock_t *iter;
        
        if (priv == NULL)
                return;

        /* check wifi link status, invoke link_cb accordingly */
        res = link_status();
        if (res == 0 && priv->link && !(priv->flags & WLP_FLAG_LINKUP)) {
                priv->flags |= WLP_FLAG_LINKUP;
                priv->link(priv->link_ctx, 1);
                
        } else if (res < 0 && priv->link && (priv->flags & WLP_FLAG_LINKUP)) {
                priv->flags &= ~(WLP_FLAG_LINKUP);
                priv->link(priv->link_ctx, 0);
        }

        /* check network status, invoke addr_cb */
        res = net_info(&ip, NULL, NULL, NULL, NULL, &dns, &dns_result);
        if (res == 0 && priv->addr && !(priv->flags & WLP_FLAG_GOT_ADDR)) {
                priv->flags |= WLP_FLAG_GOT_ADDR;
                priv->addr(priv->addr_ctx, &ip);

        } else if (res < 0 && priv->addr && (priv->flags & WLP_FLAG_GOT_ADDR)) {
                priv->flags &= ~(WLP_FLAG_GOT_ADDR);
                priv->addr(priv->addr_ctx, NULL);
        }
                
        /* invoke any dns lookup cb's */
        if (res == 0 && priv->lookup) {
                if (dns_result == WLP_DNS_OK) {
                        priv->lookup(priv->lookup_ctx, &dns);
                        priv->lookup = NULL;
                } else if (dns_result == WLP_DNS_ERROR) {
                        priv->lookup(priv->lookup_ctx, NULL);
                        priv->lookup = NULL;
                }
        }

        /* to avoid closing sockets while iterating through them */
        priv->flags |= WLP_FLAG_POLL;

        /* check status for each sock, invoke conn cb or rx cb accordingly */
        for (iter = sock_head; iter; iter = iter->next) {
                int avail;
                int8_t state;
                
                if ((avail = sock_status(iter->id, &state)) < 0)
                        /* fail to get status, consider this socket lost.
                         * This might happen after wlp_reset() if some sockets
                         *  were open prior to the call
                         */
                        state = WLP_SOCK_STATE_ERROR;
                
                 if (state == WLP_SOCK_STATE_CONNECTED &&
                    !(iter->flags & WLP_SOCK_FLAG_CONNECTED) && iter->conn) {
                        /* got connection */
                        iter->flags |= WLP_SOCK_FLAG_CONNECTED;
                        iter->flags &= ~WLP_SOCK_FLAG_ERROR;
                        iter->conn(iter->ctx, iter->id, 1);
                        
                } else if (state == WLP_SOCK_STATE_ERROR &&
                           !(iter->flags & WLP_SOCK_FLAG_ERROR) &&
                           iter->conn) {
                        /* connection error */
                        iter->flags &= ~WLP_SOCK_FLAG_CONNECTED;
                        iter->flags |= WLP_SOCK_FLAG_ERROR;
                        iter->conn(iter->ctx, iter->id, 0);
                        
                } else if (state == WLP_SOCK_STATE_ACCEPT && iter->listen) {
                        /* got incoming connection */
                        iter->listen(iter->ctx, iter->id);
                        
                } else if (avail > 0 && iter->rx) {
                        /* data pending, we check this before checking the
                         * state to make sure the client gets all the data
                         * before the rx_cb() signals a disconnection through
                         * len 0 (next case)
                         */
                        iter->rx(iter->rx_ctx, iter->id, avail);
                        
                } else if (state == WLP_SOCK_STATE_DISCONNECTED &&
                           (iter->flags & WLP_SOCK_FLAG_CONNECTED) &&
                           iter->rx) {
                        /* closed conn */
                        iter->flags &= ~WLP_SOCK_FLAG_CONNECTED;
                        iter->rx(iter->rx_ctx, iter->id, 0);
                }
        }

        priv->flags &= ~WLP_FLAG_POLL;

        /* if wlp_close() was called from a callback, we need to finalize the
         * closing of those sockets here.
         */
        iter = sock_head;
        while (iter) {
                struct wlp_sock_t *sock = iter;
                if (!(iter->flags & WLP_SOCK_FLAG_DELETED))
                        goto loop;

                if (prev == NULL)
                        sock_head = sock_head->next;
                else
                        prev->next = iter->next;

                iter = iter->next;
                sock_free(sock);
                continue;
        loop:
                prev = iter;
                iter = iter->next;
        }
}


int wlp_set_mode(uint8_t mode)
{
        struct wlp_set_mode_req req;
        struct wlp_set_mode_cfm cfm;
        int err;

        req.id = WLP_SET_MODE;
        req.mode = mode;
        
        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        return cfm.res;
}


int wlp_get_mode(uint8_t *mode)
{
        struct wlp_get_mode_req req;
        struct wlp_get_mode_cfm cfm;
        int err;

        req.id = WLP_GET_MODE;
        
        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        *mode = cfm.mode;
        return cfm.res;
}


int wlp_set_channel(uint8_t channel)
{
        struct wlp_set_channel_req req;
        struct wlp_set_channel_cfm cfm;
        int err;
        
        req.id = WLP_SET_CHANNEL;
        req.channel = channel;
        
        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        return cfm.res;
}



static int link_status(void)
{
        struct wlp_link_status_req req;
        struct wlp_link_status_cfm cfm;
        int err;

        req.id = WLP_LINK_STATUS;

        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        return cfm.res;
}

static int net_config(const struct ip_addr *ip, const struct ip_addr *netmask,
                      const struct ip_addr *gw, const struct ip_addr *dns,
                      int dhcp)
{
        struct wlp_net_config_req req;
        struct wlp_net_config_cfm cfm;
        int err;

        req.id = WLP_NET_CONFIG;
        req.ip  = ip ? *ip : ip_addr_any;
        req.netmask = netmask ? *netmask : ip_addr_any;
        req.gw = gw ? *gw : ip_addr_any;
        req.dns_server = dns ? *dns : ip_addr_any;
        req.dhcp = dhcp;
        
        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;
        
        return cfm.res;
        
}

static int net_info(struct ip_addr *ip, struct ip_addr *netmask,
                    struct ip_addr *gw, struct ip_addr *dns, int *dhcp,
                    struct ip_addr *dns_lookup, int *dns_result)
{
        struct wlp_net_info_req req;
        struct wlp_net_info_cfm cfm;
        int err;

        req.id = WLP_NET_INFO;
        
        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        
        if (ip)
                *ip = cfm.ip;

        if (netmask)
                *netmask = cfm.netmask;

        if (gw)
                *gw = cfm.gw;

        if (dns)
                *dns = cfm.dns_server;

        if (dhcp)
                *dhcp = cfm.dhcp;

        if (dns_result)
                *dns_result = cfm.dns_result;

        if (dns_lookup)
                *dns_lookup = cfm.dns_lookup;

        return cfm.res;
}


static int sock_status(int16_t sockid, int8_t *state)
{
        struct wlp_sock_status_req req;
        struct wlp_sock_status_cfm cfm;
        int err;

        req.id = WLP_SOCK_STATUS;
        req.sockid = htons(sockid);
        
        if ((err = send_req((struct wlp_req *) &req, sizeof(req),
                            (struct wlp_cfm *) &cfm, sizeof(cfm))) < 0)
                return err;

        *state = cfm.state;
        return cfm.res;
}
     
#ifndef INTERRUPT_DRIVEN_IO
static int send_req(struct wlp_req *req, int reqlen,
                    struct wlp_cfm *cfm, int cfmlen)
{
        uint8_t totlen = sizeof(totlen) + reqlen;
        uint8_t acklen;
        WLP_LOCK();
        priv->write(priv->rw_ctx, &totlen, sizeof(totlen));
        priv->read(priv->rw_ctx, &acklen, sizeof(acklen));
        if (acklen != totlen) {
                WLP_UNLOCK();
                return WLP_ERR_PROTO;
        }
        
        priv->write(priv->rw_ctx, req, reqlen);
        priv->read(priv->rw_ctx, cfm, cfmlen);
        WLP_UNLOCK();
        cfm->res = ntohs(cfm->res);
        return 0;
}
#endif

#ifdef WLP_ENABLE_UART_LOCK
static int send_req_nolock(struct wlp_req *req, int reqlen,
                           struct wlp_cfm *cfm, int cfmlen)
{
        uint8_t totlen = sizeof(totlen) + reqlen;
        uint8_t acklen;
        priv->write(priv->rw_ctx, &totlen, sizeof(totlen));
        priv->read(priv->rw_ctx, &acklen, sizeof(acklen));
        if (acklen != totlen) {
                WLP_UNLOCK();
                return WLP_ERR_PROTO;
        }
        
        priv->write(priv->rw_ctx, req, reqlen);
        priv->read(priv->rw_ctx, cfm, cfmlen);
        cfm->res = ntohs(cfm->res);
        return 0;
}
#endif

static struct wlp_sock_t *sock_find(int id)
{
        struct wlp_sock_t *iter;
        for (iter = sock_head; iter; iter = iter->next)
                if (iter->id == id)
                        return iter;

        return NULL;
}


static struct wlp_sock_t *sock_alloc(void)
{
#ifdef WLP_SOCK_DYNAMIC
        return calloc(1, sizeof(struct wlp_sock_t)); 
#else 
        int i;
        for (i = 0; i < WLP_SOCK_MAX; i++) {
                struct wlp_sock_t *sock = &priv->sock[i];
                if (!sock->id) {
                        memset(sock, 0, sizeof(struct wlp_sock_t));
                        return sock;
                }
        }

        return NULL;
#endif /* WLP_SOCK_DYNAMIC */
}        


static void sock_free(struct wlp_sock_t *sock)
{
#ifdef WLP_SOCK_DYNAMIC
        free(sock);
#else
        if (sock)
                sock->id = 0;
#endif /* WLP_SOCK_DYNAMIC */
}
