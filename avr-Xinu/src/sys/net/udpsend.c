/* udpsend.c - udpsend */

#include <conf.h>
#include <kernel.h>
#include <network.h>

void udpdump(struct udp *pudp, int dlen, int order);
uint16_t udp_calc_checksum(IPaddr faddr, struct udp *packet, uint16_t packet_len);

/*
 *------------------------------------------------------------------------
 *  udpsend  -  send one UDP datagram to a given (foreign) IP address
 *------------------------------------------------------------------------
 */
 
int udpsend(IPaddr faddr, int fport, int lport, struct epacket *packet, int datalen)
{
	register struct	udp	*udpptr;
	register struct	ip	*ipptr;

	/* Fill in UDP header; pass to ipsend to fill in IP header */

	ipptr = (struct ip *) packet->ep_data;
	ipptr->i_proto = IPRO_UDP;
	udpptr = (struct udp *) ipptr->i_data;
	udpptr->u_sport = hton16(lport);
	udpptr->u_dport = hton16(fport);
	udpptr->u_udplen = hton16(UHLEN+datalen);
	if (isodd(datalen))
		udpptr->u_data[datalen] = (char)0;
	udpptr->u_ucksum = hton16(udp_calc_checksum(faddr, udpptr, UHLEN+datalen)); // or = 0
//	udpdump(udpptr, UHLEN+datalen, 0);
	return( ipsend(faddr, packet, UHLEN+datalen) );
}

/*
 * Copyright (c) 2006-2008 by Roland Riegel <feedback@roland-riegel.de>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
/**
 * Calculates the checksum of a UDP packet.
 *
 * \returns The 16-bit checksum.
 */
 
uint16_t udp_calc_checksum(IPaddr faddr, struct udp *packet, uint16_t packet_len)
{
    /* pseudo header */
    uint16_t checksum = IPRO_UDP + packet_len;
    checksum = net_calc_checksum(checksum, (uint8_t*)Net.myaddr, 4, 4);
    checksum = net_calc_checksum(checksum, (uint8_t*)faddr, 4, 4);
	
    /* real package */
    return ~net_calc_checksum(checksum, (uint8_t*) packet, packet_len, 6);
}
