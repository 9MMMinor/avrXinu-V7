/*! \page License
 * Copyright (C) 2010, H&D Wireless AB All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of H&D Wireless AB may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY H&D WIRELESS AB ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
 * SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*!
 * \file *********************************************************************
 *
 * \brief wlp_httpd Web server
 *
 * This is a simple web server application built on the oWL-pico
 * low-footprint WiFi API.
 * 
 * The server listens for incoming connections on a specified TCP port
 * (normally port 80). When a connection is established the server
 * attempts to read a HTTP GET request from it, then the http_get()
 * function in web_contents.c is called to write the requested file
 * to the connection.
 *
 * See documentation in web_content.c for information on how to change or
 * ass pages to the web server.
 *
 */

#define OWL_TRACE OWL_TRACE_ERR

#ifdef __avr8__
#include <board.h>
#endif  /* __avr8__ */

#ifdef __avrXinu__
#include <avr-Xinu.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <wlp_api.h>
#include <wlp_inet.h>

#include <owl/core/owl_debug.h>

#include "httpd/wlp_httpd.h"
#include "httpd/match.h"
#include "httpd/web_content.h"

#define HTTPD_BUFFER_LEN 128

/* HTTP header line separator */
#define EOL_TOKEN "\r\n"

struct http_query {
        char *path;

        /* Data buffer */
        char buf[HTTPD_BUFFER_LEN];
};

struct httpd {
        /* Server listening socket (for incoming connections) */
        int lsock;
        /* Connected data socket for serving pages to a
         * client */
        int conn_sock;
        /* The current query being read */
        struct http_query http_query;
};

static void listen_cb(void *ctx, int sockid);
static void conn_status_cb(void *ctx, int sockid, int connected);
static void rx_cb(void *ctx, int sockid, int len);
static void http_rx(char *buf, size_t len, struct httpd *priv);
static int http_parse_line(char *line, struct httpd *priv);
static void http_close_conn(struct httpd *priv);

/* Startup the http server. It will listen for connections on the
 * specified port, listen_cb() will be called to accept incoming
 * connections.
 */
struct httpd *httpd_create(int port)
{
        int err;
        struct httpd *priv = calloc(1, sizeof(struct httpd));
        if (priv == NULL)
                return NULL;

        /* Create a listening socket for the HTTP server */
        if ( (priv->lsock = wlp_socket(WLP_SOCK_STREAM, 0) ) < 0 ) {
                owl_err("Failed to open TCP socket");
                goto err;
        }
        
        err = wlp_bind(priv->lsock,
                       IP_ADDR_ANY,
                       port);
        if ( err ) {
                owl_err("wlp_bind failed err: %d", err);
                goto err;
        }
        err = wlp_listen(priv->lsock,
                         1, /* conn backlog */
                         listen_cb,
                         priv);
        if ( err ) {
                owl_err("wlp_listen failed err: %d", err);
                goto err;
        }
        return priv;
err:
        if ( priv ) {
                if ( priv->lsock != WLP_INVALID_SOCKET_ID )
                        wlp_close(priv->lsock);
                
                free(priv);
        }
        return NULL;

}

/* Shutdown the http server. */
void httpd_destroy(struct httpd *priv)
{
        http_close_conn(priv);
        if ( priv->lsock != WLP_INVALID_SOCKET_ID ) {
                wlp_close(priv->lsock);
        }
        free(priv);
}

/* Close a connection and free any buffered data */
static void http_close_conn(struct httpd *priv) {

        if ( WLP_INVALID_SOCKET_ID != priv->conn_sock ) {
                wlp_close(priv->conn_sock);
                priv->conn_sock = WLP_INVALID_SOCKET_ID;
        }
}

/* The listen_cb() function is called whenever a new connection is
 * pending on the listening socket. */
static void listen_cb(void *ctx, int sockid) {
        struct httpd *priv = ctx;
        int sock;

        owl_info("Accepting new connection");
        if ( priv->conn_sock != WLP_INVALID_SOCKET_ID ) {
                owl_info("Closing socket %d", priv->conn_sock);
                http_close_conn(priv);
        }
        /* Accept the connection and store the socket for the newly
         * established connection in priv */
        sock = wlp_accept(priv->lsock);
        if ( sock < 0 ) {
                owl_err("wlp_accept failed lsock:%d err:%d", priv->lsock, sock);
                return;
        }

        wlp_set_conn_cb(sock, conn_status_cb, priv);
        wlp_set_recv_cb(sock, rx_cb, priv);
        
        owl_info("Socket %d", sock);
        priv->conn_sock = sock;
}

/* The conn_status_cb() function is called whenever a
 * data socket is connected (connected == 1) or
 * aborted (connected == 0). 
*/
static void conn_status_cb(void *ctx, int sockid, int connected) {
        struct httpd *priv = ctx;
        owl_info("connected:%d", connected);
        if ( connected ) {
                struct ip_addr addr;
                int err;
                err = wlp_get_peeraddr(sockid, &addr);
                if ( err ) {
                        owl_err("wlp_get_peeraddr failed err :%d", err);
                        return;
                }
                owl_info("Data connection to peer %s established",
                         inet_ntoa(&addr));
        }
        else {
                /* If the connection is properly closed by the remote
                 * end rx_cb() will be called instead so we only need
                 * to handle the error case here. */
                http_close_conn(priv);
                owl_info("Data connection unexpectedly lost");
        }
}

/* The rx_cb() function is called whenever data has arrived
 * on a connected data socket. It is also called with
 * len == 0 when the connection was closed by the peer.
 * The function will read the data from the socket and
 * pass it to http_rx() for processing.
 */
static void rx_cb(void *ctx, int sockid, int len) {
        struct httpd *priv = ctx;
        struct http_query *q = &priv->http_query;
        int rlen;
#ifdef __avr8__
        irqflags_t iflags;
#endif /* __avr8__ */
	
#ifdef __avrXinu__
//	STATWORD ps;
#endif
        
        if ( 0 == len ) {
                owl_info("Data connection closed by peer");
                http_close_conn(priv);
                return;
        }

        if (len > HTTPD_BUFFER_LEN)
                len = HTTPD_BUFFER_LEN;
        
        owl_info("%d bytes rx", len);

        /* We disable interrupts on avr8 here to make sure that we can read all
         * the expected data without risking buffer overruns.
         *
         * This should really be handled by a interrupt-driven avr8 uart driver
         * and a ringbuf. 
         *
         */
#ifdef __avr8__
        iflags = cpu_irq_save();
#endif /* __avr8__ */
#ifdef __avrXinu__
//	disable(ps);
#endif /* __avrXinu */
        rlen = wlp_recv(sockid, q->buf, len);
#ifdef __avr8__
        cpu_irq_restore(iflags);
#endif /* __avr8__ */
#ifdef __avrXinu__
//	restore(ps);
#endif /* __avrXinu */
        owl_info("recvd %d bytes", rlen);
        if (rlen < 0) {
                owl_err("wlp_recv failed err: %d", rlen);
                
        } else if (priv->conn_sock != WLP_INVALID_SOCKET_ID) {
                owl_info("passing %d bytes to parser", rlen);
                http_rx(q->buf, rlen, priv);
        } else {
                owl_info("discarding %d bytes", rlen);
        }
}

/* Split the input into HTTP header lines (lines terminated
 * by '\r\n') and call http_parse_line() on the first line. */
static void http_rx(char *buf, size_t len, struct httpd *priv) {
        char *p;

        owl_dbg("len:%d", len);

        /* We only parse the first line of the query */
        p = match_string(EOL_TOKEN,
                         NULL, /* no match offset */
                         buf,
                         len,
                         1 /* case sensitive */);
        if ( p ) {
                int err;

                *p = '\0';
                err = http_parse_line(buf, priv);
                if ( err ) {
                        goto err;
                }
                owl_info("closing connection");
                http_close_conn(priv);

        }

        return;
err:
        owl_info("Invalid query. Closing connection");
        http_close_conn(priv);
        
}

/* Parse a HTTP message line. 
 * If the line is a proper HTTP GET method then http_get() will
 * be called to attempt to retrieve the named HTML page for the
 * client. Note that http_get() is implemented in web_content.c.
 *
 * Return 0 if the line was a proper HTTP GET method (not that this
 * does not mean that the requested page existed, if it does not
 * exist then a HTTP 404 error message will be sent to the client).
 */
static int http_parse_line(char *line, struct httpd *priv) {
        char *p, *path;
        int err;

        owl_dbg("line:'%s'", line);
        /* Expected input is "GET <path> ...." */
        if ( strncmp(line, "GET ", 4) ) {
                return -1;
        }
        p = line + 4;
        while (isspace(*p)) p++;
        path = p;
        while (!isspace(*p)) p++;
        *p = '\0';

        err = http_get(path, priv->conn_sock);
        if ( err ) {
                owl_err("HTTP GET for invalid file \"%s\"\n",
                        path);
                return err;
        }
        return 0;
}


