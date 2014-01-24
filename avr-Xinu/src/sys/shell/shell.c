/* shell.c - shell */

#include <avr-Xinu.h>
#include <shell.h>
#include <cmd.h>
#include <tty.h>

#ifndef NAMESPACE
#define NAMESPACE -1
#endif

SYSCALL getutim(long *);
int lexan(char *);
int addarg(int, int);
SYSCALL	setdev(int, int, int);
SYSCALL	setnok(int, int);

struct	shvars	Shl;				/* globals used by Xinu shell	*/
struct	cmdent	cmds[]  = 	{		/* shell commands		*/
/* 		 name		Builtin?	procedure	*/

		{"bpool",	FALSE,		x_bpool},
//		{"conf",	FALSE,		x_conf},
		{"date",	FALSE,		x_date},
//		{"devs",	FALSE,		x_devs},
//		{"dg",		FALSE,		x_dg},
		{"netstat",	FALSE,		x_net},
//		{"cat",		FALSE,		x_cat},
//		{"close",	FALSE,		x_close},
//		{"cp",		FALSE,		x_cp},
//		{"create",	FALSE,		x_creat},
//		{"echo",	FALSE,		x_echo},
//		{"exit",	TRUE,		x_exit},
		{"help",	FALSE,		x_help},
//		{"kill",	TRUE,		x_kill},
//		{"logout",	TRUE,		x_exit},
		{"mem",		FALSE,		x_mem},
//		{"mount",	FALSE,		x_mount},
//		{"mv",		FALSE,		x_mv},
		{"ps",		FALSE,		x_ps},
//		{"reboot",	TRUE,		x_reboot},
//		{"rf",		FALSE,		x_rf},
//		{"rls",		FALSE,		x_rls},
//		{"rm",		FALSE,		x_rm},
//		{"routes",	FALSE,		x_routes},
//		{"ruptime",	FALSE,		x_uptime},
//		{"sleep",	FALSE,		x_sleep},
//		{"time",	FALSE,		x_date},
//		{"unmount",	FALSE,		x_unmou},
//		{"uptime",	FALSE,		x_uptime},
//		{"who",		FALSE,		x_who},
//		{"worm"		FALSE,		playworm},
		{"?",		FALSE,		x_help}	};

static	char	*errhd	= "Syntax error\n"; /* global error messages	*/
static	char	*fmt	= "Cannot open %s\n";
static	char	*fmt2	= "[%d]\n";

/*------------------------------------------------------------------------
 *  shell  -  Xinu shell with file redirection and background processing
 *------------------------------------------------------------------------
 */
 
int shell(int dev)
{
	int		ntokens;
	int		i, j, len;
	int		com;
	char	*outnam, *innam;
	int		stdIN, stdOUT, stdERR;
	Bool	backgnd;
	char	ch, mach[SHMLEN];
	int		child;
	FILE * stream = stdout;

	Shl.shncmds = sizeof(cmds)/sizeof(struct cmdent);
	for (getname(mach) ; TRUE ; ) {
		fprintf(stream, "%s %% ", mach);
		getutim(&Shl.shlast);
		if ( (len = read(file_get_fdesc(stream), (unsigned char *)Shl.shbuf, SHBUFLEN)) == 0)
			len = read(file_get_fdesc(stream), (unsigned char *)Shl.shbuf, SHBUFLEN);
		if (len == EOF)
			break;
		Shl.shbuf[len-1] = NULLCH;
		if ( (ntokens=lexan(Shl.shbuf)) == SYSERR) {
			fprintf(stream, errhd);
			continue;
		} else if (ntokens == 0)
			continue;
		outnam = innam = NULL;
		backgnd = FALSE;

		/* handle '&' */

		if (Shl.shtktyp[ntokens-1] == '&') {
			ntokens-- ;
			backgnd = TRUE;
		}

		/* scan tokens, accumulating length;  handling redirect	*/

		for (len=0,i=0 ; i<ntokens ; ) {
			if ((ch = Shl.shtktyp[i]) == '&') {
				ntokens = -1;
				break;
			} else if (ch == '>') {
				if (outnam != NULL || i >= --ntokens) {
					ntokens = -1;
					break;
				}
				outnam = Shl.shtok[i+1];
				for (ntokens--,j=i ; j<ntokens ; j++) {
					Shl.shtktyp[j] = Shl.shtktyp[j+2];
					Shl.shtok  [j] = Shl.shtok  [j+2];
				}
				continue;
			} else if (ch == '<') {
				if (innam != NULL || i >= --ntokens) {
					ntokens = -1;
					break;
				}
				innam = Shl.shtok[i+1];
				for (ntokens--,j=i ; j < ntokens ; j++) {
					Shl.shtktyp[j] = Shl.shtktyp[j+2];
					Shl.shtok  [j] = Shl.shtok  [j+2];
				}
				continue;
			} else {
				 len += strlen(Shl.shtok[i++]);
			}
		}
		if (ntokens <= 0) {
			fprintf(stream, errhd);
			continue;
		}
		stdIN = stdOUT = file_get_fdesc(stream);
		stdERR = CONSOLE;

		/* Look up command in table */

		for (com=0 ; com<Shl.shncmds ; com++) {
			if (strcmp(cmds[com].cmdnam,Shl.shtok[0]) == 0)
				break;
		}
		if (com >= Shl.shncmds) {
			fprintf(stream, "%s: not found\n", Shl.shtok[0]);
			continue;
		}

		/* handle built-in commands with procedure call */

		if (cmds[com].cbuiltin) {
			if (innam != NULL || outnam != NULL || backgnd)
				fprintf(stream, errhd);
			else if ( (*cmds[com].cproc)(stdIN, stdOUT,
				stdERR, ntokens, Shl.shtok) == SHEXIT)
				break;
			continue;
		}

		/* Open files and redirect I/O if specified */

		if (innam != NULL && (stdIN=open(NAMESPACE,innam,"ro"))
			== SYSERR) {
			fprintf(stream, fmt, innam);
			continue;
		}
		if (outnam != NULL && (stdOUT=open(NAMESPACE,outnam,"w"))
			== SYSERR) {
			fprintf(stream, fmt, outnam);
			continue;
		}

		/* compute space needed for string args. (in bytes) */

		/* add a null for the end of each string, plus a    */
		/*    pointer to the string (see xinu2, p300)       */
//		len += ntokens * (sizeof(char *) + sizeof(char));

		/* plus a (char *) null for the end of the table    */
//		len += sizeof(char *);

		/* plus a pointer to the head of the table          */
//		len += sizeof(char *);

		
//		len = (len+3) & ~(unsigned) 0x3;

		control(file_get_fdesc(stream), TCINT, (void *)getpid(), (void *)0);

		/* create process to execute conventional command */

		if ( (child = create(cmds[com].cproc, SHCMDSTK, SHCMDPRI,
				Shl.shtok[0],4,stdIN, stdOUT, stdERR, ntokens))
				== SYSERR ) {
			fprintf(stream, "Cannot create\n");
			close(stdOUT);
			close(stdIN);
			continue;
		}
		addarg(child, ntokens);		/* , len); */
		setdev(child, stdIN, stdOUT);
		if (backgnd) {
			fprintf(stream, fmt2, child);
			resume(child);
		} else {
			setnok(getpid(), child);
			recvclr();
			resume(child);
			if (receive() == INTRMSG) {
				setnok(BADPID, child);
				fprintf(stream, fmt2, child);
			}
		}
	}
	return(OK);
}
