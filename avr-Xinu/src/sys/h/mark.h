/* mark.h - unmarked */

typedef	int	MARKER[1];	/* by declaring it to be an array, the	*/
						/* name provides an address so forgotten*/
						/* &'s don't become a problem			*/
#ifndef	MAXMARK
#define	MAXMARK	20		/* maximum number of marked locations	*/
#endif
#ifdef	MEMMARK
extern	int	*(marks[]);
extern	int	nmarks;
extern	int	mkmutex;


#define	unmarked(L)		(L[0]<0 || L[0]>=nmarks || marks[L[0]]!=L)
#define marked(L)               !unmarked(L)
    
#endif

// prototypes
void _mkinit(void);
int mark(int*);
