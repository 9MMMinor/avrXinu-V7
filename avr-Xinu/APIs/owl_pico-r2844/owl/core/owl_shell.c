#ifdef __avrXinu__
#include <avr-Xinu.h>
void printMem(void);
#else
#include <stdio.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>

#define OWL_TRACE OWL_TRACE_DBG

#include <owl/core/owl_shell.h>
#include <owl/core/owl_debug.h>
#include <owl/core/owl_err.h>

#ifndef MAX_PRINTF_LEN
#define MAX_PRINTF_LEN 160
#endif

#ifndef OWLSH_MAXARGS
# define OWLSH_MAXARGS 12
#endif

#ifndef OWLSH_CMDLEN
# define OWLSH_CMDLEN  80
#endif

struct owlsh_ctx {
        void *data;                             /* context, passed to cmd_f */
        struct owlsh_cmd *cmdlist;              /* list of commands */
        const char *prompt;                     /* prompt */
        void (*outb)(void *ctx, int c);
        void *ctx;                              /* passed to outb */
    
        char *buf;
        int buflen;
        int curpos;
        int escape;

        int flags;
        
        int argc;
        const char *argv[OWLSH_MAXARGS];

        struct owlsh_cmd *defcmd;
};

static void parse_line(char *line, int *argc, const char **argv);
static struct owlsh_cmd *owlsh_find_cmd(struct owlsh_ctx *priv,
                                        const char *label);

void
owlsh_destroy(struct owlsh_ctx *priv)
{
        if (priv == NULL)
                return;

        if (priv->buf)
                free(priv->buf);

        free(priv);
}

struct owlsh_ctx *
owlsh_create(const char *prompt, 
             void (*outb)(void *ctx, int c), void *ctx, int flags)
{
        struct owlsh_ctx *priv;
        
        if (outb == NULL)
                return NULL;

        priv = calloc(1, sizeof(struct owlsh_ctx));

        if (priv == NULL)
                return NULL;
        priv->prompt = prompt ? prompt : "$";
        priv->buflen = OWLSH_CMDLEN;
        priv->buf = calloc(1, priv->buflen);
        if (priv->buf == NULL) {
                free (priv);
			kprintf("free %p\n",priv);
			printMem();
                return NULL;
        }
        priv->flags = flags;
        priv->outb = outb;
        priv->ctx = ctx;
        if (!(priv->flags & OWLSH_NOPROMPT))
                owlsh_printf(priv, "%s ", priv->prompt);
        return priv;
}

void
owlsh_addcmd(struct owlsh_ctx *priv, struct owlsh_cmd *cmd, void *ctx)
{
        cmd->ctx = ctx;
        if (priv->cmdlist == NULL) {
                priv->cmdlist = cmd;

        } else {
                struct owlsh_cmd *iter = priv->cmdlist;
                while (iter->next)
                        iter = iter->next;

                iter->next = cmd;
        }

        cmd->next = NULL;
}


void
owlsh_defcmd(struct owlsh_ctx *priv, struct owlsh_cmd *cmd, void *ctx)
{
        cmd->ctx = ctx;
        priv->defcmd = cmd;
}

void *
owlsh_data(struct owlsh_ctx *priv)
{
        return priv->data;
}

void
owlsh_key(struct owlsh_ctx *priv, char key)
{
        struct owlsh_cmd *cmd = NULL;

        /* check escapechars */
        if (priv->escape) {
                /* save one char for the null termination */
                if (priv->curpos >= OWLSH_CMDLEN - 1) {
                        priv->escape = 0;
                        return;
                }

                switch (key) {
                case 'r':
                        priv->buf[priv->curpos] = '\r';
                        break;
                        
                case 'n':
                        priv->buf[priv->curpos] = '\n';
                        break;

                case '\\':
                        priv->buf[priv->curpos] = '\\';
                        break;
                }
                
                priv->curpos++;
                priv->escape = 0;
                return;
        }
        switch (key) {
        case '\\':
                if (priv->flags & OWLSH_ESCAPECHARS) {
                        priv->escape = 1;
                        break;
                }
                
                /* fall through */

        case '\b':
		case 0x7f:					/* Xinu delete key mapping */
                if (priv->curpos == 0)
                        break;

                if (!(priv->flags & OWLSH_NOECHO))
                        owlsh_printf(priv, "\b \b");
                priv->curpos--;
                break;

        case '\r':
        case '\n':
                priv->buf[priv->curpos] = '\0';

                if (!(priv->flags & OWLSH_NOECHO))
                        owlsh_printf(priv, "\n\r");

                parse_line(priv->buf, &priv->argc, priv->argv);
                cmd = owlsh_find_cmd(priv, priv->argv[0]);
                if (cmd != NULL) {
                        priv->data = cmd->ctx;
                        cmd->cmd_f(priv, priv->argc, priv->argv);
                } else if (priv->defcmd) {
                        priv->data = priv->defcmd->ctx;
                        priv->defcmd->cmd_f(priv, priv->argc, priv->argv);
                } else if (strlen(priv->argv[0])) {
                        owlsh_printf(priv, "command not found:%s\n\r",
                                     priv->argv[0]);
                }

                priv->curpos = 0;

                if (!(priv->flags & OWLSH_NOPROMPT))
                        owlsh_printf(priv, "%s ", priv->prompt);
                break;

        default:
                /* save one char for the null termination */
                if (priv->curpos >= OWLSH_CMDLEN - 1)
                        break;
                
                /* only store visible chars */
                if (key >= ' ' && key < 0x7f) {
                        if (!(priv->flags & OWLSH_NOECHO))
                                owlsh_printf(priv, "%c", key);
                        priv->buf[priv->curpos] = key;
                        priv->curpos++;
                }

                break;
        }
}

void
owlsh_write(struct owlsh_ctx *priv, char *buf, int len)
{
        int i;
        for (i = 0; i < len; i++)
                priv->outb(priv->ctx, buf[i]);
}

void
owlsh_hexdump(struct owlsh_ctx *priv, const char *title, const char *pos,
              int len)
{
	char line[80], *p;
	int i, j;
        owlsh_printf(priv, "%s - hexdump(len=%d):\n\r", title, len); 
	for (i = 0; i < len; i += 16) {
		p = line;
		for (j = 0; j < 16; j++) {
                        if (i + j < len)
                                sniprintf(p, line + sizeof(line) - p, "%02x ", 
                                          (unsigned char)pos[i + j]);
			else
				strcpy(p, "   ");
			p += 3;
		}
                
                strcpy(p, ": ");
                for (j = 0; j < 16; j++) {
                        if (i + j < len) {
                                if (isprint((unsigned char) pos[i + j]))
                                        *p++ = pos[i + j];
                                else
                                        *p++ = '.';
                        }
                }
		*p = '\0';
		owlsh_printf(priv, "   %s\n\r", line);
	}
} 

extern void My_free(void *);
int
owlsh_printf(struct owlsh_ctx *priv, const char *fmt, ...)
{
        char *str = NULL;
        va_list args;
        int len;
        char *iter;

        va_start(args, fmt);

        if ((str = malloc(MAX_PRINTF_LEN)) == NULL)
                return OWL_ERR_MEM;
        
        if ((len = vsniprintf(str, MAX_PRINTF_LEN, fmt, args)) < 0) {
                free(str);
                return OWL_ERR_MEM;
        }
        iter = str;
        while (*iter)
                priv->outb(priv->ctx, *iter++);
        free(str);
        return len;
}


/**
 * Caller responsible for freeing the result string.
 *
 */ 
char *
owlsh_joinargs(int argc, const char *argv[])
{
        int i;
        char *str = calloc(1, 1);
        if (str == NULL)
                return NULL;

        /* join arg list */
        for (i = 0; i < argc; i++) {
                int sep;
                void *p;

                /* one byte extra for the null char and one byte for the
                 * extra space added unless this was the last token
                 */
                sep = i < argc - 1 ? 1 : 0;
                p = realloc(str, strlen(str) + strlen(argv[i]) + 1 +
                            sep);
                if (p == NULL) {
                        free(str);
                        return NULL;
                }
                        
                str = p;
                strcat(str, argv[i]);
                if (sep)
                        strcat(str, " ");
        }

        return str;
}

static struct owlsh_cmd *
owlsh_find_cmd(struct owlsh_ctx *priv, const char *label)
{
        struct owlsh_cmd *cmd = NULL;
        struct owlsh_cmd *iter;
        int ambiguous = 0;
        
        if (strlen(label) == 0)
                return NULL;
        
        for (iter = priv->cmdlist; iter; iter = iter->next) {

                /* exact match */
                if (!strcmp(iter->label, label))
                        return iter;

                /* partial match */
                if (strncmp(iter->label, label, strlen(label)))
                        continue;

                /* ambiguous */
                if (cmd)
                        ambiguous = 1;

                cmd = iter;
        }
        
        return ambiguous ? NULL : cmd;
}

static void
parse_line(char *line, int *argc, const char **argv)
{
        char *tok;
        *argc = 0;
        argv[*argc] = "";
        for (tok = strtok(line, " "); tok; tok = strtok(NULL, " ")) {
                argv[(*argc)++] = tok;
                if (*argc == OWLSH_MAXARGS)
                        return;
        }
}

static void
cmd_help_f(struct owlsh_ctx *priv, int argc, const char *argv[])
{
        struct owlsh_cmd *iter;

        (void) argc; /* ununsed */
        (void) argv; /* ununsed */
        
        for (iter = priv->cmdlist; iter; iter = iter->next)
                owlsh_printf(priv, "%-20s %s\n\r", iter->label,
                             iter->help ? iter->help : "");
}

struct owlsh_cmd cmd_help = {
        "help", cmd_help_f, "print this information", NULL, NULL
};
