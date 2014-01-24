/* mem.h - freestk, roundew, truncew */

/*----------------------------------------------------------------------
 * roundew, truncew - round or trunctate address to next even word
 *----------------------------------------------------------------------
 */
#define	roundew(x)	(WORD *)( (3 + (WORD)(x)) & ~03 )
#define	truncew(x)	(WORD *)( ((WORD)(x)) & ~03 )


/*----------------------------------------------------------------------
 * roundmb, truncmb -- round or truncate address up to size of mblock
 *----------------------------------------------------------------------
 */
//#define	roundmb(x)	(WORD *)( (7 + (WORD)(x)) & ~07 )
#define	roundmb(x)	(WORD *)( ((sizeof(struct mblock)-1) + (WORD)(x)) & ~(sizeof(struct mblock)-1) )
#define	truncmb(x)	(WORD *)( ((WORD)(x)) & ~(sizeof(struct mblock)-1) )


/*----------------------------------------------------------------------
 *  freestk  --  free stack memory allocated by getstk
 *----------------------------------------------------------------------
 */
#define freestk(p,len)	freemem( (struct mblock *)( (unsigned)(p)			\
				- (unsigned)(roundmb(len))	\
				+ (unsigned)sizeof(int) ),	\
				(unsigned)roundmb(len) )

struct mblock {
	struct mblock *volatile mnext;
	unsigned int volatile mlen;
	};
extern struct mblock memlist;	/* head of free memory list	*/
extern char *__malloc_heap_start;
extern char *__malloc_heap_end;

#define NULLBLK (struct mblock *)0

extern WORD *getstk();

