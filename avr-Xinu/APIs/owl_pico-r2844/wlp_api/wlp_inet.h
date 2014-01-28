#ifndef WLP_INET_H
#define WLP_INET_H
#include <stdint.h>

#include "wlp_proto.h"

#ifndef LITTLE_ENDIAN
# define LITTLE_ENDIAN 0x0a0b0c0d /* lsb first */
#endif

#ifndef BIG_ENDIAN
# define BIG_ENDIAN    0x0d0c0b0a /* msb first */
#endif


#if BYTE_ORDER == LITTLE_ENDIAN
uint16_t htons(uint16_t x);
uint16_t ntohs(uint16_t x);
uint32_t htonl(uint32_t x);
uint32_t ntohl(uint32_t x);
#else
# define htons(x) (x)
# define ntohs(x) (x)
# define htonl(x) (x)
# define ntohl(x) (x)
#endif


int inet_aton(const char *cp, struct ip_addr *addr);
char *inet_ntoa(const struct ip_addr *addr);

#endif
