/* kernel.h - disable, enable, halt, restore, isodd, min */

//#include <avr/io.h>
#include <stdint.h>		/* AVR integer types */

/* Symbolic constants used throughout Xinu */

typedef	char		Bool;	/* Boolean type				*/
#define	FALSE		0		/* Boolean constants		*/
#define	TRUE		1
#define	NULL		(char *)0	/* Null pointer for linked lists*/
#define NULLPTR		(char *)0
#define	NULLCH		'\0'	/* The null character		*/
#define	NULLSTR		""		/* Pointer to empty string	*/
#define	SYSCALL		WORD	/* System call declaration	*/
#define DEVCALL		WORD
#define	LOCAL		static int	/* Local procedure declaration	*/
#define	COMMAND		int		/* Shell command declaration	*/
#define	BUILTIN		int		/* Shell builtin " "		*/
#define	INTPROC		void	/* Interrupt procedure  "	*/
#define	PROCESS		WORD	/* Process declaration		*/
#define	LOWBYTE		0x00ff	/* mask for low-order 8 bits	*/
#define	HIBYTE		0xff00	/* mask for high 8 of 16 bits	*/
#define	RESCHYES	1		/* tell	ready to reschedule	*/
#define	RESCHNO		0		/* tell	ready not to resch.	*/
#define	MINSTK		200		/* minimum process stack size	*/
#define	NULLSTK		200		/* process 0 stack size		*/
#define	MAGIC		0xaa	/* unusual value for top of stk	*/

/* Universal return constants */

#define	OK			1		/* system call ok		*/
#define	SYSERR		-1		/* system call failed		*/
#define	EOF			-2		/* End-of-file (usu. from read)	*/
#define	TIMEOUT		-3		/* time out  (usu. recvtim)	*/
#define	INTRMSG		-4		/* keyboard "intr" key pressed	*/
					    /*  (usu. defined as ^B)	*/
/* Initialization constants */

#define	INITSTK		600		/* initial process stack size	*/
#define	INITPRIO	20		/* initial process priority		*/
#define	INITNAME	"main"	/* initial process name			*/
#define	INITARGS	1,0		/* initial count/arguments		*/
#define	INITRET		userret	/* processes return address		*/
#define	INITPS		0x80	/* initial process SREG (interrupts enabled)	*/
#define	INITREG		0		/* initial register contents	*/
#define	QUANTUM		10		/* clock ticks until preemption	*/

/* Machine dependent definitions					*/

#define MININT INT16_MIN
#define MAXINT INT16_MAX



/* Machine size definitions						*/

typedef	char	CHAR;		/* sizeof the unit the holds a character*/
typedef	int     WORD;		/* maximum of (int, char *)		*/
typedef	char	*PTR;		/* sizeof a char. or fcnt. pointer 	*/
typedef int     INT;		/* sizeof compiler integer		*/
typedef	char	REG;		/* sizeof machine register		*/

typedef uint8_t	STATWORD[1];	/* saved machine status for disable/restore */
                                /* make array so address automatic          */

#define hibyte(x) (unsigned char)(((int)(x)>>8)&0xff)
#define lobyte(x) (unsigned char)(((int)(x))&0xff)
#define	isodd(x)	(01&(WORD)(x))
#define	min(a,b)	( (a) < (b) ? (a) : (b) )
#define enable() __asm__ __volatile__ ("sei" ::)          /* enable global interrupts */
#define pause() __asm__ __volatile__ ("sleep" ::)         /* enter sleep mode waiting for interrupt */
#define halt() __asm__ __volatile__ ("break" ::)          /* set CPU in Stopped Mode */


extern int rdyhead, rdytail;
extern int preempt;
extern int noint;

extern void disable(STATWORD);
extern void restore(STATWORD);

/* Device calls */
extern int ioerr(void);
extern int ionull(void);
/* debug print */
extern int kprintf(const char *fmt, ...);
extern int kprintf_P(const char *fmt, ...);

