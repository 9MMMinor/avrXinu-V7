#ifndef OWL_DEBUG_H
#define OWL_DEBUG_H

#include "owl_env.h"

/**
 * In each source files that uses owl_err(), owl_info(), owl_dbg() or
 * owl_assert(), define OWL_TRACE to the desired debug level before including
 * this file; e.g
 *
 * #define OWL_TRACE OWL_TRACE_INFO
 * #include "owl_debug.h"
 *
 * Debug traces and assertions can be globally disabled by defining
 * OWL_TRACE_DISABLE_ALL and/or OWL_ASSERT_DISABLE_ALL in the makefile.
 *
 * Default trace level (which can be override in each c-file) is set with
 * OWL_TRACE_DEFAULT
 *
 */

#ifndef OWL_TRACE
# define OWL_TRACE OWL_TRACE_ERR
# ifndef OWL_TRACE_DISABLE_ALL
#  define OWL_TRACE_DISABLE_ALL
# endif
#endif

#define OWL_TRACE_NONE    0
#define OWL_TRACE_ERR     1
#define OWL_TRACE_INFO    2
#define OWL_TRACE_DBG     3
#define OWL_TRACE_VDBG    4
#define OWL_TRACE_DATA    5

#define OWL_TRACE_DEBUG -1
#if OWL_TRACE == OWL_TRACE_DEBUG
# error "use OWL_TRACE_DBG instead of OWL_TRACE_DEBUG"
#endif

int owl_printf(const char *fmt, ...);

#if !defined(OWL_ASSERT_DISABLE_ALL)
# define owl_assert(cond)                                               \
        do {                                                            \
                if (!(cond)) {                                          \
                        owl_printf("--- ASSERTION FAILED ---\n\r");     \
                        owl_printf("-A- %s:%d %s: %s\n\r",              \
                                   __FILE__, __LINE__, __func__, #cond); \
                }                                                       \
        } while(0)

#else
# define owl_assert(...)
#endif

#if !defined(OWL_TRACE_DISABLE_ALL) &&                                  \
        (OWL_TRACE >= OWL_TRACE_VDBG ||                                 \
         (!defined(OWL_TRACE) && OWL_TRACE_DEFAULT >= OWL_TRACE_VDBG))       
# define owl_vdbg(format, args...)                                       \
        owl_printf("-V- %s:%d %s: " format "\n\r", __FILE__, __LINE__,  \
                   __func__, ## args)
#else
# define owl_vdbg(...)
#endif

#if !defined(OWL_TRACE_DISABLE_ALL) &&                                  \
        (OWL_TRACE >= OWL_TRACE_DBG ||                                  \
         (!defined(OWL_TRACE) && OWL_TRACE_DEFAULT >= OWL_TRACE_DBG))       
# define owl_dbg(format, args...)                                       \
        owl_printf("-D- %s:%d %s: " format "\n\r", __FILE__, __LINE__,  \
                   __func__, ## args)
#else
# define owl_dbg(...)
#endif

#if !defined(OWL_TRACE_DISABLE_ALL) &&                                  \
        (OWL_TRACE >= OWL_TRACE_INFO ||                                 \
         (!defined(OWL_TRACE) && OWL_TRACE_DEFAULT >= OWL_TRACE_INFO))      
# define owl_info(format, args...)                              \
        owl_printf("-I- %s:%d %s: " format "\n\r", __FILE__, __LINE__,  \
                   __func__, ## args)
#else
# define owl_info(...)
#endif

#if !defined(OWL_TRACE_DISABLE_ALL) &&                                  \
    (OWL_TRACE >= OWL_TRACE_ERR ||                                      \
     (!defined(OWL_TRACE) && OWL_TRACE_DEFAULT >= OWL_TRACE_ERR))       
# define owl_err(format, args...)                                       \
        owl_printf("-E- %s:%d %s: " format "\n\r", __FILE__, __LINE__,  \
                   __func__, ## args)
#else
# define owl_err(...)
#endif

void owl_hexdump_f(const char *title, const char *pos, int len);

#if !defined(OWL_TRACE_DISABLE_ALL) && OWL_TRACE >= OWL_TRACE_DATA
# define owl_hexdump(args...) owl_hexdump_f(args)
#else
# define owl_hexdump(args...)
#endif

#endif
