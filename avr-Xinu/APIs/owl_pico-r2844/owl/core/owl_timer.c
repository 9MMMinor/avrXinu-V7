#define OWL_TRACE OWL_TRACE_ERR

#ifdef __avrXinu__
#include <avr-Xinu.h>
#else
#include <stdio.h>
#endif

#include <stdlib.h>

#include <owl/core/owl_timer.h>
#include <owl/core/owl_debug.h>
#include <owl/core/owl_err.h>

struct owltmr_tmo {
        struct owltmr_tmo *next;
        owltmr_tmo_cb_t *cb;
        void *ctx;
#define TMO_EXPIRED  (1 << 0) /* fired but not deleted */
#define TMO_PERIODIC (1 << 1) /* reschedules automatically after fire */
#define TMO_DELETED  (1 << 2) /* pending delete */
        uint32_t flags;
        uint32_t tick;      /* compare tick */
        uint32_t intvl;     /* interval in ticks */
};

struct owltmr_ctx {
        struct owltmr_tmo *head;
        volatile uint32_t tick;
        uint32_t hz;
#define OWLTMR_FLAG_POLL (1 << 0)
        int flags;
};


static struct owltmr_ctx *priv = NULL;

int
owltmr_init(int hz)
{
        if (hz == 0)
                return OWL_ERR_PARAM;
        
        priv = (struct owltmr_ctx *)calloc(1, sizeof(struct owltmr_ctx));
        if (priv == NULL) {
                owl_err("could not alloc priv");
                return OWL_ERR_MEM;
        }

        priv->hz = hz;
        return 0;
}

struct owltmr_tmo *
owltmr_addtmo(int ms, int periodic, owltmr_tmo_cb_t *cb, void *ctx)
{
        struct owltmr_tmo *tmo;

        owl_info("entry ms:%d periodic:%d cb:%p ctx:%p",
                 ms, periodic, cb, ctx);

        if (cb == NULL) {
                owl_err("cb is null");
                return NULL;
        }
        
        tmo = calloc(1, sizeof(struct owltmr_tmo));
        if (tmo == NULL) {
                owl_err("could not alloc tmo");
                return NULL;
        }

        tmo->cb = cb;
        tmo->ctx = ctx;
        if (periodic)
                tmo->flags |= TMO_PERIODIC;

        if (priv->head == NULL) {
                owl_dbg("set head to %p", tmo);
                priv->head = tmo;
                
        } else {
                struct owltmr_tmo *iter;
                iter = priv->head;
                while (iter->next)
                        iter = iter->next;

                owl_dbg("adding timer %p to tail of %p", tmo, iter);
                iter->next = tmo;
        }

        owltmr_modtmo(tmo, ms);
        return tmo;
}

int
owltmr_deltmo(struct owltmr_tmo *tmo)
{
        struct owltmr_tmo *iter;
        struct owltmr_tmo *prev = NULL;
        
        if (tmo == NULL) {
                owl_info("tmo is null");
                return 0;
        }

        owl_info("entry tmo:%p %s%s%s", tmo,
                 (tmo->flags & TMO_EXPIRED) ? "EXPIRED " : "",
                 (tmo->flags & TMO_PERIODIC) ? "PERIODIC " : "",
                 (tmo->flags & TMO_DELETED) ? "DELETED " : "");
        
        owl_assert(!(tmo->flags & TMO_DELETED));
                
        /* Timer is currently executing, will be removed when iter->cb()
         * returns in owltmr_poll(), now just mark as deleted
         */
        if (priv->flags & OWLTMR_FLAG_POLL) {
                tmo->flags |= TMO_DELETED;
                return 0;
        }
        
        for (iter = priv->head; iter; prev = iter, iter = iter->next) {
                if (iter != tmo)
                        continue;

                if (prev == NULL)
                        priv->head = priv->head->next;
                else
                        prev->next = tmo->next;

                owl_info("free tmo %p", tmo);
                free(tmo);
                return 0;
        }

        owl_err("tmo not found");
        return OWL_ERR_PARAM;
}

void
owltmr_modtmo(struct owltmr_tmo *tmo, int ms)
{
        tmo->tick = priv->tick;
        tmo->intvl = ms * priv->hz / 1000;
        tmo->flags &= ~TMO_EXPIRED;
}

uint32_t
owltmr_get_ms(void)
{
        return 1000 * priv->tick / priv->hz;
}

void
owltmr_tick(void)
{
        if (priv == NULL)
                return;
        
        priv->tick++;
}

void
owltmr_mdelay(int ms)
{
        uint32_t expire = priv->tick + ms * priv->hz / 1000;
        owl_dbg("busy waiting until:%d now:%d [ticks] hz:%d\n",
                expire, priv->tick, priv->hz);
        while (priv->tick < expire);
}

void
owltmr_poll(void)
{
        struct owltmr_tmo *prev = NULL;
        struct owltmr_tmo *iter;
        
        priv->flags |= OWLTMR_FLAG_POLL; /* used in owltmr_deltmo() */
        
        for (iter = priv->head; iter; iter = iter->next) {
                if (iter->flags & TMO_EXPIRED)
                        continue;
                if (iter->flags & TMO_DELETED) {
                        continue;
                }
                if (!owltmr_intvl_passed(iter->tick, priv->tick, iter->intvl))
                        continue;
                
                owl_dbg("invoking tmo:%p", iter);
                /* set flag before invoking cb() since owltmr_modtmo() might
                 * be invoked from the cb
                 */
                iter->flags |= TMO_EXPIRED; 
                iter->cb(iter, iter->ctx);

                if (iter->flags & TMO_PERIODIC)
                        owltmr_modtmo(iter, 1000 * iter->intvl / priv->hz);
        }

        priv->flags &= ~OWLTMR_FLAG_POLL;

        iter = priv->head;
        while (iter) {
                struct owltmr_tmo *tmo = iter;
                if (!(iter->flags & TMO_DELETED))
                        goto loop;

                if (prev == NULL)
                        priv->head = priv->head->next;
                else
                        prev->next = tmo->next;

                /* free this timer and update iter, keep prev as it
                 * is since it is still pointing to the previous
                 * valid tmo
                 */
                iter = iter->next;
                owl_info("free tmo %p", tmo);
                free(tmo);
                continue;
        loop:
                prev = iter;
                iter = iter->next;
        }
}

int
owltmr_intvl_passed(uint32_t old, uint32_t new, uint32_t diff)
{
        /* no wrap; timeout's scheduled with ms=0 should be invoked every time
           owltmr_poll() is called, therefore we check use >= in the
           comparisons below */
        if (new >= old && new - old >= diff)
                return 1;

        /* wrap */
        else if (new < old && (((uint32_t) (-1) - old) + new) >= diff)
                return 1;

        return 0;
}

uint32_t
owltmr_get_intvl(uint32_t old, uint32_t new)
{
        /* wrap */
        if (new < old)
                return ((uint32_t) (-1) - old) + new;
        
        /* no wrap */
        return new - old;
}
