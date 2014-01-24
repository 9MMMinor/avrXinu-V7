/* dhcp_client.c	*/

/*
 * Copyright (c) 2006-2008 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * Modified for avr-Xinu
 *
 * Michael Minor, October 19, 2010.
 *				Oct 24, 2011.
 *
 */

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *		Needs timeout response work; client_abort fixes
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

#include <avr-Xinu.h>
#include <network.h>

#include <string.h>
#include <avr/pgmspace.h>

// seconds to TIMEOUT from dgread (typically 5 seconds)
#define DG_DELAY_TIME DG_TIME/TICK

/*
 * A DHCP client for automatically retrieving the network configuration when the link goes up.
 *
 * The client provides some events through a callback function. These events inform the application
 * whenever e.g. the network has been successfully configured or the DHCP lease has expired.
 *
 */

/*
 * file
 * DHCP client implementation (license: GPLv2)
 *
 * author Roland Riegel
 */

static uint8_t* dhcp_client_packet_add_msgtype(uint8_t* options, uint8_t type);
static uint8_t* dhcp_client_packet_add_params(uint8_t* options);
static uint8_t* dhcp_client_packet_add_server(uint8_t* options);
static uint8_t* dhcp_client_packet_add_ip(uint8_t* options);
static uint8_t* dhcp_client_packet_add_end(uint8_t* options);

Bool dhcp_client_send(struct dhcp_header *packet, uint8_t msg_type);
static uint8_t dhcp_client_parse(const struct dhcp_header* packet);
static void dhcp_client_event_report(enum dhcp_client_event event);
static void dhcp_Panic(void);
void packet_Dump(char * routine, void *epkptr, int len);
PROCESS dhcp_Client();
static int dhcpSleep(int);
uint8_t dhcp_client_in(struct dhcp_header* header);

int dgdump(void);
void panic(char *msg);
int netnum(IPaddr netpart, IPaddr address);

struct dhcp_client_state state;

/*
 *---------------------------------------------------------------------------------------------
 *						RFC 2131
 *
 *
 *
 *
 *
 *
 *	 --------                               -------
 *	|        | +-------------------------->|       |<-------------------+
 *	| INIT-  | |     +-------------------->| INIT  |                    |
 *	| REBOOT |DHCPNAK/         +---------->|       |<---+               |
 *	|        |Restart|         |            -------     |               |
 *	 --------  |  DHCPNAK/     |               |                        |
 *      |      Discard offer   |      -/Send DHCPDISCOVER               |
 *	-/Send DHCPREQUEST         |               |                        |
 *      |      |     |      DHCPACK            v        |               |
 *	 -----------     |   (not accept.)/   -----------   |               |
 *	|           |    |  Send DHCPDECLINE |           |                  |
 *	| REBOOTING |    |         |         | SELECTING |<----+            |
 *	|           |    |        /          |           |     |DHCPOFFER/  |
 *	 -----------     |       /            -----------   |  |Collect     |
 *      |            |      /                  |   |       |  replies   |
 *	DHCPACK/         |     /  +----------------+   +-------+            |
 *	Record lease, set|    |   v   Select offer/                         |
 *	timers T1, T2   ------------  send DHCPREQUEST      |               |
 *      |   +----->|            |             DHCPNAK, Lease expired/   |
 *      |   |      | REQUESTING |                  Halt network         |
 *      DHCPOFFER/ |            |                       |               |
 *      Discard     ------------                        |               |
 *      |   |        |        |                   -----------           |
 *      |   +--------+     DHCPACK/              |           |          |
 *      |              Record lease, set    -----| REBINDING |          |
 *      |                timers T1, T2     /     |           |          |
 *      |                     |        DHCPACK/   -----------           |
 *      |                     v     Record lease, set   ^               |
 *      +----------------> -------      /timers T1,T2   |               |
 *                 +----->|       |<---+                |               |
 *                 |      | BOUND |<---+                |               |
 *    DHCPOFFER, DHCPACK, |       |    |            T2 expires/   DHCPNAK/
 *     DHCPNAK/Discard     -------     |             Broadcast  Halt network
 *                 |       | |         |            DHCPREQUEST         |
 *                 +-------+ |        DHCPACK/          |               |
 *                      T1 expires/   Record lease, set |               |
 *                   Send DHCPREQUEST timers T1, T2     |               |
 *                   to leasing server |                |               |
 *                           |   ----------             |               |
 *                           |  |          |------------+               |
 *                           +->| RENEWING |                            |
 *                              |          |----------------------------+
 *                               ----------
 *
 *         Figure 5:  State-transition diagram for DHCP clients
 *
 * T1 = 0.5 * duration_of_lease
 * state.time_rebind = T2 = 0.875 * duration_of_lease
 *
 *---------------------------------------------------------------------------------------------
 */



#define	MAXDHCP	600				/* maximum size of DHCP datagram	*/
#define BROADCAST_DHCP "0.0.0.0:67" /* host 0 is broadcast */
#ifndef DHCP_REQUEST_IP
#define DHCP_REQUEST_IP 192,168,1,106
#endif
static char *dhcp_buff;

int dhcpInit(void)
{
	memset(&state, 0, sizeof(state));
	dot2ip(state.ip_local, DHCP_REQUEST_IP);
	state.dhcpsem = screate(1);
	return OK;
}

PROCESS	dhcp_Client()
{
	STATWORD ps;
	int	len;
	uint8_t msg_type;
	
	dhcp_buff = (char *)getmem(MAXDHCP);
	state.state = DHCP_STATE_INIT;
	memset(Net.myaddr, 0, IPLEN);
	if ( (state.device=(int)open(INTERNET, BROADCAST_DHCP, (void *)68)) == SYSERR)
	{
		kprintf_P(PSTR("dhcp_client_start: open fails\n"));
		send(state.startupPid,DHCP_CLIENT_EVT_ERROR);
		dhcp_Panic();
	}
	/* enable read(state.device,...) time-outs */
	/* set data mode: read & write dhcp_headers */
	control(state.device,DG_SETMODE,(void *)(DG_DMODE|DG_TMODE),(void *)0);
#ifdef DEBUG
	dgdump();
#endif
	while ( TRUE ) {
		switch (state.state)	{
				
			case DHCP_STATE_INIT:
				
				state.state = DHCP_STATE_SELECTING;
				if ( !dhcp_client_send((struct dhcp_header *)dhcp_buff, DHCP_MESSAGETYPE_DHCPDISCOVER) )
				{
					// Panic quit.
					send(state.startupPid,DHCP_CLIENT_EVT_TIMEOUT);
					dhcp_Panic();
				}				
				/* fall through */
				
			case DHCP_STATE_SELECTING:
				
				/* discard anything which is not an offer */
				if( (msg_type = dhcp_client_in((struct dhcp_header *)dhcp_buff)) != DHCP_MESSAGETYPE_DHCPOFFER)
				{
					// Panic quit.
					send(state.startupPid,DHCP_CLIENT_EVT_ERROR);
					dhcp_Panic();
				}			
				
				/* state transition */
				state.state = DHCP_STATE_REQUESTING;
				
				/* generate dhcp reply */
				if(!dhcp_client_send((struct dhcp_header *)dhcp_buff, DHCP_MESSAGETYPE_DHCPREQUEST))	{
					dhcp_client_abort();
					dhcp_client_event_report(DHCP_CLIENT_EVT_ERROR);
					break;
				}
				/* fall through */
				
			case DHCP_STATE_REQUESTING:
			case DHCP_STATE_REBINDING:
				
				/* ignore other/resent offers */
				if ( ( msg_type=dhcp_client_in((struct dhcp_header *)dhcp_buff) ) == DHCP_MESSAGETYPE_DHCPOFFER)
					break;
				
				/* restart from scratch if we receive something which is not an ack */
				if(msg_type != DHCP_MESSAGETYPE_DHCPACK)	{
					enum dhcp_client_event evt = DHCP_CLIENT_EVT_LEASE_DENIED;
					if(state.state == DHCP_STATE_REBINDING)
						evt = DHCP_CLIENT_EVT_LEASE_EXPIRED;
					
					dhcp_client_abort();
					dhcp_client_event_report(evt);
					break;
				}
				
				/* configure interface */
				disable(ps);
				memcpy(Net.myaddr, state.ip_local, IPLEN);
				netnum(Net.mynet, Net.myaddr);
				Net.mavalid = TRUE;
				memcpy(Net.gateway, state.ip_gateway, IPLEN);
				restore(ps);
				send(state.startupPid,DHCP_CLIENT_EVT_LEASE_ACQUIRED);
				state.state = DHCP_STATE_BOUND;
				/* fall through */
				
			case DHCP_STATE_BOUND:
				
				/* wait for the lease to expire */
				if (dhcpSleep(DG_DELAY_TIME) == TIMEOUT)
					state.state = DHCP_STATE_REBINDING;
				break;
				
		}
		len = read(state.device, (uint8_t *)dhcp_buff, MAXDHCP);
		if (len != TIMEOUT)	{
			/* ignore OFFER, ACK, NAK */
			state.dhcplen = len;
		}
	}
	return (SYSERR); /* can't get here */
}

int dhcp_client_start(void)
{
	int msg;
    /* reset network configuration */
	
	kprintf_P(PSTR("dhcp starting"));
	state.startupPid = getpid();
	resume( (state.dhcpPid = create(dhcp_Client,300,NETIPRI+1,"DHCPd",0)) );
	while ( (msg = recvtim(10)) == TIMEOUT)
	{
		kprintf_P(PSTR("."));
	}
	kprintf_P(PSTR("\n"));
	dhcp_client_event_report((enum dhcp_client_event)msg);
    return (OK);
}

void dhcp_Panic(void)
{
	freemem(dhcp_buff, MAXDHCP);
	kill( getpid() );		/* suicide */
}

/**
 * Aborts all ongoing DHCP actions and returns to INIT.
 *
 * \note The network configuration is not changed by calling this function. You
 *       may want to deconfigure the network interface to no longer occupy an
 *       IP address managed by the DHCP server.
 */
void dhcp_client_abort()
{
    if(state.state == DHCP_STATE_INIT)
        return;
	if (!isbadpid(state.timerPid))
	    kill(state.timerPid);
	state.timerPid = BADPID;
    state.state = DHCP_STATE_INIT;
}

/*
 *---------------------------------------------------------------------------
 * dhcp_client_in -- handles all incoming DHCP packets.
 *---------------------------------------------------------------------------
 */

uint8_t dhcp_client_in(struct dhcp_header* header)
{
	struct etblk *etptr;
	uint8_t msg_type;
	
	
	etptr = &eth[devtab[ETHER].dvminor];
    /* accept responses only */
    if (header->op != DHCP_OP_BOOTREPLY)
        return (SYSERR);
	
    /* accept responses with our id only */
    else if (header->xid != HTON32(DHCP_CLIENT_XID))
        return (SYSERR);
	
    /* accept responses with correct cookie only */
    else if (header->cookie != HTON32(DHCP_COOKIE))
        return (SYSERR);
	
    /* accept responses with our hardware address only */
    else if (memcmp(header->chaddr, etptr->etpaddr, EPADLEN) != 0)
        return (SYSERR);
	
    /* parse dhcp packet options */
    else if ( (msg_type = dhcp_client_parse(header)) == 0 )
        return (SYSERR);
	
	else
		return msg_type;
}

/**
 * Adds a message type option to the given DHCP packet.
 *
 * \param[in] options A pointer to the packet's end.
 * \param[in] type The message type to add to the packet.
 * \returns A pointer to the packet's new end.
 */
uint8_t* dhcp_client_packet_add_msgtype(uint8_t* options, uint8_t type)
{
    *options++ = DHCP_OPTION_MESSAGETYPE;
    *options++ = DHCP_OPTION_LEN_MESSAGETYPE;
    *options++ = type;
	
    return options;
}

/**
 * Adds a parameter request option to the given DHCP packet.
 *
 * The option requests the network parameters needed from the DHCP server.
 *
 * \param[in] options A pointer to the packet's end.
 * \returns A pointer to the packet's new end.
 */
uint8_t* dhcp_client_packet_add_params(uint8_t* options)
{
    *options++ = DHCP_OPTION_PARAMREQUEST;
    *options++ = 2;
    *options++ = DHCP_OPTION_NETMASK;
    *options++ = DHCP_OPTION_ROUTER;
	
    return options;
}

/**
 * Adds the DHCP server IP address as an option to the given DHCP packet.
 *
 * \param[in] options A pointer to the packet's end.
 * \returns A pointer to the packet's new end.
 */
uint8_t* dhcp_client_packet_add_server(uint8_t* options)
{
    *options++ = DHCP_OPTION_SERVERID;
    *options++ = DHCP_OPTION_LEN_SERVERID;
    memcpy(options, state.ip_server, sizeof(state.ip_server));
    options += sizeof(state.ip_server);
	
    return options;
}

/**
 * Adds our anticipated IP address as an option to the given DHCP packet.
 *
 * \param[in] options A pointer to the packet's end.
 * \returns A pointer to the packet's new end.
 */
uint8_t* dhcp_client_packet_add_ip(uint8_t* options)
{
    *options++ = DHCP_OPTION_REQUESTIP;
    *options++ = DHCP_OPTION_LEN_REQUESTIP;
    memcpy(options, state.ip_local, sizeof(state.ip_local));
    options += sizeof(state.ip_local);
	
    return options;
}

/**
 * Adds an option to the given DHCP packet marking the end of the option section.
 *
 * \param[in] options A pointer to the packet's end.
 * \returns A pointer to the packet's new end.
 */
uint8_t* dhcp_client_packet_add_end(uint8_t* options)
{
    *options++ = DHCP_OPTION_END;
	
    return options;
}

/*
 *------------------------------------------------------------------------------------
 * dhcp_client_send -- fill a DHCP packet appropriate for the current client state.
 *				send packet and recieve reply.
 *				returns TRUE on success, FALSE on failure.
 *------------------------------------------------------------------------------------
 */

Bool dhcp_client_send(struct dhcp_header *packet, uint8_t msg_type)
{
	int i, resp;
	
	/* generate dhcp header */
    memset(packet, 0, sizeof(*packet));
    packet->op = DHCP_OP_BOOTREQUEST;
    packet->htype = DHCP_HTYPE_ETHERNET;
    packet->hlen = DHCP_HLEN_ETHERNET;
    packet->xid = HTON32(DHCP_CLIENT_XID);
    packet->flags = HTON16(DHCP_FLAG_BROADCAST);
    packet->cookie = HTON32(DHCP_COOKIE);
    memcpy(packet->ciaddr, (uint8_t *)Net.myaddr, IPLEN); /* zero or last validated IP addr */
	memcpy(packet->yiaddr, state.ip_local, IPLEN);
	memcpy(packet->siaddr, state.ip_server, IPLEN);
    memcpy(packet->chaddr, (uint8_t *)eth[0].etpaddr, EPADLEN);
	
    /* append dhcp options */
    uint8_t* options_end = (uint8_t*) (packet + 1);
    options_end = dhcp_client_packet_add_msgtype(options_end, msg_type);
    options_end = dhcp_client_packet_add_params(options_end);
	if(msg_type == DHCP_MESSAGETYPE_DHCPDISCOVER)
		{
        options_end = dhcp_client_packet_add_ip(options_end);
		}
    if(msg_type == DHCP_MESSAGETYPE_DHCPREQUEST)
		{
        options_end = dhcp_client_packet_add_ip(options_end);
        options_end = dhcp_client_packet_add_server(options_end);
		}
    options_end = dhcp_client_packet_add_end(options_end);
	state.dhcplen = options_end - (uint8_t*)packet;
	
	for (i=0; i<DHCP_CLIENT_RETRIES; i++)	{
		write(state.device, (uint8_t *)packet, state.dhcplen);
		resp = read(state.device, (uint8_t *)packet, MAXDHCP);
		if (resp != TIMEOUT)	{
			state.dhcplen = resp;
			return TRUE;
		}
	}
	dhcp_client_event_report(DHCP_CLIENT_EVT_ERROR);
	dhcp_client_abort();
	return FALSE;
}

/**
 * Parses the given DHCP server packet.
 *
 * \param[in] packet The packet received from the DHCP server.
 * \returns The message type of the packet on success, \c 0 on failure.
 */
uint8_t dhcp_client_parse(const struct dhcp_header* packet)
{
    if(!packet)
        return 0;
	
    /* save ip address proposed by the server */
    memcpy(state.ip_local, packet->yiaddr, sizeof(state.ip_local));
	memcpy(state.ip_server, packet->siaddr, sizeof(state.ip_server));
	
    /* parse options */
    uint8_t msg_type = 0;
    const uint8_t* opts = (const uint8_t*) (packet + 1);
    while(1)	{
        const uint8_t* opt_data = opts + 2;
        uint8_t opt_type = *opts;
        uint8_t opt_len = *(opts + 1);
		
        if(opt_type == DHCP_OPTION_PAD)	{
            ++opts;
        }
        else if(opt_type == DHCP_OPTION_END)	{
            break;
        }
        else if(opt_type == DHCP_OPTION_NETMASK && opt_len == DHCP_OPTION_LEN_NETMASK)	{
            memcpy(state.ip_netmask, opt_data, sizeof(state.ip_netmask));
            opts += DHCP_OPTION_LEN_NETMASK + 2;
        }
        else if(opt_type == DHCP_OPTION_ROUTER && opt_len / 4 > 0)	{
            memcpy(state.ip_gateway, opt_data, sizeof(state.ip_gateway));
            opts += opt_len + 2;
        }
        else if (opt_type == DHCP_OPTION_TIMELEASE && opt_len == DHCP_OPTION_LEN_TIMELEASE)	{
            state.time_rebind = ntoh32(*((const uint32_t*) opt_data));
            state.time_rebind -= state.time_rebind / 8;
            opts += DHCP_OPTION_LEN_TIMELEASE + 2;
        }
        else if (opt_type == DHCP_OPTION_MESSAGETYPE && opt_len == DHCP_OPTION_LEN_MESSAGETYPE)	{
            msg_type = *opt_data;
            opts += DHCP_OPTION_LEN_MESSAGETYPE + 2;
        }
        else if (opt_type == DHCP_OPTION_SERVERID && opt_len == DHCP_OPTION_LEN_SERVERID)	{
            memcpy(state.ip_server, opt_data, sizeof(state.ip_server));
            opts += DHCP_OPTION_LEN_SERVERID + 2;
        }
        else	{
            opts += opt_len + 2;
        }
    }
	
    return msg_type;
}

void dhcp_client_event_report(enum dhcp_client_event event)
{
	//#ifdef DEBUG
    switch(event)
    {
        case DHCP_CLIENT_EVT_LEASE_ACQUIRED:
            kprintf_P(PSTR("[dhcp] lease acquired\n"));
            break;
        case DHCP_CLIENT_EVT_LEASE_DENIED:
            kprintf_P(PSTR("[dhcp] lease denied\n"));
            break;
        case DHCP_CLIENT_EVT_LEASE_EXPIRING:
            kprintf_P(PSTR("[dhcp] lease expiring\n"));
            break;
        case DHCP_CLIENT_EVT_LEASE_EXPIRED:
            kprintf_P(PSTR("[dhcp] lease expired\n"));
            break;
        case DHCP_CLIENT_EVT_TIMEOUT:
            kprintf_P(PSTR("[dhcp] timeout\n"));
            break;
        case DHCP_CLIENT_EVT_ERROR:
            kprintf_P(PSTR("[dhcp] error\n"));
            break;
    }
	//#endif
}

/*
 *------------------------------------------------------------------------
 *  dhcpSleep -  DHCP sleep timer
 *------------------------------------------------------------------------
 */

static int dhcpSleep(int delta)
{
	
	state.time_rebind -= delta;
	
	if (state.time_rebind <= 0)
		return (TIMEOUT);
	return OK;
}
