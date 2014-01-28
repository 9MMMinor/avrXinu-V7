
#define OWL_TRACE OWL_TRACE_DATA

#ifdef __avrXinu__
#include <avr-Xinu.h>
#endif

#include <stdlib.h>
#include <string.h>

#ifdef __linux__
#include <fcntl.h>
#include <sys/stat.h>
#endif

#ifdef __avr8__
# define F_CPU 32000000UL
# include <util/delay.h>
#endif

#include <owl/core/owl_shell.h>
#include <owl/core/owl_err.h>
#include <owl/core/owl_debug.h>
#include <owl/drivers/owl_uart.h>

#include "wlp_shell.h"
#include "wlp_api.h"
#include "wlp_inet.h"

/*
 * Evaluate the owl pico API (wlp_api) using the wlp_shell.
 *
 * The functions provided by the wlp_api can be invoked from a command line,
 * using a one-to-one mapping between commands and api function wherever
 * possible.
 *
 *
 *
 *
 */




static void set_baudrate_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void reset_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void linkup_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void linkdown_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void set_mode_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void get_mode_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void set_channel_f(struct owlsh_ctx *sh, int argc, const char *argv[]);

static void get_hwaddr_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void get_network_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void set_ipaddr_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void get_ipaddr_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void set_dhcp_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void get_dhcp_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void set_dhcpd_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void socket_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void bind_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void listen_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void connect_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void close_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void send_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void sendto_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void get_peeraddr_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void get_hostbyname_f(struct owlsh_ctx *sh, int argc,
                             const char *argv[]);
static void get_fw_version_f(struct owlsh_ctx *sh, int argc,
                             const char *argv[]);
static void get_version_f(struct owlsh_ctx *sh, int argc, const char *argv[]);

#ifdef __linux__
static void send_file_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void recv_file_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void recv_f(struct owlsh_ctx *sh, int argc, const char *argv[]);
static void write_fd(int fd, const char *buf, int len);
#endif /* __linux__ */

static int parse_ssid(const char *str, struct wl_ssid_t *ssid);

static int parse_int(const char *str, int *out);
static int parse_uint16(const char *str, uint16_t *out);
#ifdef __avrXinu__
static int parse_int32(const char *str, int32_t *out);
#endif

static void recv_cb(void *ctx, int sockid, int len);
static void lookup_cb(void *ctx, const struct ip_addr *ip);
static void conn_cb(void *ctx, int sockid, int connected);
static void listen_cb(void *ctx, int sockid);

struct owlsh_cmd cmd_reset = {
        "reset", reset_f, "wlp_reset()"
};
struct owlsh_cmd cmd_set_baudrate = {
        "set_baudrate", set_baudrate_f, "wlp_set_baudrate()"
};
struct owlsh_cmd cmd_linkup = {
        "linkup", linkup_f, "wlp_linkup(ssid, key, wep)"
};
struct owlsh_cmd cmd_linkdown = {
        "linkdown", linkdown_f, "wlp_linkdown()"
};
struct owlsh_cmd cmd_set_mode = {
        "set_mode", set_mode_f, "wlp_set_mode()"
};
struct owlsh_cmd cmd_get_mode = {
        "get_mode", get_mode_f, "wlp_get_mode()"
};
struct owlsh_cmd cmd_set_channel = {
        "set_channel", set_channel_f, "wlp_set_channel()"
};
struct owlsh_cmd cmd_get_hwaddr = {
        "get_hwaddr", get_hwaddr_f, "wlp_get_hwaddr()"
};
struct owlsh_cmd cmd_get_network = {
        "get_network", get_network_f, "wlp_get_network()"
};
struct owlsh_cmd cmd_set_ipaddr = {
        "set_ipaddr", set_ipaddr_f, "wlp_set_ipaddr(ip, mask, gw, dns)"
};
struct owlsh_cmd cmd_get_ipaddr = {
        "get_ipaddr", get_ipaddr_f, "wlp_get_ipaddr()"
};
struct owlsh_cmd cmd_set_dhcp = {
        "set_dhcp", set_dhcp_f, "wlp_set_dhcp(enable)"
};
struct owlsh_cmd cmd_get_dhcp = {
        "get_dhcp", get_dhcp_f, "wlp_get_dhcp()"
};
struct owlsh_cmd cmd_set_dhcpd = {
        "set_dhcpd", set_dhcpd_f, "wlp_set_dhcpd()"
};

struct owlsh_cmd cmd_socket = {
        "socket", socket_f, "wlp_socket(type, proto)"
};
struct owlsh_cmd cmd_bind = {
        "bind", bind_f, "wlp_bind(sock, ip, port)"
};
struct owlsh_cmd cmd_listen = {
        "listen", listen_f, "wlp_listen(sockid, backlog)"
};
struct owlsh_cmd cmd_connect = {
        "connect", connect_f, "wlp_connect(sockid, ip, port)"
};
struct owlsh_cmd cmd_close = {
        "close", close_f, "wlp_close(sockid)"
};
struct owlsh_cmd cmd_send = {
        "send", send_f, "wlp_send(sockid, data)"
};
struct owlsh_cmd cmd_sendto = {
        "sendto", sendto_f, "wlp_sendto(sockid, data, ip, port)"
};
struct owlsh_cmd cmd_get_peeraddr = {
        "get_peeraddr", get_peeraddr_f, "wlp_get_peeraddr(sockid)"
};
struct owlsh_cmd cmd_get_hostbyname = {
        "get_hostbyname", get_hostbyname_f, "wlp_get_hostbyname(dnsname)"
};
struct owlsh_cmd cmd_get_fw_version = {
        "get_fw_version", get_fw_version_f, "wlp_get_fw_version(code, build)"
};
struct owlsh_cmd cmd_get_version = {
        "get_version", get_version_f, "print version"
};

#ifdef __linux__
struct owlsh_cmd cmd_send_file = {
        "send_file", send_file_f, "send_file <sockid> <file>"
};
struct owlsh_cmd cmd_recv_file = {
        "recv_file", recv_file_f, "recv_file <sockid> <file>"
};
struct owlsh_cmd cmd_recv = {
        "recv", recv_f, "recv <sockid>"
};

static int recv_fd = -1;
static int recv_sockid = 0;
#endif /* __linux__ */

     
#ifdef __linux__
static void
write_fd(int fd, const char *buf, int len)
{
        int fpos = 0;
        int err;
        while (fpos < len) {
                if ((err = write(fd, buf + fpos, len - fpos)) < 0) {
                        owl_err("write fail:%d", err);
                        return;
                }
                fpos += err;
        }
}
#endif /* __linux__ */

static void
recv_cb(void *ctx, int sockid, int len)
{
        int res;
        struct owlsh_ctx *sh = ctx;
        char buf[64];
        
        owlsh_printf(sh, "recv_cb sockid:%d len:%d %s\n\r", sockid, len,
                     len ? "" : "(connection lost)");

        while (len > 0) {
                int recvlen;
                if (len > sizeof(buf))
                        recvlen = sizeof(buf);
                else
                        recvlen = len;
                
                res = wlp_recv(sockid, buf, recvlen);
                if (res < 0)
                        owl_err("fail to recv %d bytes err:%d\n\r",
                                recvlen, res);
                else if (res != len)
                        owl_info("got %d bytes, %d remains\n\r",
                                 res, len - res);

                if (res > 0) {
                        len -= res;
#ifdef __linux__
                        if (recv_fd > 0)
                                write_fd(recv_fd, buf, res);
                        else
#endif /* __linux__ */
                                owlsh_hexdump(sh, "RX", buf, res);
                }
        }
}

static void
lookup_cb(void *ctx, const struct ip_addr *ip)
{
        struct owlsh_ctx *sh = ctx;
        if (ip == (const struct ip_addr *)NULL)
                owlsh_printf(sh, "lookup_cb: host not found\n\r");
        else
                owlsh_printf(sh, "lookup_cb ip:%s\n\r", inet_ntoa(ip));
}
        
static void
conn_cb(void *ctx, int sockid, int connected)
{
        struct owlsh_ctx *sh = ctx;
        owlsh_printf(sh, "conn_cb sockid:%d connected:%d\n\r",
                     sockid, connected);
}

static void
listen_cb(void *ctx, int sockid)
{
        struct owlsh_ctx *sh = ctx;
        int acceptid;
        owlsh_printf(sh, "listen_cb sockid:%d, accepting ... ", sockid);

        if ((acceptid = wlp_accept(sockid)) > 0) {
                wlp_set_conn_cb(acceptid, conn_cb, sh);
                wlp_set_recv_cb(acceptid, recv_cb, sh);
        }
        
        owlsh_printf(sh, "%s %d (%s)\n\r", acceptid >= 0 ? "ok" : "err",
                     acceptid, owl_error(acceptid));
}

static void
reset_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        res = wlp_reset();
        owlsh_printf(sh, "%s %d (%s)\n\r",
                     res >= 0 ? "ok" : "err", res, owl_error(res));
        
}

static void
set_baudrate_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        int err;
        int32_t baudrate = 0;
        int rtscts = 0;
        extern void *owluart; /* defined in main.c */
        
        if (argc >= 3) {
                /* set_baudrate <baudrate> <rtscts> */
                if (!parse_int32(argv[1], &baudrate))
                        goto err_parse;
                if (!parse_int(argv[2], &rtscts))
                        goto err_parse;
                
        } else if (argc >= 2) {
                /* set_baudrate <baudrate> */
                if (!parse_int32(argv[1], &baudrate))
                        goto err_parse;

        } else {
                goto err_parse;
        }

        res = wlp_set_baudrate(baudrate, rtscts);
        if (!res) {
#if defined(__linux__ ) || defined(__avrXinu__)
                sleep(1);
#elif defined(__avr8__)
                _delay_ms(100);
#endif
                if ((err = owluart_init(owluart, baudrate,
                                        UART_DATABITS_8, UART_PARITY_NONE,
                                        UART_STOPBITS_1, rtscts)) < 0)
                        goto err_uart;
#if defined(__linux__ ) || defined(__avrXinu__)
                sleep(1);
#elif defined(__avr8__)
                _delay_ms(100);
#endif
        }
        
        owlsh_printf(sh, "%s %d (%s)\n\r",
                     res >= 0 ? "ok" : "err", res, owl_error(res));
        return;
        
err_parse:
        owlsh_printf(sh, "usage: set_baudrate <baudrate> [rtscts]\n\r");
        return;

err_uart:
        owlsh_printf(sh, "err %d (failed to configure host uart)\n\r", err);
}

static void
linkup_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        struct wl_ssid_t ssid;
        memset(&ssid, 0, sizeof(ssid));
        if (argc >= 4) {
                /* linkup <ssid> <key> <wep> */
                int wep;

                if (!parse_ssid(argv[1], &ssid))
                        goto err_parse;
                if (!parse_int(argv[3], &wep))
                        goto err_parse;
                res = wlp_linkup(&ssid, argv[2], wep);
        } else if (argc >= 3) {
                /* linkup <ssid> <key> */
                if (!parse_ssid(argv[1], &ssid))
                        goto err_parse;
                res = wlp_linkup(&ssid, argv[2], 0);
        } else if (argc >= 2) {
                /* linkup <ssid> */
                if (!parse_ssid(argv[1], &ssid))
                        goto err_parse;
                
                res = wlp_linkup(&ssid, NULL, 0);
        } else if (argc >= 1) {
                /* linkup */
                res = wlp_linkup((struct wl_ssid_t *)NULL, NULL, 0);
        } else {
                goto err_parse;
        }
                
        owlsh_printf(sh, "%s %d (%s)\n\r",
                     res >= 0 ? "ok" : "err", res, owl_error(res));
        return;

err_parse:
        owlsh_printf(sh, "usage: linkup [ssid] [key] [wep]\n\r");
}

static void
linkdown_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        res = wlp_linkdown();
        owlsh_printf(sh, "%s %d (%s)\n\r",
                     res >= 0 ? "ok" : "err", res, owl_error(res));

}

static void
set_mode_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        uint8_t mode;
        
        if (argc < 2)
                goto err_parse;
        if (!strcmp(argv[1], "sta"))
                mode = WL_MODE_STA;
        else if (!strcmp(argv[1], "ap"))
                mode = WL_MODE_AP;
        else
                goto err_parse;

        res = wlp_set_mode(mode);
        owlsh_printf(sh, "%s %d (%s)\n\r",
                     res >= 0 ? "ok" : "err", res, owl_error(res));
        return;

err_parse:
        owlsh_printf(sh, "usage: set_mode <sta|ap>\n\r");

}


static void
get_mode_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        uint8_t mode;

        res = wlp_get_mode(&mode); /* always success */
        owlsh_printf(sh, "%s %d (%s)\n\r",
                     res >= 0 ? "ok" : "err", res,
                     mode == WL_MODE_STA ? "sta" : "ap");
}


static void
set_channel_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        int channel;
        
        if (argc < 2)
                goto err_parse;
        if (!parse_int(argv[1], &channel))
                goto err_parse;

        res = wlp_set_channel(channel);
        owlsh_printf(sh, "%s %d (%s)\n\r",
                     res >= 0 ? "ok" : "err", res, owl_error(res));
        return;

err_parse:
        owlsh_printf(sh, "usage: set_channel <channel>\n\r");

}

static void
get_hwaddr_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        struct wl_mac_addr_t hwaddr;
        res = wlp_get_hwaddr(&hwaddr);
        
        /* out vars always valid */
        owlsh_printf(sh, "%s %d (%02x:%02x:%02x:%02x:%02x:%02x)\n\r",
                     res >= 0 ? "ok" : "err", res,
                     hwaddr.octet[0], hwaddr.octet[1], hwaddr.octet[2],
                     hwaddr.octet[3], hwaddr.octet[4], hwaddr.octet[5]);
}

static void
get_network_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        struct wlp_network_t net;
        char ssid_str[WL_SSID_MAX_LENGTH + 1];
        res = wlp_get_network(&net);
                
        memcpy(ssid_str, net.ssid.ssid, net.ssid.len);
        ssid_str[net.ssid.len] = 0;
                
        /* out vars always valid */
        owlsh_printf(sh, "%s %d (%s %02x:%02x:%02x:%02x:%02x:%02x)\n\r",
                     res >= 0 ? "ok" : "err", res,
                     net.ssid.len ? ssid_str : "N/A",
                     net.bssid.octet[0], net.bssid.octet[1],
                     net.bssid.octet[2], net.bssid.octet[3],
                     net.bssid.octet[4], net.bssid.octet[5]);
}

static void
set_ipaddr_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        struct ip_addr ip, mask, gw, dns;

        mask = ip_addr_any;
        gw = ip_addr_any;
        dns = ip_addr_any;

        if (argc < 2)
                goto err_parse;
        if (!inet_aton(argv[1], &ip))
                goto err_parse;
        if (argc >= 3 && !inet_aton(argv[2], &mask))
                goto err_parse;
        if (argc >= 4 && !inet_aton(argv[3], &gw))
                goto err_parse;
        if (argc >= 5 && !inet_aton(argv[4], &dns))
                goto err_parse;

        res = wlp_set_ipaddr(&ip, &mask, &gw, &dns);
        owlsh_printf(sh, "%s %d (%s)\n\r",
                     res >= 0 ? "ok" : "err", res, owl_error(res));
        return;
        
err_parse:
        owlsh_printf(sh, "usage: set_ipaddr <ip> [mask] [gw] [dns]\n\r");
}

static void
get_ipaddr_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        struct ip_addr ip, mask, gw, dns;
                
        res = wlp_get_ipaddr(&ip, &mask, &gw, &dns);
        owlsh_printf(sh, "%s %d (", res >= 0 ? "ok" : "err", res);
        owlsh_printf(sh, "ip:%s ", inet_ntoa(&ip));
        owlsh_printf(sh, "mask:%s ", inet_ntoa(&mask));
        owlsh_printf(sh, "gw:%s ", inet_ntoa(&gw));
        owlsh_printf(sh, "dns:%s)\n\r", inet_ntoa(&dns));
}

static void
set_dhcp_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        int dhcp;

        if (argc < 2)
                goto err_parse;
        if (!parse_int(argv[1], &dhcp))
                goto err_parse;

        res = wlp_set_dhcp(dhcp);
        owlsh_printf(sh, "%s %d (%s)\n\r",
                     res >= 0 ? "ok" : "err", res, owl_error(res));
        return;
        
err_parse:
        owlsh_printf(sh, "usage: set_dhcp <0|1>\n\r");
}

static void
get_dhcp_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        int enabled;
                
        res = wlp_get_dhcp(&enabled);
        owlsh_printf(sh, "%s %d (enabled:%d)\n\r",
                     res >= 0 ? "ok" : "err", res, enabled);
}

static void
set_dhcpd_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        int enable;

        if (argc < 2)
                goto err_parse;
        if (!parse_int(argv[1], &enable))
                goto err_parse;

        res = wlp_set_dhcpd(enable);
        owlsh_printf(sh, "%s %d (%s)\n\r",
                     res >= 0 ? "ok" : "err", res, owl_error(res));
        return;
        
err_parse:
        owlsh_printf(sh, "usage: set_dhcpd <0|1>\n\r");
}

static void
socket_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int sockid;
        int type;
        int protocol = 0;
                
        if (argc < 2)
                goto err_parse;

        if (!strcmp(argv[1], "stream"))
                type = WLP_SOCK_STREAM;
        else if (!strcmp(argv[1], "dgram"))
                type = WLP_SOCK_DGRAM;
        else if (!strcmp(argv[1], "raw"))
                type = WLP_SOCK_RAW;
        else
                goto err_parse;
        
        if (argc > 2 && !parse_int(argv[2], &protocol))
                goto err_parse;

        sockid = wlp_socket(type, protocol);
        wlp_set_conn_cb(sockid, conn_cb, sh);
        wlp_set_recv_cb(sockid, recv_cb, sh);

        owlsh_printf(sh, "%s %d (%s)\n\r", sockid >= 0 ? "ok" : "err", sockid,
                     owl_error(sockid));
        return;
        
err_parse:
        owlsh_printf(sh, "usage: socket <stream|dgram|raw> [protocol]\n\r");
        
}
static void
bind_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        int sockid;
        struct ip_addr ip;
        uint16_t port;
        
        if (argc < 4)
                goto err_parse;
        if (!parse_int(argv[1], &sockid))
                goto err_parse;
        if (!inet_aton(argv[2], &ip))
                goto err_parse;
        if (!parse_uint16(argv[3], &port))
                goto err_parse;

        res = wlp_bind(sockid, &ip, port);

        owlsh_printf(sh, "%s %d (%s)\n\r", res >= 0 ? "ok" : "err", res,
                     owl_error(res));
        return;
        
err_parse:
        owlsh_printf(sh, "usage: bind <sockid> <ip> <port>\n\r");
        
}

static void
listen_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        int sockid;
        int backlog = 1;
        
        if (argc < 2)
                goto err_parse;
        if (!parse_int(argv[1], &sockid))
                goto err_parse;
        if (argc > 2 && !parse_int(argv[2], &backlog))
                goto err_parse;

        
        res = wlp_listen(sockid, backlog, listen_cb, sh);

        owlsh_printf(sh, "%s %d (%s)\n\r", res >= 0 ? "ok" : "err", res,
                     owl_error(res));
        return;
        
err_parse:
        owlsh_printf(sh, "usage: listen <sockid> [backlog]\n\r");
        
}

static void
connect_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        int sockid;
        struct ip_addr ip;
        uint16_t port;
        
        if (argc < 4)
                goto err_parse;
        if (!parse_int(argv[1], &sockid))
                goto err_parse;
        if (!inet_aton(argv[2], &ip))
                goto err_parse;
        if (!parse_uint16(argv[3], &port))
                goto err_parse;

        res = wlp_connect(sockid, &ip, port);

        owlsh_printf(sh, "%s %d (%s)\n\r", res >= 0 ? "ok" : "err", res,
                     owl_error(res));
        return;
        
err_parse:
        owlsh_printf(sh, "usage: connect <sockid> <ip> <port>\n\r");
}

static void
close_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        int sockid;
        
        if (argc < 2)
                goto err_parse;
        if (!parse_int(argv[1], &sockid))
                goto err_parse;
                
        res = wlp_close(sockid);

        owlsh_printf(sh, "%s %d (%s)\n\r", res >= 0 ? "ok" : "err", res,
                     owl_error(res));
        return;
        
err_parse:
        owlsh_printf(sh, "usage: close <sockid>\n\r");
}

static void
send_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        int sockid;
        char *data;
        int len;
        int pos = 0;
                
        if (argc < 3)
                goto err_parse;

        if (!parse_int(argv[1], &sockid))
                goto err_parse;
        
        if ((data = owlsh_joinargs(argc - 2, argv + 2)) == NULL) {
                res = OWL_ERR_MEM;
                goto out;
        }

        len = strlen(data);
        owlsh_printf(sh, "sending %d bytes\n\r", len);
        while (pos < len) {

                if ((res = wlp_send(sockid, data + pos, len - pos)) < 0) {
                        free(data);
                        goto out;
                                
                } else {
                        owlsh_printf(sh, "wrote:%d remain:%d\n\r",
                                     res, len - pos - res);
                }
                pos += res;
        }
        free(data);
        res = 0;
        
out:        
        owlsh_printf(sh, "%s %d (%s)\n\r", res >= 0 ? "ok" : "err", res,
                     owl_error(res));
        return;
        
err_parse:
        owlsh_printf(sh, "usage: send <sockid> <data ...>\n\r");
}

static void
sendto_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        int sockid;
        struct ip_addr ip;
        uint16_t port;
        char *data;
        int len;
        int pos = 0;
                
        if (argc < 4)
                goto err_parse;

        if (!parse_int(argv[1], &sockid))
                goto err_parse;

        if (!inet_aton(argv[2], &ip))
                goto err_parse;

        if (!parse_uint16(argv[3], &port))
                goto err_parse;
        
        if ((data = owlsh_joinargs(argc - 4, argv + 4)) == NULL) {
                res = OWL_ERR_MEM;
                goto out;
        }

        len = strlen(data);
        owlsh_printf(sh, "sending %d bytes to %s:%d\n\r", len,
                     inet_ntoa(&ip), port);
        
        while (pos < len) {

                if ((res = wlp_sendto(sockid, data + pos, len - pos,
                                      &ip, port)) < 0) {
                        free(data);
                        goto out;
                                
                } else {
                        owlsh_printf(sh, "wrote:%d remain:%d\n\r",
                                     res, len - pos - res);
                }
                pos += res;
        }
        free(data);
        res = 0;
        
out:        
        owlsh_printf(sh, "%s %d (%s)\n\r", res >= 0 ? "ok" : "err", res,
                     owl_error(res));
        return;
        
err_parse:
        owlsh_printf(sh, "usage: sendto <sockid> <ip> <port> <data ...>\n\r");
}

static void
get_peeraddr_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        int sockid;
        struct ip_addr peer;

        if (argc < 2)
                goto err_parse;
        if (!parse_int(argv[1], &sockid))
                goto err_parse;
        
        res = wlp_get_peeraddr(sockid, &peer);
        owlsh_printf(sh, "%s %d (%s) ", res >= 0 ? "ok" : "err", res,
                     owl_error(res));
        owlsh_printf(sh, "peer:%s\n\r", inet_ntoa(&peer));
        return;
        
err_parse:
        owlsh_printf(sh, "usage: get_peeraddr <sockid>\n\r");
}

static void
get_hostbyname_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;

        if (argc < 2)
                goto err_parse;
        
        res = wlp_get_hostbyname(argv[1], lookup_cb, sh);
        owlsh_printf(sh, "%s %d (%s)\n\r", res >= 0 ? "ok" : "err", res,
                     owl_error(res));
        return;
        
err_parse:
        owlsh_printf(sh, "usage: get_hostbyname <dnsname>\n\r");
}

static void
get_fw_version_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int res;
        uint32_t code;
        char build[16];
        
        res = wlp_get_fw_version(&code, build, sizeof(build));
        owlsh_printf(sh, "%s %d (%s)\n\r", res >= 0 ? "ok" : "err", res,
                     owl_error(res));
        owlsh_printf(sh, "version:%d.%d.%d build:%s\n\r",
                     WLP_MAJOR_VERSION(code), WLP_MINOR_VERSION(code),
                     WLP_PATCH_VERSION(code), build);
}

static void
get_version_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        owlsh_printf(sh, "version:%d.%d.%d\n\r",
                     WLP_MAJOR_VERSION(WLP_VERSION_CODE),
                     WLP_MINOR_VERSION(WLP_VERSION_CODE),
                     WLP_PATCH_VERSION(WLP_VERSION_CODE));
}

#ifdef __linux__
static void
send_file_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        int sockid;
        const char *name;
        char buf[256];
        int read_err = 0;
        int send_err = 0;
        int len = 0;
        int fd;
        struct stat stat_buf;
        int file_len;
        int read_len = 0;
        
        if (argc < 3)
                goto err_parse;
        if (!parse_int(argv[1], &sockid))
                goto err_parse;
        name = argv[2];

        if ((fd = open(name, O_RDONLY)) < 0) {
                owlsh_printf(sh, "could not open '%s' fd:%d\n\r", name, fd);
                return;
        }

        if (stat(name, &stat_buf) < 0) {
                close(fd);
                owlsh_printf(sh, "could not stat %s\n\r", name);
                return;
        }

        file_len = stat_buf.st_size;
        owlsh_printf(sh, "sending %d bytes\n\r", file_len);
        
        while (read_len < file_len) {
                if ((read_err = read(fd, buf, sizeof(buf))) < 0)
                        goto out;

                read_len += read_err;
                send_err = 0;
                int buflen = read_err;
                int bufpos = 0;
                while (bufpos < buflen) {
                        if ((send_err = wlp_send(sockid, buf + bufpos,
                                                 buflen - bufpos)) < 0)
                                goto out;
                        len += send_err;
                        bufpos += send_err;
                        owlsh_printf(sh, "wlp_send: %d\n\r", send_err);
                }
        }

out:
        owlsh_printf(sh, "done -- read_err:%d send_err:%d len:%d\n\r",
                     read_err, send_err, len);
        close(fd);
        return;
err_parse:
        owlsh_printf(sh, "usage: send_file <sockid> <file>\n\r");
}

static void
recv_file_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        const char *name;
        int sockid;
        
        if (argc < 3)
                goto err_parse;
        if (!parse_int(argv[1], &sockid))
                goto err_parse;
        name = argv[2];

        if (recv_fd >= 0) {
                owlsh_printf(sh, "closing %d\n\r", recv_fd);
                close(recv_fd);
        }

        if ((recv_fd = open(name, O_CREAT | O_RDWR | O_TRUNC, 0660)) < 0) {
                owlsh_printf(sh, "failed to open %s\n\r", name);
                return;
        }

        recv_sockid = sockid;
        owlsh_printf(sh, "rx data for sockid %d will be written to %s\n\r",
                     sockid, name);
        return;

        
err_parse:
        owlsh_printf(sh, "usage: recv_file <sockid> <file>\n\r");
}


static void
recv_f(struct owlsh_ctx *sh, int argc, const char *argv[])
{
        if (recv_fd < 0)
                return;

        owlsh_printf(sh, "closing %d\n\r", recv_fd);
        recv_sockid = 0;
        close(recv_fd);
        recv_fd = -1;
}
#endif

static int
parse_uint16(const char *str, uint16_t *out)
{
        char *end;
#ifdef __avrXinu__
	int32_t val;
#else
        int val;
#endif

        val = strtol(str, &end, 0);
        if (end == str)
                return 0;

        if (val > 65535)
                return 0;
                
        if (val < 0)
                return 0;
        
        *out = val;
        return 1;
}

#ifdef __avrXinu__
static int
parse_int32(const char *str, int32_t *out)
{
	char *end;
	
	*out = strtol(str, &end, 0);
	if (end == str)
		return 0;
	
	return 1;
}
#endif

static int
parse_int(const char *str, int *out)
{
        char *end;
	
        *out = strtol(str, &end, 0);
        if (end == str)
                return 0;
        
        return 1;
}

static int
parse_ssid(const char *str, struct wl_ssid_t *ssid)
{
        ssid->len = strlen(str);
        memcpy(ssid->ssid, str, ssid->len);
        return 1;
}
