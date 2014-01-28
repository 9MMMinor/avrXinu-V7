//
//  owl.h
//  owl_pico_avrxinu
//
//  Created by Michael Minor on 2/20/12.
//  Copyright (c) 2012 Michael M Minor. All rights reserved.
//

#ifndef owl_pico_avrxinu_owl_h
#define owl_pico_avrxinu_owl_h

#ifndef	Nowl
#define	Nowl		1		/* number of serial owl lines	*/
#endif

#ifndef	OW_OBUFLEN
#define	OW_OBUFLEN	64		/* num.	chars in output	queue	*/
#endif
#ifndef	OW_IBUFLEN
#define	OW_IBUFLEN	64		/* num.	chars in input	queue	*/
#endif


struct	owlcblk	{					/* owl control block	*/
	struct wlp_req	*req;
	int				reqlen;
	struct wlp_cfm	*cfm;
	int				cfmlen;
	uint8_t			*rwbuf;
	int				rwlen;
	int				pid;
	int				userLevel_state;
	volatile int	interruptLevel_state;
	int				mutex_Semaphore;
};
extern struct owlcblk owltab[Nowl];

typedef int devcall;

#ifndef F_CPU
//#define F_CPU 11059200UL
#define F_CPU 3686400UL
#endif

/* User-level (driver upper-half) states */
#define CONTROL_STATUS_REQ 1
#define BLOCK_WRITE 2
#define BLOCK_READ 3

/* Interrupt-level (driver lower-half) states */
#define READY 1
#define WACK  2
#define TRANSMITTING_REQ 3
#define RECEIVING_CFM 4
#define RECEIVING_DATA 5
#define TRANSMITTING_DATA 6
#define INITIALIZE 7
#define INITIALIZE_ACK 8

devcall owl_Init(struct devsw *devptr);	/* entry in device switch table	*/
void owl_ISR(struct owlcblk *owptr);
void USART_Flush( void );
int board_uart_read_f(void *ctx, void *buf, int len);
int board_uart_write_f(void *ctx, const void *buf, int len);
int sendRequest(struct wlp_req *req, int reqlen,
				struct wlp_cfm *cfm, int cfmlen,
				uint8_t *buf, int buflen);
int owl_BlockRead(void *ctx, void *buf, int len);
int owl_BlockWrite(void *ctx, const void *buf, int len);
int owl_RW(struct owlcblk *owptr, uint8_t *buf, int len);
int owluart_Startup(struct wlp_req *wlpRequest);
//int sendRequest(struct wlp_req *req, int reqlen,
//				struct wlp_cfm *cfm, int cfmlen,
//				uint8_t *buf, int buflen);

#endif
