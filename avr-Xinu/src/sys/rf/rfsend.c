/* rfsend.c - rfsend */

#include <avr-Xinu.h>
#include <network.h>

/*
 *------------------------------------------------------------------------
 *  rfsend  --  send message to remote server and await reply
 *------------------------------------------------------------------------
 */

#ifndef WLP_API_WIRELESS

int
rfsend(struct fphdr *fptr, int reqlen, int rplylen)
{
	STATWORD ps;    
	int	trys;
	int	ret;

	/* Clear server queue, and send packet to it */

	if (Rf.device == RCLOSED) {
		Rf.device = open(INTERNET, RSERVER, ANYLPORT);
		if (Rf.device == SYSERR ||
		    control(Rf.device, DG_SETMODE, (char*)(DG_DMODE|DG_TMODE), NULL)
		    	== SYSERR)
			return(SYSERR);
	}
	disable(ps);
	control(Rf.device, DG_CLEAR, NULL, NULL);
	for (trys=0 ; trys<RMAXTRY ; trys++) {
		if ( write(Rf.device, (unsigned char *)fptr, reqlen)
			== SYSERR) {
			restore(ps);
			return(SYSERR);
		}
		if ( (ret=read(Rf.device, (unsigned char *)fptr, rplylen) )
		    !=SYSERR && ret != TIMEOUT) {
			restore(ps);
			return(ret);
		}
	}
	restore(ps);
	return(SYSERR);
}

#endif /* ndef */

#ifdef WLP_API_WIRELESS
#define RFILESERVER 192,168,1,100
#define RFILEPORT 2001
#define TO_IP4_ADDR(a,b)

#include <wlp_api.h>
#include <wlp_inet.h>
#include <wlp_proto.h>

static void rfsend_callBack(void *ctx, int sockid, int len);
static struct rf_ctx	{
	int	pid;
} rf_ctx;

int
rfsend(struct fphdr *fptr, int reqlen, int rplylen)
{   
	int	trys;
	int	ret;
	struct ip_addr rfserver;
	int message;
	
	/* Clear server queue, and send packet to it */
	
	if (Rf.device == RCLOSED) {
		if ( (Rf.device = wlp_socket(WLP_SOCK_DGRAM, 0)) <= 0 )	{
//			kprintf("socket err = %d\n", Rf.device);
			return(SYSERR);
		}
		IP4_ADDR(&rfserver, 192,168,1,100);
		if ( (ret = wlp_connect(Rf.device, &rfserver, RFILEPORT)) < 0 )	{
//			kprintf("connect fail\n");
			wlp_close(Rf.device);
			return(SYSERR);
		}
		rf_ctx.pid = getpid();
		wlp_set_recv_cb(Rf.device, rfsend_callBack, &rf_ctx);
	}
			
	recvclr();	/* clear incomming message buffer */

	for (trys=0 ; trys<RMAXTRY ; trys++) {
		if ( wlp_send(Rf.device, (const char *)fptr, reqlen) <= 0) {
			return(SYSERR);
		}
		ret = 0;
		message = recvtim(5*TICK); /* receive or time-out in 5 seconds */
		if (message > 0)	{
			ret = wlp_recv(Rf.device, (char *)fptr, message);
		}
		if (ret > 0)	{
			return(ret);
		}
	}
	return(SYSERR);
}

static void
rfsend_callBack(void *ctx, int sockid, int len)
{
	struct rf_ctx *ctx_ptr = ctx;
		
//	kprintf("rfsend_callBack avail=%d\n", len);
	send(ctx_ptr->pid, len);
}

#endif /* WLP_API_WIRELESS */
