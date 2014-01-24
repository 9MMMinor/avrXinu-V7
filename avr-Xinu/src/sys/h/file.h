/* file.h */
#ifndef __FILE_H_
#define __FILE_H_ 1

#define	FLREAD		001		/* file mode bit for "read"	*/
#define	FLWRITE		002		/* file mode bit for "write"	*/
#define	FLRW		003		/* file mode bits for read+write*/
#define	FLNEW		010		/* file mode bit for "new file"	*/
#define	FLOLD		020		/* file mode bit for "old file"	*/

/* Definition of characters in the mode argument to open */

#define	FLOMODE		'o'		/* Old  => file exists		*/
#define	FLNMODE		'n'		/* New  => file does not exist	*/
#define	FLRMODE		'r'		/* Read => open read-only	*/
#define	FLWMODE		'w'		/* Write=> open for writing	*/

/* Universal file control functions (additional args in parentheses)	*/

#define	FLREMOVE	3001		/* remove file (name)		*/
#define	FLACCESS	3002		/* test access (name, mode)	*/
#define	FLRENAME	3003		/* change file name (old, new)	*/
#define FLMKDIR		3004		/* mkdir(char *name, int mode)  */
#define FLRMDIR		3005		/* rmdir(char *name)			*/

#endif
