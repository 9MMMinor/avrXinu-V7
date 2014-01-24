
/*
 * Copyright (c) 2006-2008 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/**
 * The events the DHCP client may fire.
 */
enum dhcp_client_event
{
    /** The network has been successfully configured. */
    DHCP_CLIENT_EVT_LEASE_ACQUIRED = 0,
    /** The DHCP server has denied a configuration. */
    DHCP_CLIENT_EVT_LEASE_DENIED,
    /** The network configuration is about to expire, a new configuration cycle is being started. */
    DHCP_CLIENT_EVT_LEASE_EXPIRING,
    /** The network configuration has expired and could not be renewed. The network interface should be deconfigured. */
    DHCP_CLIENT_EVT_LEASE_EXPIRED,
    /** No DHCP server responded to our requests. */
    DHCP_CLIENT_EVT_TIMEOUT,
    /** An unknown error occured. */
    DHCP_CLIENT_EVT_ERROR
};

struct dhcp_header
{
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint8_t ciaddr[4];
    uint8_t yiaddr[4];
    uint8_t siaddr[4];
    uint8_t giaddr[4];
    uint8_t chaddr[16];
    char sname[64];
    char file[128];
    uint32_t cookie;
};

enum dhcp_state
{
    DHCP_STATE_INIT = 0,
    DHCP_STATE_SELECTING,
    DHCP_STATE_REQUESTING,
    DHCP_STATE_BOUND,
    DHCP_STATE_REBINDING
};

struct dhcp_client_state
{
    enum dhcp_state state;
    int device;
    int timerPid;
    uint8_t ip_server[4];
    uint8_t ip_local[4];
    uint8_t ip_netmask[4];
    uint8_t ip_gateway[4];
    uint32_t time_rebind;
    uint8_t retries;
	int dhcpsem;			/* mutual exclusion semaphore */
	int dhcplen;
	int dhcpPid;			/* process sending DHCPREQUEST */
	int startupPid;			/* process which starts DHCP */
	int dhcpPacketId;		/* sequential id */
};

#define DHCP_TMSTK 200			/* dhcp timer stack size */
#define DHCP_TMPRI 100			/* dhcp timer priority */
#define DHCP_TMNAM "dhpc_timer"	/* dhcp timer name */
#define DHCP_CLIENT_TIMEOUT 40	/* TICKS (usually 1/10ths of a second) */
#define DHCP_CLIENT_RETRIES 3
#define DHCP_CLIENT_XID 0x55059934
#define DHCP_CLIENT_PORT 68
#define DHCP_SERVER_PORT 67

#define DHCP_OP_BOOTREQUEST 0x01
#define DHCP_OP_BOOTREPLY 0x02
#define DHCP_HTYPE_ETHERNET 0x01
#define DHCP_HLEN_ETHERNET 6
#define DHCP_FLAG_BROADCAST 0x8000
#define DHCP_COOKIE 0x63825363

#define DHCP_OPTION_PAD 0x00
#define DHCP_OPTION_END 0xff
#define DHCP_OPTION_NETMASK 0x01
#define DHCP_OPTION_ROUTER 0x03
#define DHCP_OPTION_TIMESERVER 0x04
#define DHCP_OPTION_DNS 0x06
#define DHCP_OPTION_REQUESTIP 0x32
#define DHCP_OPTION_TIMELEASE 0x33
#define DHCP_OPTION_MESSAGETYPE 0x35
#define DHCP_OPTION_SERVERID 0x36
#define DHCP_OPTION_PARAMREQUEST 0x37
#define DHCP_OPTION_TIMERENEW 0x3a
#define DHCP_OPTION_TIMEREBIND 0x3b

#define DHCP_OPTION_LEN_NETMASK 4
#define DHCP_OPTION_LEN_REQUESTIP 4
#define DHCP_OPTION_LEN_TIMELEASE 4
#define DHCP_OPTION_LEN_MESSAGETYPE 1
#define DHCP_OPTION_LEN_SERVERID 4
#define DHCP_OPTION_LEN_TIMERENEW 4
#define DHCP_OPTION_LEN_TIMEREBIND 4

#define DHCP_MESSAGETYPE_DHCPDISCOVER 1
#define DHCP_MESSAGETYPE_DHCPOFFER 2
#define DHCP_MESSAGETYPE_DHCPREQUEST 3
#define DHCP_MESSAGETYPE_DHCPDECLINE 4
#define DHCP_MESSAGETYPE_DHCPACK 5
#define DHCP_MESSAGETYPE_DHCPNACK 6
#define DHCP_MESSAGETYPE_DHCPRELEASE 7

int dhcp_client_start(void);
void dhcp_client_abort();
