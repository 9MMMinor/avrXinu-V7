
#include "wlp_inet.h"

#if BYTE_ORDER == LITTLE_ENDIAN
uint16_t htons(uint16_t x)
{
        return ((x << 8) & 0xff00) |
                ((x >> 8) & 0xff);
}

uint16_t ntohs(uint16_t x)
{
        return htons(x);
}

uint32_t htonl(uint32_t x)
{
        return ((x << 24) & 0xff000000) |
                ((x << 8) & 0xff0000) |
                ((x >> 8) & 0xff00) |
                ((x >> 24) & 0xff);
}

uint32_t ntohl(uint32_t x)
{
        return htonl(x);
}
#endif


/**
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 *
 * @param cp IP address in ascii represenation (e.g. "127.0.0.1")
 * @param addr pointer to which to save the ip address in network order
 * @return 1 if cp could be converted to addr, 0 on failure
 */

/* Here for now until needed in other places in lwIP */
#ifndef isprint
#define in_range(c, lo, up)  ((uint8_t)c >= lo && (uint8_t)c <= up)
#define isprint(c)           in_range(c, 0x20, 0x7f)
#define isdigit(c)           in_range(c, '0', '9')
#define isxdigit(c)          (isdigit(c) || in_range(c, 'a', 'f') || in_range(c, 'A', 'F'))
#define islower(c)           in_range(c, 'a', 'z')
#define isspace(c)           (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
#endif    

int
inet_aton(const char *cp, struct ip_addr *addr)
{
        uint32_t val;
        int base, n, c;
        uint32_t parts[4];
        uint32_t *pp = parts;

        c = *cp;
        for (;;) {
                /*
                 * Collect number up to ``.''.
                 * Values are specified as for C:
                 * 0x=hex, 0=octal, 1-9=decimal.
                 */
                if (!isdigit(c))
                        return (0);
                val = 0;
                base = 10;
                if (c == '0') {
                        c = *++cp;
                        if (c == 'x' || c == 'X') {
                                base = 16;
                                c = *++cp;
                        } else
                                base = 8;
                }
                for (;;) {
                        if (isdigit(c)) {
                                val = (val * base) + (int)(c - '0');
                                c = *++cp;
                        } else if (base == 16 && isxdigit(c)) {
                                val = (val << 4) | (int)(c + 10 - (islower(c) ?
                                                                   'a' : 'A'));
                                c = *++cp;
                        } else
                                break;
                }
                if (c == '.') {
                        /*
                         * Internet format:
                         *  a.b.c.d
                         *  a.b.c   (with c treated as 16 bits)
                         *  a.b (with b treated as 24 bits)
                         */
                        if (pp >= parts + 3)
                                return (0);
                        *pp++ = val;
                        c = *++cp;
                } else
                        break;
        }
        /*
         * Check for trailing characters.
         */
        if (c != '\0' && (!isprint(c) || !isspace(c)))
                return (0);
        /*
         * Concoct the address according to
         * the number of parts specified.
         */
        n = pp - parts + 1;
        switch (n) {

        case 0:
                return (0);       /* initial nondigit */

        case 1:             /* a -- 32 bits */
                break;

        case 2:             /* a.b -- 8.24 bits */
                if (val > 0xffffffUL)
                        return (0);
                val |= parts[0] << 24;
                break;

        case 3:             /* a.b.c -- 8.8.16 bits */
                if (val > 0xffff)
                        return (0);
                val |= (parts[0] << 24) | (parts[1] << 16);
                break;

        case 4:             /* a.b.c.d -- 8.8.8.8 bits */
                if (val > 0xff)
                        return (0);
                val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
                break;
        }
        if (addr)
                addr->addr = htonl(val);
        return (1);
}

/**
 * Convert numeric IP address into decimal dotted ASCII representation.
 * returns ptr to static buffer; not reentrant!
 *
 * @param addr ip address in network order to convert
 * @return pointer to a global static (!) buffer that holds the ASCII
 *         represenation of addr
 */
char *
inet_ntoa(const struct ip_addr *addr)
{
        static char str[16];
        uint32_t s_addr = addr->addr;
        char inv[3];
        char *rp;
        uint8_t *ap;
        uint8_t rem;
        uint8_t n;
        uint8_t i;

        rp = str;
        ap = (uint8_t *)&s_addr;
        for(n = 0; n < 4; n++) {
                i = 0;
                do {
                        rem = *ap % (uint8_t)10;
                        *ap /= (uint8_t)10;
                        inv[i++] = '0' + rem;
                } while(*ap);
                while(i--)
                        *rp++ = inv[i];
                *rp++ = '.';
                ap++;
        }
        *--rp = 0;
        return str;
}

