#ifndef OWL_SHELL_H
#define OWL_SHELL_H

struct owlsh_ctx;

#define OWLSH_NOPROMPT    (1 << 0)
#define OWLSH_NOECHO      (1 << 1)
#define OWLSH_ESCAPECHARS (1 << 2)

struct owlsh_cmd {
        const char *label;
        void (*cmd_f)(struct owlsh_ctx *shell, int args, const char *argv[]);
        const char *help;
        void *ctx;
        struct owlsh_cmd *next;
};

struct owlsh_ctx *owlsh_create(
        const char *prompt, 
        void (*outb)(void *ctx, int c), void *ctx, int flags);
void owlsh_addcmd(struct owlsh_ctx *priv, struct owlsh_cmd *cmd, void *ctx);
void owlsh_defcmd(struct owlsh_ctx *priv, struct owlsh_cmd *cmd, void *ctx);
void owlsh_destroy(struct owlsh_ctx *priv);
void owlsh_key(struct owlsh_ctx *priv, char key);
void *owlsh_data(struct owlsh_ctx *priv);
int owlsh_printf(struct owlsh_ctx *priv, const char *fmt, ...)
#if __GNUC__
    __attribute__((format(printf, 2, 3)))
#endif
    ;
void owlsh_write(struct owlsh_ctx *priv, char *buf, int len);
char *owlsh_joinargs(int argc, const char *argv[]);
void owlsh_hexdump(struct owlsh_ctx *priv, const char *title, const char *pos,
                   int len);

extern struct owlsh_cmd cmd_help;

#endif
