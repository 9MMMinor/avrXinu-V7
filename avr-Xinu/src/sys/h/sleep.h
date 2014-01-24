/* sleep.h */


extern int clkruns;					/* 1 iff clock exists; 0 otherwise	*/
									/* Set at system startup.		*/
extern int clockq;					/* q index of sleeping process list	*/
extern unsigned char countTick;		/* used to count down clock ticks	*/
extern volatile long clktime;				/* current time in secs since 1/1/70	*/
extern volatile unsigned long ctr100;		/* Version 8 timer/counter */
extern unsigned int clmutex;		/* mutual exclusion sem. for clock	*/
extern int *sltop;					/* address of first key on clockq	*/
extern int slnempty;				/* 1 iff clockq is nonempty		*/

extern Bool defclk;					/* >0 iff clock interrupts are deferred	*/
extern unsigned int clkdiff;		/* number of clock clicks deferred	*/
extern void clkint();				/* clock interrupt handler		*/
