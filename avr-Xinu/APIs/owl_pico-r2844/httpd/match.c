#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "match.h"

char *match_string(char *match, 
                   size_t *match_offset,
                   char *src, 
                   size_t src_len,
                   int case_sens) {
        char *p, *mp;

        p = src;
        mp = match;
        if ( match_offset ) {
                mp += *match_offset;
                *match_offset = 0;
        }
#define TO_LOWER(x) ( case_sens ? (x) : tolower((x)) )
        /* Scan through the source buffer */
        while ( p - src < src_len ) {
                /* If we reach the end of mp we have a match */
                if ( '\0' == *mp ) {
                        goto out;
                }
                if ( TO_LOWER(*p) != TO_LOWER(*mp) ) {
                        mp = match;
                        if ( TO_LOWER(*p) == TO_LOWER(*mp) ) {
                                mp++;
                        }
                }
                else {
                        mp++;
                }
                p++;
        }
        p = NULL;
out:
        if ( match_offset ) {
                *match_offset = mp - match;
        }

        return p;
}
