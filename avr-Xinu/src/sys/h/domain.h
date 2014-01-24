/* domain.h - dn_cat */

#ifndef	NSERVER
#define	NSERVER		"192.168.1.100:53"	/* Server IP address & UDP port	*/
#endif

/* Definitions of bits in the operation and parameter field */

#define	DN_QR		0100000		/* Query (0) or response (1) bit	*/
#define	DN_OPCDE	0074000		/* operation  code for query:	*/
					/*  0 => standard query		*/
					/*  1 => inverse query,		*/
					/*  2 => completion query,	*/
					/*  3 => completion (1 answer),	*/
					/*  4-15 => reserved.		*/
#define	DN_AA		0002000		/* 1 if authoritative answer	*/
#define	DN_TR		0001000		/* 1 if message truncated	*/
#define	DN_RD		0000400		/* 1 if recursion desired	*/
#define	DN_RA		0000200		/* 1 if recursion available	*/
#define	DN_RESVD	0000160		/* reserved			*/
#define	DN_RESP		0000017		/* response code:		*/
					/*  0 => no errors in query	*/
					/*  1 => format error in query	*/
					/*  2 => server failure		*/
					/*  3 => name does not exist	*/
#define DN_BIT_QR		15		/* QR bit; query (0) or response (1) */
#define DN_BIT_OPCODE	11		/* op code field */
#define DN_BIT_AA		10		/* AA bit; authorative answer (1) */
#define DN_BIT_TC		9		/* TC bit; truncated (1) */
#define	DN_BIT_RD		8		/* RD bit; recursion desired (1) */
#define	DN_BIT_RA		7		/* RA bit; recursion available (1) */
#define	DN_BIT_RESVD	4		/* reserved	3- bit field */
#define	DN_BIT_RCODE	0		/* response code:		*/

 
struct	dn_mesg	{				/* domain system message format	*/
	uint16_t	dn_id;			/* message id			*/
	uint16_t	dn_opparm;		/* operation and parmameter bits*/
	uint16_t	dn_qcount;		/* # entries in question section*/
	uint16_t	dn_acount;		/* # RRs in answer section	*/
	uint16_t	dn_ncount;		/* # RRs in nameserver section	*/
	uint16_t	dn_dcount;		/* # RRs in additional section	*/
	uint8_t		dn_qaaa[1];		/* start of rest of the message	*/
	/* remaining fields of the domain name message are of variable	*/
	/* length, and consist of (1) a question section, (2) an answer	*/
	/* section, (3) an authority section (which says where to find	*/
	/* answers when they cannot be supplied), and (4) an addition	*/
	/* information section.  Entries in these are Resource Records.	*/
};

typedef struct {
	uint16_t	id :16;		/* query identification number */
#if BYTE_ORDER == BIG_ENDIAN
	/* fields in third byte */
	uint16_t	qr: 1;		/* response flag */
	uint16_t	opcode: 4;	/* purpose of message */
	uint16_t	aa: 1;		/* authoritive answer */
	uint16_t	tc: 1;		/* truncated message */
	uint16_t	rd: 1;		/* recursion desired */
	/* fields in fourth byte */
	uint16_t	ra: 1;		/* recursion available */
	uint16_t	unused :1;	/* unused bits (MBZ as of 4.9.3a3) */
	uint16_t	ad: 1;		/* authentic data from named */
	uint16_t	cd: 1;		/* checking disabled by resolver */
	uint16_t	rcode :4;	/* response code */
#endif
#if BYTE_ORDER == LITTLE_ENDIAN
	/* fields in third byte */
	uint16_t	rd :1;		/* recursion desired */
	uint16_t	tc :1;		/* truncated message */
	uint16_t	aa :1;		/* authoritive answer */
	uint16_t	opcode :4;	/* purpose of message */
	uint16_t	qr :1;		/* response flag */
	/* fields in fourth byte */
	uint16_t	rcode :4;	/* response code */
	uint16_t	cd: 1;		/* checking disabled by resolver */
	uint16_t	ad: 1;		/* authentic data from named */
	uint16_t	unused :1;	/* unused bits (MBZ as of 4.9.3a3) */
	uint16_t	ra :1;		/* recursion available */
#endif
	/* remaining bytes */
	uint16_t	qdcount :16;	/* number of question entries */
	uint16_t	ancount :16;	/* number of answer entries */
	uint16_t	nscount :16;	/* number of authority entries */
	uint16_t	arcount :16;	/* number of resource entries */
} DNS_Header_t;

struct	dn_qsuf	{			/* question section name suffix	*/
	uint16_t	dn_type;		/* type of this name		*/
	uint16_t	dn_clas;		/* class of this name		*/
};

/*
 * Currently defined opcodes.
 */
typedef enum {
	ns_o_query	= 0,	/* Standard query. */
	ns_o_iquery = 1,	/* Inverse query (deprecated/unsupported). */
	ns_o_status = 2,	/* Name server status query (unsupported). */
						/* Opcode 3 is undefined/reserved. */
	ns_o_notify = 4,	/* Zone change notification. */
	ns_o_update = 5,	/* Zone update message. */
	ns_o_max = 6
} ns_opcode;

/*
 * Currently defined response codes.
 */
typedef	enum	{
	ns_r_noerror	= 0,	/* No error occurred. */
	ns_r_formerr	= 1,	/* Format error. */
	ns_r_servfail	= 2,	/* Server failure. */
	ns_r_nxdomain	= 3,	/* Name error. */
	ns_r_notimpl	= 4,	/* Unimplemented. */
	ns_r_refused	= 5,	/* Operation refused. */
							/* these are for BIND_UPDATE */
	ns_r_yxdomain	= 6,	/* Name exists */
	ns_r_yxrrset	= 7,	/* RRset exists */
	ns_r_nxrrset	= 8,	/* RRset does not exist */
	ns_r_notauth	= 9,	/* Not authoritative for zone */
	ns_r_notzone	= 10,	/* Zone of record different from zone section */
	ns_r_max		= 11,
							/* The following are EDNS extended rcodes */
	ns_r_badvers	= 16,
							/* The following are TSIG errors */
	ns_r_badsig		= 16,
	ns_r_badkey		= 17,
	ns_r_badtime	= 18
} ns_rcode;

/*
 * Currently defined type values for resources and queries.
 */

typedef enum __ns_type {
	ns_t_invalid = 0,	/* Cookie. */
	ns_t_a = 1,			/* Host address. */
	ns_t_ns = 2,		/* Authoritative server. */
	ns_t_md = 3,		/* Mail destination. */
	ns_t_mf = 4,		/* Mail forwarder. */
	ns_t_cname = 5,		/* Canonical name. */
	ns_t_soa = 6,		/* Start of authority zone. */
	ns_t_mb = 7,		/* Mailbox domain name. */
	ns_t_mg = 8,		/* Mail group member. */
	ns_t_mr = 9,		/* Mail rename name. */
	ns_t_null = 10,		/* Null resource record. */
	ns_t_wks = 11,		/* Well known service. */
	ns_t_ptr = 12,		/* Domain name pointer. */
	ns_t_hinfo = 13,	/* Host information. */
	ns_t_minfo = 14,	/* Mailbox information. */
	ns_t_mx = 15,		/* Mail routing information. */
	ns_t_txt = 16,		/* Text strings. */
	ns_t_rp = 17,		/* Responsible person. */
	ns_t_afsdb = 18,	/* AFS cell database. */
	ns_t_x25 = 19,		/* X_25 calling address. */
	ns_t_isdn = 20,		/* ISDN calling address. */
	ns_t_rt = 21,		/* Router. */
	ns_t_nsap = 22,		/* NSAP address. */
	ns_t_nsap_ptr = 23,	/* Reverse NSAP lookup (deprecated). */
	ns_t_sig = 24,		/* Security signature. */
	ns_t_key = 25,		/* Security key. */
	ns_t_px = 26,		/* X.400 mail mapping. */
	ns_t_gpos = 27,		/* Geographical position (withdrawn). */
	ns_t_aaaa = 28,		/* Ip6 Address. */
	ns_t_loc = 29,		/* Location Information. */
	ns_t_nxt = 30,		/* Next domain (security). */
	ns_t_eid = 31,		/* Endpoint identifier. */
	ns_t_nimloc = 32,	/* Nimrod Locator. */
	ns_t_srv = 33,		/* Server Selection. */
	ns_t_atma = 34,		/* ATM Address */
	ns_t_naptr = 35,	/* Naming Authority PoinTeR */
	ns_t_kx = 36,		/* Key Exchange */
	ns_t_cert = 37,		/* Certification record */
	ns_t_a6 = 38,		/* IPv6 address (deprecates AAAA) */
	ns_t_dname = 39,	/* Non-terminal DNAME (for IPv6) */
	ns_t_sink = 40,		/* Kitchen sink (experimentatl) */
	ns_t_opt = 41,		/* EDNS0 option (meta-RR) */
	ns_t_tkey = 249,	/* Transaction key */
	ns_t_tsig = 250,	/* Transaction signature. */
	ns_t_ixfr = 251,	/* Incremental zone transfer. */
	ns_t_axfr = 252,	/* Transfer zone of authority. */
	ns_t_mailb = 253,	/* Transfer mailbox records. */
	ns_t_maila = 254,	/* Transfer mail agent records. */
	ns_t_any = 255,		/* Wildcard match. */
	ns_t_zxfr = 256,	/* BIND-specific, nonstandard. */
	ns_t_max = 65536
} ns_type;

#define	DN_MLEN		128		/* message length (small query)	*/
#define	dn_cat(t,f)	{*t++ =(char)strlen(f);strcpy(t,f);t+=strlen(f);}

/* Query type codes */

#define	DN_QTHA		1		/* Host address			*/
#define	DN_QTNS		2		/* Authoratative name server	*/
#define	DN_QTMD		3		/* Mail destination (obsolete)	*/
#define	DN_QTMF		4		/* Mail forwarder (obsolete)	*/
#define	DN_QTCN		5		/* Canonical name for an alias	*/
#define	DN_QTSZ		6		/* Start of zone of authority	*/
#define	DN_QTMB		7		/* Mailbox domain name		*/
#define	DN_QTMG		8		/* Mail group member		*/
#define	DN_QTMR		9		/* Mail rename domain name	*/
#define	DN_QTNL		10		/* Null resource record		*/
#define	DN_QTWK		11		/* Well-known service descriptor*/
#define	DN_QTPR		12		/* Domain name pointer		*/
#define	DN_QTHI		13		/* Host information		*/
#define	DN_QTMI		14		/* Mailbox or mail list info.	*/
#define	DN_QTMX		15		/* Mail, replaces MD & MF	*/

/* Query class codes */

#define	DN_QCIN		1		/* The DARPA Internet		*/
#define	DN_QCCS		2		/* CSNET (now obsolete)		*/
#define	DN_QCHA		3		/* Chaos network		*/

#define	DN_CMPRS	0300		/* Compressed format is pointer	*/
#define	DN_CPTR		037777		/* Compressed format bits of ptr*/
#define	DN_RLEN		10		/* resource record heading len.	*/
