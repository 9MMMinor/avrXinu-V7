#ifndef WLP_SHELL_H
#define WLP_SHELL_H

#include <owl/core/owl_shell.h>

extern struct owlsh_cmd cmd_reset;
extern struct owlsh_cmd cmd_set_baudrate;
extern struct owlsh_cmd cmd_get_fw_version;
extern struct owlsh_cmd cmd_get_version;
extern struct owlsh_cmd cmd_set_mode;
extern struct owlsh_cmd cmd_get_mode;
extern struct owlsh_cmd cmd_set_channel;

extern struct owlsh_cmd cmd_linkup;
extern struct owlsh_cmd cmd_linkdown;
extern struct owlsh_cmd cmd_get_hwaddr;
extern struct owlsh_cmd cmd_get_network;
extern struct owlsh_cmd cmd_set_ipaddr;
extern struct owlsh_cmd cmd_get_ipaddr;
extern struct owlsh_cmd cmd_set_dhcp;
extern struct owlsh_cmd cmd_get_dhcp;
extern struct owlsh_cmd cmd_set_dhcpd;

extern struct owlsh_cmd cmd_socket;
extern struct owlsh_cmd cmd_bind;
extern struct owlsh_cmd cmd_listen;
extern struct owlsh_cmd cmd_connect;
extern struct owlsh_cmd cmd_close;
extern struct owlsh_cmd cmd_send;
extern struct owlsh_cmd cmd_sendto;
extern struct owlsh_cmd cmd_get_peeraddr;
extern struct owlsh_cmd cmd_get_hostbyname;

extern struct owlsh_cmd cmd_send_file;
extern struct owlsh_cmd cmd_recv_file;
extern struct owlsh_cmd cmd_recv;

#endif
