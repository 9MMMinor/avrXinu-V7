#ifndef WLP_PING_H
#define WLP_PING_H

#include <stdint.h>

#include <wlp_api.h>

#include <owl/core/owl_shell.h>

#define ICMP_ER 0      /* echo reply */
#define ICMP_DUR 3     /* destination unreachable */
#define ICMP_SQ 4      /* source quench */
#define ICMP_RD 5      /* redirect */
#define ICMP_ECHO 8    /* echo */
#define ICMP_TE 11     /* time exceeded */
#define ICMP_PP 12     /* parameter problem */
#define ICMP_TS 13     /* timestamp */
#define ICMP_TSR 14    /* timestamp reply */
#define ICMP_IRQ 15    /* information request */
#define ICMP_IR 16     /* information reply */

/**
 * Send ICMP echo request(s).
 *
 * @param addr is the destionation IP address
 * @param count is the number of ICMP packets to send
 * @param datasize the size of the payload in the ICMP packet
 * @param reply_cb is a pointer to a function that will be invoked once for
 *              every ICMP packet received. The len parameter will hold
 *              the total size of the packet, the src parameter will point
 *              to the source ip address, the seqno parameter is the ICMP
 *              sequence number and the type is the ICMP type (e.g. echo reply
 *              or destination unreachable).
 * @param done_cb will be invoked when all the ICMP packets have been
 *              transmitted and a delay of 1 second after the last packet.
 * @param ctx is an opaque context pointer passed to reply_cb and done_cb.
 * @return 0 on success
 *         OWL_ERR_MEM if out of memory.
 *
 */
int ping(const struct ip_addr *addr, int count, int datasize,
         void (reply_cb)(void *ctx, int len, struct ip_addr *src, int seqno,
                         int type),
         void (done_cb)(void *ctx, struct ip_addr *dst, int sent, int recvd),
         void *ctx);

extern struct owlsh_cmd cmd_ping;


#endif
