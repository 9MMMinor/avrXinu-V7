#define OWL_TRACE OWL_TRACE_DBG
#include <stdlib.h>
#include <string.h>

#include <owl/core/owl_shell.h>
#include <owl/core/owl_err.h>
#include <owl/core/owl_debug.h>
#include <owl/core/owl_timer.h>

#include "wlp_ttcp.h"
#include "wlp_api.h"
#include "wlp_inet.h"

struct ttcp_ctx {
        int sockid;
        char *buf;
        uint16_t buflen;
        uint16_t nbuf;
        uint32_t start_ms;
        uint32_t stop_ms;
        int send;

        uint32_t recvd; /* recv mode only */
        int acceptid;   /* recv mode only */

        void (*progress_cb)(void *ctx, int progress);
        void (*done_cb)(void *ctx, int err, int bytes, uint32_t ms);
        void *ctx;
};

static void recv_cb(void *ctx, int sockid, int len);
static void listen_cb(void *ctx, int sockid);
static void conn_cb(void *ctx, int sockid, int connected);
static int ttcp_done(struct ttcp_ctx *priv, int err);
static int sendbuf(struct ttcp_ctx *ttcp);

/**
 * Start a ttcp connection for transmission. The ttcp transfer will
 * proceed asynchronously upon successful return.
 * 
 *
 * @return sockid if started successfully.
 */
int
ttcp_connect(const struct ip_addr *ip, uint16_t port, uint16_t nbuf,
             uint16_t buflen,
             void (*progress_cb)(void *ctx, int progress),
             void (*done_cb)(void *ctx, int err, int bytes, uint32_t ms),
             void *ctx)
{
        int err;
        struct ttcp_ctx *priv;
        
        owl_info("ip:%s port:%d nbuf:%d buflen:%d",
                 inet_ntoa(ip), port, nbuf, buflen);

        if (nbuf == 0 || buflen == 0 || port == 0)
                return OWL_ERR_PARAM;

        /* Allocate our private ttcp struct */
        if ((priv = calloc(1, sizeof(struct ttcp_ctx))) == NULL) {
                owl_info("fail to alloc priv");
                return OWL_ERR_MEM;
        }

        priv->nbuf = nbuf;
        priv->buflen = buflen;
        priv->progress_cb = progress_cb;
        priv->done_cb = done_cb;
        priv->ctx = ctx;
        priv->send = 1;
        
        /* Allocate a buffer to hold the data we're going to transmit */
        if ((priv->buf = malloc(priv->buflen)) == NULL) {
                owl_info("fail to alloc buf (%d b)", priv->buflen);
                return ttcp_done(priv, OWL_ERR_MEM);
        }
        
        /* Create a TCP socket */
        if ((priv->sockid = wlp_socket(WLP_SOCK_STREAM, 0)) < 0) {
                owl_info("fail to create sock err:%d", priv->sockid);
                return ttcp_done(priv, priv->sockid);
        }

        /* Connect the socket to the remote ip and port. The conn_cb
         * passed to wlp_connect() will be invoked when the connection
         * attempt is completed. The result will be passed in the
         * "connected" parameter to conn_cb.
         */
        if ((err = wlp_connect(priv->sockid, ip, port)) < 0) {
                owl_info("fail to connect err:%d", err);
                return ttcp_done(priv, err);
        }

        wlp_set_conn_cb(priv->sockid, conn_cb, priv);
        wlp_set_recv_cb(priv->sockid, recv_cb, priv);

        /* Wait for conn_cb() */
        return priv->sockid;
}


/**
 * Start a ttcp connection for reception. The ttcp transfer will
 * proceed asyncrously upon successful return.
 * 
 *
 * @return sockid if started successfully.
 */
int
ttcp_listen(uint16_t port, uint16_t buflen,
            void (*progress_cb)(void *ctx, int progress),
            void (*done_cb)(void *ctx, int err, int bytes, uint32_t ms),
            void *ctx)
{
        int err;
        struct ttcp_ctx *priv;
        
        owl_info("port:%d buflen:%d", port, buflen);

        /* Allocate our private ttcp struct */
        if ((priv = calloc(1, sizeof(struct ttcp_ctx))) == NULL) {
                owl_info("fail to alloc priv");
                return ttcp_done(priv, OWL_ERR_MEM);
        }

        priv->buflen = buflen;
        priv->progress_cb = progress_cb;
        priv->done_cb = done_cb;
        priv->ctx = ctx;

        /* Allocate a buffer to hold the data we're going to recv */
        if ((priv->buf = malloc(priv->buflen)) == NULL) {
                owl_err("fail to alloc buf (%d b)", priv->buflen);
                return ttcp_done(priv, OWL_ERR_MEM);
        }

        /* Create a TCP socket */
        if ((priv->sockid = wlp_socket(WLP_SOCK_STREAM, 0)) < 0) {
                owl_err("fail to create sock err:%d", priv->sockid);
                return ttcp_done(priv, priv->sockid);
        }

        /* Bind the socket to the specified port */
        if ((err = wlp_bind(priv->sockid, IP_ADDR_ANY, port)) < 0) {
                owl_err("fail to bind sock err:%d", err);
                return ttcp_done(priv, err);
        }

        /* Start listening, listen_cb() will be invoked when we get an
         * incoming connection
         */
        if ((err = wlp_listen(priv->sockid, 1, listen_cb, priv)) < 0) {
                owl_err("fail to listen err:%d", err);
                return ttcp_done(priv, err);
        }

        /* Wait for listen_cb() */
        return priv->sockid;
}

static void
conn_cb(void *ctx, int sockid, int connected)
{
        struct ttcp_ctx *priv = ctx;
        int i;
        
        owl_info("connected:%d", connected);
        
        /* In send mode: connection attempt failed or lost connection
         * In recv mode: lost connection
         */
        if (!connected) {
                ttcp_done(priv, OWL_ERR_CONN);
                return;
        }
        
        owl_assert(priv->send);
        owl_info("ttcp connected, start send");

        priv->start_ms = owltmr_get_ms();
        for (i = 0; i < priv->nbuf; i++) {
                int err;
                if ((err = sendbuf(priv)) < 0) {
                        owl_err("fail to send buf %d err:%d", i, err);
                        ttcp_done(priv, err);
                        return;
                }
                
                /* print some progress information */
                if (priv->progress_cb)
                        priv->progress_cb(priv->ctx, i);

        }
        priv->stop_ms = owltmr_get_ms();
        ttcp_done(priv, 0);
}

static void
listen_cb(void *ctx, int sockid)
{
        struct ttcp_ctx *priv = ctx;
        if ((priv->acceptid = wlp_accept(sockid)) < 0) {
                owl_err("fail to accept err:%d", priv->acceptid);
                ttcp_done(priv, priv->acceptid);
                return;
        }
        
        wlp_set_conn_cb(priv->acceptid, conn_cb, priv);
        wlp_set_recv_cb(priv->acceptid, recv_cb, priv);


        /* conn accepted, incoming data will be handled in the recv_cb() */
        owl_info("sockid:%d acceptid:%d", sockid, priv->acceptid);
        
        priv->start_ms = owltmr_get_ms();
}

static void
recv_cb(void *ctx, int sockid, int len)
{
        struct ttcp_ctx *priv = ctx;
        int err;

        owl_dbg("sockid:%d len:%d", sockid, len);
        
        /* if remote end shuts down when we're in send mode, we'll
         * get this information from wlp_send() anyway
         */
        if (priv->send) {
                owl_info("ignoring rx data");
                return;
        }

        /* rx done when remote end shuts down connection */
        if (len == 0) {
                owl_info("done len:0");
                priv->stop_ms = owltmr_get_ms();
                ttcp_done(priv, 0);
                return;
        }

        /* get the data, recv_cb() will be invoked as long as there is data
         * to fetch, so done care about reading out all the len bytes
         */
        
        if ((err = wlp_recv(sockid, priv->buf, priv->buflen)) < 0) {
                owl_err("recv failed err:%d", err);
                ttcp_done(priv, err);
                return;
        }
                
        
        owl_dbg("recv:%d", err);
        priv->nbuf++;
        priv->recvd += err;
        
        /* print some progress information */
        if (priv->progress_cb)
                priv->progress_cb(priv->ctx, priv->nbuf);
}

static int
sendbuf(struct ttcp_ctx *priv)
{
        int pos = 0;
        while (pos < priv->buflen) {
                int len;
                if ((len = wlp_send(priv->sockid, priv->buf + pos,
                                    priv->buflen - pos)) < 0)
                        return len;
                
                owl_dbg("send:%d", len);
                pos += len;
        }

        owl_assert(pos == priv->buflen);
        return pos;
}

static int
ttcp_done(struct ttcp_ctx *priv, int err)
{
        owl_info("err:%d", err);

        uint32_t bytes = priv->send ? priv->nbuf * priv->buflen : priv->recvd;
        uint32_t ms = owltmr_get_intvl(priv->start_ms, priv->stop_ms);
        
        if (priv->done_cb)
                priv->done_cb(priv->ctx, err, bytes, ms);
        
        free(priv->buf);
        wlp_close(priv->sockid);
        if (!priv->send)
                wlp_close(priv->acceptid);
        free(priv);
        return err;
}



static void
progress_cb(void *ctx, int i)
{
        struct owlsh_ctx *sh = ctx;
        owlsh_printf(sh, ".");
        if (i > 0 && i % 80 == 0)
                owlsh_printf(sh, "\n\r");
}

static void
done_cb(void *ctx, int err, int bytes, uint32_t ms)
{
        struct owlsh_ctx *sh = ctx;
        owlsh_printf(sh, "\n\r");
        
        if (err)
                owlsh_printf(sh, "ttcp err:%d (%s)\n\r", err, owl_error(err));
                
        else if (ms > 0)
                owlsh_printf(sh, "ttcp ok: %d bytes processed in %lu ms, "
                             "%lu.%lu KB/s\n\r",
                             bytes, ms, bytes / ms, bytes % ms);
        else
                owlsh_printf(sh, "ttcp ok: %d bytes processed in 0 ms\n\r",
                             bytes);
}

static void
ttcp_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int i;
        uint16_t port = 5000;          /* -p */
        uint16_t nbuf = 1024;          /* -n */
        uint16_t buflen = 1024;        /* -l */
        int connect = 0;              /* -t , -r */
        struct ip_addr ip = ip_addr_any;
        int err;

	/* parse arguments, ugly */
	for (i = 1; i < argc; i++) {
                
                if (strcmp(argv[i], "-p") == 0) {
                        /* -p <port> */
			if (++i >= argc)
				goto err_parse;
                        
                        port = atoi(argv[i]);
                        
                } else if (strcmp(argv[i], "-n") == 0) {
                        
                        /* -n <nbuf> */
			if (++i >= argc)
				goto err_parse;
                        
                        nbuf = atoi(argv[i]);

		} else if (strcmp(argv[i], "-l") == 0) {
			/* -l <buflen> */
			if (++i >= argc)
				goto err_parse;
                        
                        buflen = atoi(argv[i]);

		} else if (strcmp(argv[i], "-t") == 0) {
                        /* transmit */
                        connect = 1;

                } else if (strcmp(argv[i], "-r") == 0) {
                        /* receieve */
                        connect = 0;

		} else if (!inet_aton(argv[i], &ip)) {
                        /* ip address */
                        goto err_parse;
                }
        }

        if (connect && ip_addr_isany(&ip))
                goto err_parse;
        
        if (connect)
                err = ttcp_connect(&ip, port, nbuf, buflen,
                                   progress_cb, done_cb, sh);
        else
                err = ttcp_listen(port, buflen, progress_cb, done_cb, sh);


        if (err < 0)
                owlsh_printf(sh, "%s %d (%s)\n\r", err >= 0 ? "ok" : "err", err,
                             owl_error(err));
        else if (connect)
                owlsh_printf(sh, "ttcp connect %s:%d nbuf:%d buflen:%d "
                             "sockid:%d\n\r",
                             inet_ntoa(&ip), port, nbuf, buflen, err);
        else 
                owlsh_printf(sh, "ttcp listen port:%d buflen:%d lsockid:%d\n\r",
                             port, buflen, err);

        return;
        
err_parse:
        owlsh_printf(sh, "usage: ttcp [-trpnl]\n\r");
        
}

struct owlsh_cmd cmd_ttcp = { "ttcp", ttcp_f, "ttcp throughput test" };
