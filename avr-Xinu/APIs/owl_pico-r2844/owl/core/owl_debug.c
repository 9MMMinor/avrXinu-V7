#ifdef __avrXinu__
#include <avr-Xinu.h>
#else
#include <stdio.h>
#endif

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#define OWL_TRACE OWL_TRACE_DBG
#include <owl/core/owl_err.h>
#include <owl/core/owl_debug.h>

#ifndef MAX_PRINTF_LEN
#define MAX_PRINTF_LEN 160
#endif

int
owl_printf(const char *fmt, ...)
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
                owl_putc(*iter++);

        free(str);
        return len;
}

void
owl_hexdump_f(const char *title, const char *pos, int len)
{
	char line[80], *p;
	int i, j;
        owl_printf("%s - hexdump(len=%d):\n\r", title, len); 
	for(i = 0; i < len; i += 16) {
		p = line;
		for(j = 0; j < 16; j++) {
			if(i + j < len)
				sniprintf(p, line + sizeof(line) - p, "%02x ",
                                          (unsigned char)pos[i + j]);
			else
				strcpy(p, "   ");
			p += 3;
		}
		strcpy(p, ": ");
		for(j = 0; j < 16; j++) {
			if(i + j < len) {
				if(isprint((unsigned char) pos[i + j]))
					*p++ = pos[i + j];
				else
					*p++ = '.';
			}
		}
		*p = '\0';
		owl_printf("   %s\n\r", line);
	}
} 
