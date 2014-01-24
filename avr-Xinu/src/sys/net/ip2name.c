/* ip2name.c - ip2name */

#include <avr-Xinu.h>
#include <network.h>

void packet_DataDump(char * routine, uint8_t *p, int len);
void packet_Dump(char * routine, struct epacket *epkptr, int len);

/*------------------------------------------------------------------------
 *  ip2name  -  return DARPA Domain name for a host given its IP address
 *------------------------------------------------------------------------
 */

SYSCALL ip2name(IPaddr ip, char *nam)
{
	char	tmpstr[20];		/* temporary string buffer	*/
	char	*buf;			/* buffer to hold domain query	*/
	int	dg, i;
	register char	*p;
	register struct	dn_mesg *dnptr;

	dnptr = (struct dn_mesg *) (buf = (char *) getmem(DN_MLEN));
	*nam = NULLCH;
	dnptr->dn_id = 0;					/* Query ID SHOULD be zero */
	dnptr->dn_opparm = HTON16(	(0<<DN_BIT_QR)		|
								(0<<DN_BIT_OPCODE)	|
								(0<<DN_BIT_AA)		|
								(0<<DN_BIT_TC)		|
								(0<<DN_BIT_RD)		|
								(0<<DN_BIT_RA)		|
								(0<<DN_BIT_RCODE)	);
	dnptr->dn_qcount = HTON16(1);
	dnptr->dn_acount = dnptr->dn_ncount = dnptr->dn_dcount = 0;
	p = (char *)dnptr->dn_qaaa;

	/* Fill in question with  ip[3].ip[2].ip[1].ip[0].in-addr.arpa  */

	for (i=3 ; i >= 0 ; i--) {
		sprintf(tmpstr, "%d", ip[i] & LOWBYTE);
		dn_cat(p, tmpstr);
	}
	dn_cat(p, "in-addr");
	dn_cat(p, "arpa");
	*p++ = NULLCH;	/* terminate name */

	/* Add query type and query class fields to name */

	( (struct dn_qsuf *)p )->dn_type = HTON16(DN_QTPR);
	( (struct dn_qsuf *)p )->dn_clas = HTON16(DN_QCIN);
	p += sizeof(struct dn_qsuf);

	/* Broadcast query */
/*	Internet Draft                Multicast DNS              23rd March 2010
	
	5.1 One-Shot Multicast DNS Queries
	
	The most basic kind of Multicast DNS client may simply send its DNS
	queries blindly to 224.0.0.251:5353, without necessarily even being
	aware of what a multicast address is. This change can typically be
	implemented with just a few lines of code in an existing DNS resolver
	library. Any time the name being queried for falls within one of the
	reserved mDNS domains (see Section 12 "Special Characteristics of
	Multicast DNS Domains") rather than using the configured unicast DNS
	server address, the query is instead sent to 224.0.0.251:5353 (or its
	IPv6 equivalent [FF02::FB]:5353). Typically the timeout would also be
	shortened to two or three seconds. It's possible to make a minimal
	mDNS client with only these simple changes. These queries are
	typically done using a high-numbered ephemeral UDP source port,
	but regardless of whether they are sent from a dynamic port or from
	a fixed port, these queries SHOULD NOT be sent using UDP source port
	5353, since using UDP source port 5353 signals the presence of a
	fully-compliant Multicast DNS client, as described below.
		
	A simple DNS client like this will typically just take the first
	response it receives. It will not listen for additional UDP
	responses, but in many instances this may not be a serious problem.
	If a user types "http://MyPrinter.local." into their web browser and
	gets to see the status and configuration web page for their printer,
	then the protocol has met the user's needs in this case.
		
	While a basic DNS client like this may be adequate for simple
	host name lookup, it may not get ideal behavior in other cases.
	Additional refinements that may be adopted by more sophisticated
	clients are described below.
 */
//	dg = (int)open(INTERNET, "224.0.0.251:5353", ANYLPORT);
			

	dg = (int)open(INTERNET, NSERVER, ANYLPORT);
	
	control(dg, DG_SETMODE, (void *)(DG_DMODE | DG_TMODE), (void *)0);
	write (dg, (uint8_t *)buf, p - buf);
	if ( (i = read(dg, (uint8_t *)buf, DN_MLEN)) == SYSERR || i == TIMEOUT)
		kprintf("No response from name server\n");
	close(dg);
//	if (i>0) packet_DataDump("ip2name",buf,i);
	if (ntoh16(dnptr->dn_opparm) & DN_RESP ||
	    ntoh16(dnptr->dn_acount) <= 0) {
		freemem(buf, DN_MLEN);
		return(SYSERR);
	}

	/* In answer, skip name and remainder of resource record header	*/

	while (*p != NULLCH)
		if (*p & DN_CMPRS) 	/* compressed section of name	*/
			*++p = NULLCH;
		else
			p += *p + 1;
	p += DN_RLEN + 1;

	/* Copy name to user */

	*nam = NULLCH;

	while (*p != NULLCH) {
		if (*p & DN_CMPRS)
			p = buf + (net2hs(*(int *)p) & DN_CPTR);
		else {
			strncat(nam, p+1, *p);
			strcat(nam, ".");
			p += *p + 1;
		}
	}
	if (strlen(nam))	/* remove trailing dot */
		nam[strlen(nam) - 1] = NULLCH;
	freemem(buf, DN_MLEN);
	return(OK);
}
