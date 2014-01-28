#ifndef OWL_TIMER_H
#define OWL_TIMER_H
#include <inttypes.h>

struct owltmr_tmo;

typedef void (owltmr_tmo_cb_t)(struct owltmr_tmo *, void *);

int owltmr_init(int hz);
struct owltmr_tmo *owltmr_addtmo(int ms, int periodic,
                                 owltmr_tmo_cb_t *cb, void *ctx);

int owltmr_deltmo(struct owltmr_tmo *tmo);
void owltmr_modtmo(struct owltmr_tmo *tmo, int ms);
uint32_t owltmr_get_ms(void);
void owltmr_tick(void);
void owltmr_poll(void);
int owltmr_intvl_passed(uint32_t old, uint32_t new, uint32_t diff);
uint32_t owltmr_get_intvl(uint32_t old, uint32_t new);
void owltmr_mdelay(int ms);

#endif
