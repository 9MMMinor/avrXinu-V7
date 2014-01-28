#define OWL_TRACE OWL_TRACE_DATA

/* This file reuses code from the lwIP project */

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
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include <stdlib.h>
#include <string.h>

#include <owl/core/owl_shell.h>
#include <owl/core/owl_err.h>
#include <owl/core/owl_debug.h>
#include <owl/core/owl_timer.h>


#include "wlp_ping.h"
#include "wlp_api.h"
#include "wlp_inet.h"

#define IP_PROTO_ICMP    1
#define IP_HLEN 20

struct icmp_echo_hdr {
        uint8_t type;
        uint8_t code;
        uint16_t chksum;
        uint16_t id;
        uint16_t seqno;
};

/* from lwip/ip.h */
struct ip_hdr {
        uint16_t _v_hl_tos; /* version / header length / type of service */
        uint16_t _len;      /* total length */
        uint16_t _id;       /* identification */
        uint16_t _offset;   /* fragment offset field */

#define IP_RF 0x8000        /* reserved fragment flag */
#define IP_DF 0x4000        /* dont fragment flag */
#define IP_MF 0x2000        /* more fragments flag */
#define IP_OFFMASK 0x1fff   /* mask for fragmenting bits */

        uint16_t _ttl_proto;/* time to live / protocol*/
        uint16_t _chksum;   /* checksum */

        /* source and destination IP addresses */
        struct ip_addr src;
        struct ip_addr dest; 
};

struct icmp_ctx {
        struct ip_addr addr;
        struct icmp_echo_hdr *icmp;
        uint16_t datasize;
        void (*reply_cb)(void *ctx, int len, struct ip_addr *ip, int seq,
                         int type);
        void (*done_cb)(void *ctx, struct ip_addr *dst, int sent, int recvd);
        void *ctx;
        
        uint16_t id; /* icmp id */
        uint16_t seqno; /* next seqno to send */
        uint16_t count; /* number of icmp_echo to send */
        uint16_t sent;  /* number of icmp_echo sent */
        uint16_t recvd; /* replies */
        int sockid;
       
        struct owltmr_tmo *tmo;
};

static void icmp_done(struct icmp_ctx *priv);
static void send_cb(struct owltmr_tmo *tmo, void *ctx);
static void recv_cb(void *ctx, int sockid, int len);

/**
 * standard checksum
 *
 * @param dataptr points to start of data to be summed at any boundary
 * @param len length of data to be summed
 * @return host order (!) lwip checksum (non-inverted Internet sum) 
 *
 * @note accumulator size limits summable length to 64k
 * @note host endianess is irrelevant (p3 RFC1071)
 */
static uint16_t
standard_chksum(void *dataptr, uint16_t len)
{
        uint32_t acc;
        uint16_t src;
        uint8_t *octetptr;

        acc = 0;
        /* dataptr may be at odd or even addresses */
        octetptr = (uint8_t*)dataptr;
        while (len > 1)
        {
                /* declare first octet as most significant
                   thus assume network order, ignoring host order */
                src = (*octetptr) << 8;
                octetptr++;
                /* declare second octet as least significant */
                src |= (*octetptr);
                octetptr++;
                acc += src;
                len -= 2;
        }
        if (len > 0)
        {
                /* accumulate remaining octet */
                src = (*octetptr) << 8;
                acc += src;
        }
        /* add deferred carry bits */
        acc = (acc >> 16) + (acc & 0x0000ffffUL);
        if ((acc & 0xffff0000) != 0) {
                acc = (acc >> 16) + (acc & 0x0000ffffUL);
        }
        /* This maybe a little confusing: reorder sum using htons()
           instead of ntohs() since it has a little less call overhead.
           The caller must invert bits for Internet sum ! */
        return htons((uint16_t)acc);
}

/**
 *
 * Calculates the Internet checksum over a portion of memory.
 *
 * @param dataptr start of the buffer to calculate the checksum
 *                (no alignment needed)
 * @param len length of the buffer to calculate the checksum
 * @return checksum (as u16_t) to be saved directly in the protocol header
 *
 */
static uint16_t
inet_chksum(void *dataptr, uint16_t len)
{
        uint32_t acc;

        acc = standard_chksum(dataptr, len);
        while ((acc >> 16) != 0) {
                acc = (acc & 0xffff) + (acc >> 16);
        }
        return (uint16_t)~(acc & 0xffff);
}

int ping(const struct ip_addr *addr, int count, int datasize,
         void (reply_cb)(void *ctx, int len, struct ip_addr *ip, int seqno,
                         int type),
         void (done_cb)(void *ctx, struct ip_addr *dst, int sent, int recvd),
         void *ctx)
{
        struct icmp_ctx *priv;
        struct icmp_echo_hdr *icmp;
        int res;
        
        if ((priv = calloc(1, sizeof(*priv))) == NULL)
                return OWL_ERR_MEM;
        
        if ((priv->icmp = calloc(1, sizeof(*icmp) + datasize)) == NULL) {
                res = OWL_ERR_MEM;
                goto err;
        }
        
        if ((priv->sockid = wlp_socket(WLP_SOCK_RAW, IP_PROTO_ICMP)) <= 0) {
                res = OWL_ERR_MEM;
                goto err;
        }

        wlp_set_recv_cb(priv->sockid, recv_cb, priv);

        priv->addr = *addr;
        priv->count = count;
        priv->datasize = datasize;
        priv->id = 0;
        priv->reply_cb = reply_cb;
        priv->done_cb = done_cb;
        priv->ctx = ctx;
        
        /* icmp echo will be sent by the scheduled send_cb */
        if ((priv->tmo = owltmr_addtmo(0, 0, send_cb, priv)) == NULL) {
                res = OWL_ERR_MEM;
                goto err;
        }
        
        return 0;
        
err:
        icmp_done(priv);
        return res;
}


static void
send_cb(struct owltmr_tmo *tmo, void *ctx)
{
        struct icmp_ctx *priv = ctx;
        struct icmp_echo_hdr *icmp = priv->icmp;
        int size;
        int err;
        
        /* all icmp_echo sent */
        if (priv->seqno == priv->count) {
                if (priv->done_cb)
                        priv->done_cb(priv->ctx, &priv->addr,
                                      priv->sent, priv->recvd);
                
                icmp_done(priv);
                return;
        }
        
        size = sizeof(*icmp) + priv->datasize;
        
        icmp->type = ICMP_ECHO;
        icmp->code = 0;
        icmp->chksum = 0;
        icmp->id = htons(priv->id);
        icmp->seqno = htons(priv->seqno);

        icmp->chksum = inet_chksum(icmp, size);

        if ((err = wlp_sendto(priv->sockid, (char *) icmp, size,
                              &priv->addr, 0)) == size)
                priv->sent++;
        else
                owl_info("failed to send icmp echo seqno:%d err:%d",
                         priv->seqno, err);

        /* send next in 1000 ms */
        owltmr_modtmo(priv->tmo, 1000);
        priv->seqno++;        
}


static void
recv_cb(void *ctx, int sockid, int len)
{
        struct icmp_ctx *priv = ctx;
        char *buf = NULL;
        struct ip_hdr *ip;
        struct icmp_echo_hdr *icmp;
        int err = 0;
        
        owl_dbg("sockid:%d len:%d", sockid, len);

        /* if out of memory, return and allow us to get called again */        
        if ((buf = malloc(len)) == NULL) {
                owl_err("could not alloc recv buf");
                goto out;
        }

        /* recv ip packet */
        if ((err = wlp_recv(sockid, buf, len)) != len) {
                owl_err("recv failed:%d", err);
                goto out;
        }

        owl_hexdump("IP", buf, len);

        ip = (struct ip_hdr *) buf;
        icmp = (struct icmp_echo_hdr *) (buf + IP_HLEN); /* pull ip header */

        if (len < IP_HLEN + sizeof(*icmp)) {
                owl_err("too short:%d", len);
                goto out;
                
        } else if (ntohs(icmp->id) != priv->id) {
                owl_err("wrong id:%d", ntohs(icmp->id));
                goto out;
        }

        priv->recvd++;
        
        if (priv->reply_cb)
                priv->reply_cb(priv->ctx, len, &ip->src, ntohs(icmp->seqno),
                               icmp->type);

out:
        free(buf);
}

static void icmp_done(struct icmp_ctx *priv)
{
        if (priv->sockid > 0)
                wlp_close(priv->sockid);

        if (priv->tmo)
                owltmr_deltmo(priv->tmo);

        free(priv->icmp);
        free(priv);
}

static void reply_cb(void *ctx, int len, struct ip_addr *src, int seqno,
                     int type)
{
        struct owlsh_ctx *sh = ctx;
        
        if (type == ICMP_ER)
                owlsh_printf(sh, "%d bytes from %s: icmp_seq=%d\n\r",
                             len, inet_ntoa(src), seqno);
        else if (type == ICMP_DUR)
                owlsh_printf(sh, "%d bytes from %s: icmp_seq=%d "
                             "Destination Host Unreachable\n\r",
                             len, inet_ntoa(src), seqno);
        else
                owlsh_printf(sh, "%d bytes from %s: icmp_seq=%d Type:%d\n\r",
                             len, inet_ntoa(src), seqno, type);
                
}

static void done_cb(void *ctx, struct ip_addr *dst, int sent, int recvd)
{
        struct owlsh_ctx *sh = ctx;
        owlsh_printf(sh, "--- %s ping statistics ---\n\r", inet_ntoa(dst));
        owlsh_printf(sh, "%d packets transmitted, %d received\n\r",
                     sent, recvd);
}

static void
ping_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        struct ip_addr ip = ip_addr_any;
        int count = 3;
        int size = 32;
        int err, i;

	/* parse arguments */
	for (i = 1; i < argc; i++) {
                
                if (strcmp(argv[i], "-c") == 0) {
                        /* -c <count> */
			if (++i >= argc)
				goto err_parse;
                        
                        count = atoi(argv[i]);
                        
                } else if (strcmp(argv[i], "-s") == 0) {
                        
                        /* -n <size> */
			if (++i >= argc)
				goto err_parse;
                        
                        size = atoi(argv[i]);

		} else if (!inet_aton(argv[i], &ip)) {
                        /* ip address */
                        goto err_parse;
                }
        }

        if (ip_addr_isany(&ip))
                goto err_parse;
        
        err = ping(&ip, count, size, reply_cb, done_cb, sh);
        
        owlsh_printf(sh, "%s %d (%s)\n\r", err >= 0 ? "ok" : "err", err,
                     owl_error(err));
        return;
        
err_parse:
        owlsh_printf(sh, "usage: ping [-c <count>] [-s <size>] <ip>\n\r");
}

struct owlsh_cmd cmd_ping = { "ping", ping_f, "ping" };



