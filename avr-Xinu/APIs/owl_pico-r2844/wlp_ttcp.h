#ifndef WLP_TTCP_H
#define WLP_TTCP_H

#include <stdint.h>

#include <wlp_api.h>

#include <owl/core/owl_shell.h>

int ttcp_connect(const struct ip_addr *ip, uint16_t port, uint16_t nbuf,
                 uint16_t buflen,
                 void (*progress_cb)(void *ctx, int progress),
                 void (*done_cb)(void *ctx, int err, int bytes, uint32_t ms),
                 void *ctx);

int ttcp_listen(uint16_t port, uint16_t buflen,
                void (*progress_cb)(void *ctx, int progress),
                void (*done_cb)(void *ctx, int err, int bytes, uint32_t ms),
                void *ctx);

extern struct owlsh_cmd cmd_ttcp;

#endif
