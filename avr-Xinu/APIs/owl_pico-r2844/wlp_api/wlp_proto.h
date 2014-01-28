#ifndef WLP_PROTO_H
#define WLP_PROTO_H

#include <stdint.h>

#define WLP_UART_CONFIG 0
#define WLP_POLL        1
#define WLP_FW_VERSION  2
#define WLP_SET_MODE    22
#define WLP_SET_CHANNEL 23
#define WLP_RESET       24
#define WLP_GET_MODE    26

#define WLP_LINK_UP     3
#define WLP_LINK_DOWN   4
#define WLP_LINK_STATUS 5
#define WLP_LINK_INFO   6

#define WLP_NET_STATUS  7
#define WLP_NET_CONFIG  8
#define WLP_NET_INFO    9
#define WLP_NET_DNSLOOKUP    10
#define WLP_NET_DHCPD   25

#define WLP_SOCK_OPEN   11
#define WLP_SOCK_BIND   12
#define WLP_SOCK_LISTEN 13 
#define WLP_SOCK_CONNECT 14

#define WLP_SOCK_CLOSE  15
#define WLP_SOCK_RECV   16
#define WLP_SOCK_SEND   17
#define WLP_SOCK_SENDTO   18
#define WLP_SOCK_STATUS 19
#define WLP_SOCK_ACCEPT 20
#define WLP_SOCK_INFO   21

#ifndef WL_API_H
#define WL_MAX_PASS_LEN 64
#define WL_SSID_MAX_LENGTH 32
#define WL_MAC_ADDR_LENGTH 6

struct wl_ssid_t {
        char ssid[WL_SSID_MAX_LENGTH];
        uint8_t len;
};

struct wl_mac_addr_t {
        uint8_t octet[WL_MAC_ADDR_LENGTH];
};

#define WL_MODE_STA 0
#define WL_MODE_AP  1

#endif /* WL_API_H */

#ifndef __LWIP_IP_ADDR_H__

/*
 * The struct ip_addr type is based on work from the lwIP network stack.
 *
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

struct ip_addr {
        uint32_t addr;
};

extern const struct ip_addr ip_addr_any;

#define IP_ADDR_ANY ((struct ip_addr *) &ip_addr_any)
#define IP4_ADDR(ipaddr, a,b,c,d)                               \
        (ipaddr)->addr = htonl(((uint32_t)((a) & 0xff) << 24) | \
                               ((uint32_t)((b) & 0xff) << 16) | \
                               ((uint32_t)((c) & 0xff) << 8) |  \
                               (uint32_t)((d) & 0xff))
#define ip_addr_set(dest, src) \
        (dest)->addr = ((src) == NULL? 0: (src)->addr)
#define ip_addr_netcmp(addr1, addr2, mask) \
        (((addr1)->addr & (mask)->addr) == ((addr2)->addr & (mask)->addr))
#define ip_addr_cmp(addr1, addr2) \
        ((addr1)->addr == (addr2)->addr)
#define ip_addr_isany(addr1) \
        ((addr1) == NULL || (addr1)->addr == 0)

#define ip4_addr1(ipaddr) ((uint16_t)(ntohl((ipaddr)->addr) >> 24) & 0xff)
#define ip4_addr2(ipaddr) ((uint16_t)(ntohl((ipaddr)->addr) >> 16) & 0xff)
#define ip4_addr3(ipaddr) ((uint16_t)(ntohl((ipaddr)->addr) >> 8) & 0xff)
#define ip4_addr4(ipaddr) ((uint16_t)(ntohl((ipaddr)->addr)) & 0xff)

#endif /* __LWIP_IP_ADDR_H__ */

#define WLP_VERSION(a,b,c) htonl(((uint32_t)((a) & 0xff) << 16) |       \
                                 ((uint32_t)((b) & 0xff) << 8) |        \
                                 (uint32_t)((c) & 0xff))
#define WLP_MAJOR_VERSION(code) ((uint16_t)(ntohl(code) >> 16) & 0xff)
#define WLP_MINOR_VERSION(code) ((uint16_t)(ntohl(code) >> 8) & 0xff)
#define WLP_PATCH_VERSION(code) ((uint16_t)(ntohl(code)) & 0xff)

struct wlp_uart_config_req {
        uint8_t id;
        char pad[2];
        uint8_t rtscts;
        uint32_t baudrate;
}; /* sizeof: 8 */

struct wlp_poll_req {
        uint8_t id;
}; /* sizeof: 1 */

struct wlp_fw_version_req {
        uint8_t id;
}; /* sizeof: 1 */

struct wlp_set_mode_req {
        uint8_t id;
        uint8_t mode;
}; /* sizeof: 2 */

struct wlp_get_mode_req {
        uint8_t id;
}; /* sizeof: 1 */

struct wlp_set_channel_req {
        uint8_t id;
        uint8_t channel;
}; /* sizeof: 2 */

struct wlp_reset_req {
        uint8_t id;
}; /* sizeof: 1 */

struct wlp_link_up_req {
        uint8_t id;
        char pad[3];
        struct wl_ssid_t ssid;
        char key[WL_MAX_PASS_LEN];
        uint8_t wep;
}; /* sizeof: 102 */

struct wlp_link_down_req {
        uint8_t id;                        
}; /* sizeof: 1 */

struct wlp_link_status_req {
        uint8_t id;
}; /* sizeof: 1 */

struct wlp_link_info_req {
        uint8_t id;
}; /* sizeof: 1 */

struct wlp_net_status_req {
        uint8_t id;
}; /* sizeof: 1 */

struct wlp_net_config_req {
        uint8_t id;
        char pad[2];
        int8_t dhcp;
        struct ip_addr ip;
        struct ip_addr netmask;
        struct ip_addr gw;
        struct ip_addr dns_server;
}; /* sizeof: 20 */

struct wlp_net_info_req {
        uint8_t id;
}; /* sizeof: 1 */

struct wlp_net_dnslookup_req {
        uint8_t id;
#define WLP_HOSTNAME_MAX_LENGTH 127
        char host[WLP_HOSTNAME_MAX_LENGTH + 1];
}; /* sizeof: 129 */

struct wlp_net_dhcpd_req {
        uint8_t id;
        uint8_t enable;
}; /* sizeof: 2 */


struct wlp_sock_open_req {
        uint8_t id;
#define WLP_SOCK_STREAM 0
#define WLP_SOCK_DGRAM  1
#define WLP_SOCK_RAW    2
        uint8_t type;
        uint8_t protocol;
}; /* sizeof: 3 */

struct wlp_sock_bind_req {
        uint8_t id;
        char pad[1];
        int16_t sockid;
        struct ip_addr ip;
        uint16_t port;
}; /* sizeof: 10 */

struct wlp_sock_listen_req {
        uint8_t id;
        int8_t backlog;
        int16_t sockid;
}; /* sizeof: 4 */

struct wlp_sock_connect_req {
        uint8_t id;
        char pad[1];
        int16_t sockid;
        struct ip_addr ip;
        uint16_t port;
}; /* sizeof: 10 */

struct wlp_sock_close_req {
        uint8_t id;
        char pad[1];
        int16_t sockid;
}; /* sizeof: 4 */

struct wlp_sock_recv_req {
        uint8_t id;
        char pad[1];
        int16_t sockid;
        int16_t len;
}; /* sizeof: 6 */

struct wlp_sock_send_req {
        uint8_t id;
        char pad[1];
        int16_t sockid;
        int16_t len;
}; /* sizeof: 6 */

struct wlp_sock_sendto_req {
        uint8_t id;
        char pad[1];
        int16_t sockid;
        int16_t len;
        uint16_t port;
        struct ip_addr addr;
}; /* sizeof: 12 */

struct wlp_sock_status_req {
        uint8_t id;
        char pad[1];
        int16_t sockid;
}; /* sizeof: 4 */

struct wlp_sock_accept_req {
        uint8_t id;
        char pad[1];
        int16_t sockid;
}; /* sizeof: 4 */

struct wlp_sock_info_req {
        uint8_t id;
        char pad[1];
        int16_t sockid;
}; /* sizeof: 4 */

struct wlp_req {

        union {
                uint8_t id;

                struct wlp_uart_config_req uart_config;
                struct wlp_poll_req poll;
                struct wlp_fw_version_req fw_version;
                struct wlp_set_mode_req set_mode;
                struct wlp_get_mode_req get_mode;
                struct wlp_set_channel_req set_channel;
                struct wlp_reset_req reset;
                
                struct wlp_link_up_req link_up;
                struct wlp_link_down_req link_down;
                struct wlp_link_status_req link_status;
                struct wlp_link_info_req link_info;

                struct wlp_net_status_req net_status;
                struct wlp_net_config_req net_config;
                struct wlp_net_info_req net_info;
                struct wlp_net_dnslookup_req net_dnslookup;
                struct wlp_net_dhcpd_req net_dhcpd;

                struct wlp_sock_open_req sock_open;
                struct wlp_sock_bind_req sock_bind;
                struct wlp_sock_listen_req sock_listen;
                struct wlp_sock_connect_req sock_connect;
                struct wlp_sock_close_req sock_close;
                struct wlp_sock_recv_req sock_recv;
                struct wlp_sock_send_req sock_send;
                struct wlp_sock_sendto_req sock_sendto;
                struct wlp_sock_status_req sock_status;
                struct wlp_sock_accept_req sock_accept;
                struct wlp_sock_info_req sock_info;
        };
};

struct wlp_uart_config_cfm {
        int16_t res;
        char pad;
        int8_t mode; /* operation mode */
}; /* sizeof: 2 */

struct wlp_poll_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_fw_version_cfm {
        int16_t res;
        char pad[2];
        uint32_t code; /* WLP_VERSION(a, b, c) */
        char build[16];
}; /* sizeof: 24 */

struct wlp_set_mode_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_get_mode_cfm {
        int16_t res;
        char pad[1];
        uint8_t mode;
}; /* sizeof: 4 */

struct wlp_set_channel_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_reset_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_link_up_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_link_down_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_link_status_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_link_info_cfm {
        int16_t res;
        char pad1[2];
        struct wl_mac_addr_t hwaddr;
        struct wl_mac_addr_t ap;
        struct wl_ssid_t ssid;
        char pad2[1];
}; /* sizeof: 50 */

struct wlp_net_status_cfm {
        int16_t res;
}; /* sizeof: 2 */
 
struct wlp_net_config_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_net_info_cfm {
        int16_t res;
#define WLP_DNS_OK       0
#define WLP_DNS_ERROR   -1
#define WLP_DNS_AGAIN   -2
#define WLP_DNS_INVALID -3
        int8_t dns_result;
        int8_t dhcp;
        struct ip_addr ip;
        struct ip_addr netmask;
        struct ip_addr gw;
        struct ip_addr dns_server;
        struct ip_addr dns_lookup;
}; /* sizeof: 24 */

struct wlp_net_dnslookup_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_net_dhcpd_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_sock_open_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_sock_bind_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_sock_listen_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_sock_connect_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_sock_close_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_sock_recv_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_sock_send_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_sock_sendto_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_sock_status_cfm {
        int16_t res;  /* avail bytes */
        char pad[1];
        
#define WLP_SOCK_STATE_DISCONNECTED 0 /* default */
#define WLP_SOCK_STATE_CONNECTED 1
#define WLP_SOCK_STATE_LISTEN 2
#define WLP_SOCK_STATE_ACCEPT 3
#define WLP_SOCK_STATE_ERROR 4
        int8_t state;
}; /* sizeof: 4 */

struct wlp_sock_accept_cfm {
        int16_t res;
}; /* sizeof: 2 */

struct wlp_sock_info_cfm {
        int16_t res;
        char pad[2];
        struct ip_addr peer;
}; /* sizeof: 8 */

struct wlp_cfm {
        union {
                int16_t res;

                struct wlp_uart_config_cfm uart_config;
                struct wlp_poll_cfm poll;
                struct wlp_fw_version_cfm fw_version;
                struct wlp_set_mode_cfm set_mode;
                struct wlp_get_mode_cfm get_mode;
                struct wlp_set_channel_cfm set_channel;
                struct wlp_reset_cfm reset;
                
                struct wlp_link_up_cfm link_up;
                struct wlp_link_down_cfm link_down;
                struct wlp_link_status_cfm link_status;
                struct wlp_link_info_cfm link_info;
                
                struct wlp_net_status_cfm net_status;
                struct wlp_net_config_cfm net_config;
                struct wlp_net_info_cfm net_info;
                struct wlp_net_dnslookup_cfm net_dnslookup;
                struct wlp_net_dhcpd_cfm net_dhcpd;

                struct wlp_sock_open_cfm sock_open;
                struct wlp_sock_bind_cfm sock_bind;
                struct wlp_sock_listen_cfm sock_listen;
                struct wlp_sock_connect_cfm sock_connect;
                struct wlp_sock_close_cfm sock_close;
                struct wlp_sock_recv_cfm sock_recv;
                struct wlp_sock_send_cfm sock_send;
                struct wlp_sock_sendto_cfm sock_sendto;
                struct wlp_sock_status_cfm sock_status;
                struct wlp_sock_accept_cfm sock_accept;
                struct wlp_sock_info_cfm sock_info;
        };

};

#endif /* PICO_H */
