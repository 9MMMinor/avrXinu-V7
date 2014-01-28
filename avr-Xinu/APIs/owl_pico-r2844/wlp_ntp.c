#define OWL_TRACE OWL_TRACE_ERR

#ifdef __avr8__
# include <avr/interrupt.h>
#endif

#include <stdlib.h>
#include <string.h>

#include <owl/core/owl_shell.h>
#include <owl/core/owl_err.h>
#include <owl/core/owl_debug.h>
#include <owl/core/owl_timer.h>


#include "wlp_ntp.h"
#include "wlp_api.h"
#include "wlp_inet.h"

#define NTP_PORT 123

struct ntp_ctx {
        void (*done_cb)(void *ctx, struct ntp_tm *tm, int err);
        void *ctx;
        int sockid;
        struct owltmr_tmo *tmo;
};

struct ntp_hdr {
        uint8_t status;
        uint8_t stratum;
        uint8_t ppoll;
        uint8_t precision;
        uint32_t distance;
        uint32_t dispersion;
        uint32_t refid;
        uint32_t reftime_int;
        uint32_t reftime_frac;
        uint32_t org_int;
        uint32_t org_frac;
        uint32_t rec_int;
        uint32_t rec_frac;
        uint32_t xmt_int;
        uint32_t xmt_frac;
};

static void ntp_done(struct ntp_ctx *priv);
static void recv_cb(void *ctx, int sockid, int len);
static void done_cb(void *ctx, struct ntp_tm *tm, int err);
static void timeout_cb(struct owltmr_tmo *tmo, void *ctx);
static void ntp_f(struct owlsh_ctx *sh, int argc, const char *argv[]);

int
ntp_gettime(struct ip_addr *server,
             void done_cb(void *ctx, struct ntp_tm *tm, int err), void *ctx)
{
        int sockid;
        int res = 0;
        struct ntp_ctx *priv;
        struct ntp_hdr hdr;

        memset((void *) &hdr, 0, sizeof(hdr));
        hdr.status = 0x23; /* version: 4; mode: client */

        if ((priv = calloc(1, sizeof(*priv))) == NULL)
                return OWL_ERR_MEM;
        
        /* create a UDP socket */
        if ((sockid = wlp_socket(WLP_SOCK_DGRAM, 0)) <= 0) {
                res = OWL_ERR_MEM;
                goto err;
        }

        priv->done_cb = done_cb;
        priv->ctx = ctx;
        priv->sockid = sockid;

        /* Register a timeout that will be invoked in 5 seconds. The timeout
         * will close the socket and consider the ntp server unavailable
         */
        if ((priv->tmo = owltmr_addtmo(5000, 0, timeout_cb, priv)) == NULL) {
                res = OWL_ERR_MEM;
                goto err;
        }
        
        /* connect to UDP socket to the NTP server to make sure that any
         * response gets routed to this socket.
         */
        if ((res = wlp_connect(sockid, server, NTP_PORT)))
                goto err;

        if ((res = wlp_set_recv_cb(sockid, recv_cb, priv)) < 0)
                goto err;
        
        /* send the NTP request */
        if ((res = wlp_send(sockid, (char *) &hdr, sizeof(hdr))) < 0)
                goto err;

        
        return 0;

err:
        ntp_done(priv);
        return res;
}


/*
 * ntp_offtime() is based on __offtime() in glibc, which has the license
 * below.
 *
 */

/* Copyright (C) 1991, 1993, 1997, 1998, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA. */

int
ntp_offtime(const uint32_t *t, struct ntp_tm *tm)
{
        const uint16_t __mon_yday[2][13] = {
                /* normal years.  */
                { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
                /* leap years.  */
                { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
        };

#define  SECS_PER_HOUR   (60 * 60)
#define  SECS_PER_DAY   (SECS_PER_HOUR * 24UL)
        int offset = 0; /* seconds east of UTC */

        int32_t days = 0, rem = 0, y = 0;
        const uint16_t *ip = 0;
        days = *t / SECS_PER_DAY;
        rem = *t % SECS_PER_DAY;
        rem += offset;
        while (rem < 0) {
                rem += SECS_PER_DAY;
                --days;
        }

        while (rem >= SECS_PER_DAY) {
                rem -= SECS_PER_DAY;
                ++days;
        }

        tm->hour = rem / SECS_PER_HOUR;
        rem %= SECS_PER_HOUR;
        tm->min = rem / 60;
        tm->sec = rem % 60;
        /* January 1, 1970 was a Thursday.  */
        tm->wday = (4 + days) % 7;
        if (tm->wday < 0)
                tm->wday += 7;
        y = 1970;
#define DIV(a, b) ((a) / (b) - ((a) % (b) < 0))
#define LEAPS_THRU_END_OF(y) (DIV (y, 4) - DIV (y, 100) + DIV (y, 400))
# define __isleap(year)                                                 \
        ((year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0))
        while (days < 0 || days >= (__isleap (y) ? 366 : 365)) {
                /* Guess a corrected year, assuming 365 days per year.  */
                long int yg = y + days / 365 - (days % 365 < 0);
                
                /* Adjust DAYS and Y to match the guessed year.  */
                days -= ((yg - y) * 365
                         + LEAPS_THRU_END_OF (yg - 1)
                         - LEAPS_THRU_END_OF (y - 1));
                y = yg;
        }
        tm->year = y - 1900;
        tm->yday = days;
        ip = __mon_yday[__isleap(y)];
        for (y = 11; days < (long int) ip[y]; --y)
                continue;
        days -= ip[y];
        tm->mon = y;
        tm->mday = days + 1;
        return 1;
}

static void
ntp_done(struct ntp_ctx *priv)
{
        if (priv->sockid > 0)
                wlp_close(priv->sockid);

        if (priv->tmo)
                owltmr_deltmo(priv->tmo);
        
        free(priv);
}

static void
timeout_cb(struct owltmr_tmo *tmo, void *ctx)
{
        struct ntp_ctx *priv = ctx;
        if (priv->done_cb)
                priv->done_cb(priv->ctx, NULL, OWL_ERR_TIMEOUT);

        ntp_done(priv);
}

static void
recv_cb(void *ctx, int sockid, int len)
{
        struct ntp_ctx *priv = ctx;
        struct ntp_tm tm;
        struct ntp_hdr *hdr = NULL;
        uint32_t t; 
        int err = 0;
        
        owl_dbg("sockid:%d len:%d", sockid, len);

        if (len < sizeof(struct ntp_hdr)) {
                err = OWL_ERR_PROTO;
                goto out;
        }

        if ((hdr = malloc(len)) == NULL) {
                err = OWL_ERR_MEM;
                goto out;
        }
        
        if ((err = wlp_recv(sockid, (char *) hdr, len)) != len) {
                owl_err("recv failed:%d", err);
                goto out;
        }

        owl_hexdump("NTP", (char *) hdr, len);

        /* NTP timestamps are represented as a 64-bit unsigned
         * fixed-point number, in seconds relative to 0h on 1 January 1900.
         */

        /*
         * The server copies the Transmit Timestamp field of the request to
         * the Originate Timestamp in the reply and sets the Receive Timestamp
         * and Transmit Timestamp fields to the time of day according to the
         * server clock in NTP timestamp format.
         *
         */
        t = ntohl(hdr->rec_int);
        t -= 2208988800UL; /* 1970 */
        
        /* for some reason an ISR breaks registers used during muliplacation
         * and modulos of 32 bit integers. We need to disable interrupts
         * before entering such a function.
         */
#ifdef __avr8__ 
        cli();
#endif
        ntp_offtime(&t, &tm);
#ifdef __avr8__
        sei();
#endif

out:
        if (priv->done_cb)
                priv->done_cb(priv->ctx, &tm, err);

        free(hdr);
        ntp_done(priv);
}

static void
done_cb(void *ctx, struct ntp_tm *tm, int err)
{
        struct owlsh_ctx *sh = ctx;
        if (tm == NULL)
                owlsh_printf(sh, "failed to get ntp time:%s(%d)\n\r",
                             owl_error(err), err);
        else
                owlsh_printf(sh, "ntp time: %04d-%02d-%02d %02d:%02d:%02d\n\r",
                             tm->year + 1900, tm->mon + 1, tm->mday,
                             tm->hour, tm->min, tm->sec);
}

static void
ntp_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        struct ip_addr ip;
        int err;
        
        if (argc < 2)
                goto err_parse;

        if (!inet_aton(argv[1], &ip))
                goto err_parse;
        
        err = ntp_gettime(&ip, done_cb, sh);
        
        owlsh_printf(sh, "%s %d (%s)\n\r", err >= 0 ? "ok" : "err", err,
                     owl_error(err));
        return;
        
err_parse:
        owlsh_printf(sh, "usage: ntp <ntp-server>\n\r");
        
}

struct owlsh_cmd cmd_ntp = { "ntp", ntp_f, "get server time from ntp" };



