#define OWL_TRACE OWL_TRACE_INFO

#include <avr/io.h>
#include <util/delay_basic.h>

#ifdef __avr8__
# include <avr/io.h>
# define F_CPU 32000000UL
# include <util/delay.h>
# include "ioport.h"
# include "board.h"
#endif /* __avr8__ */

#include <board.h>
#include <sysclk.h>

#include <owl/core/owl_debug.h>
#include <owl/core/owl_err.h>
#include <owl/core/owl_timer.h>
#include <owl/drivers/owl_uart.h>

static irqflags_t iflags;

int
owluart_init(void *uart,
             uint32_t baudrate, enum uart_databits databits,
             enum uart_parity parity, enum uart_stopbits stopbits,
             int rtscts)
{
        USART_t *port = uart;
        uint8_t ctrlc = 0;
        uint8_t ctrlb = 0;
        uint16_t bsel;

#if BOARD == XPLAIN
        /* usb-to-serial on xplain seems broken for other configurations */
        if (port == &USARTC0) {
                if (baudrate != 9600 ||
                    databits != 8 ||
                    stopbits != 1 ||
                    parity != UART_PARITY_NONE ||
                    rtscts)
                        return OWL_ERR_NOTSUPP;
        }
#endif
        
        if (rtscts)
                return OWL_ERR_NOTSUPP;

        switch(databits) {
        case UART_DATABITS_5:
                ctrlc |= USART_CHSIZE_5BIT_gc;
                break;
        case UART_DATABITS_6:
                ctrlc |= USART_CHSIZE_6BIT_gc;
                break;
        case UART_DATABITS_7:
                ctrlc |= USART_CHSIZE_7BIT_gc;
                break;
        case UART_DATABITS_8:
                ctrlc |= USART_CHSIZE_8BIT_gc;
                break;
        case UART_DATABITS_9:
                ctrlc |= USART_CHSIZE_9BIT_gc;
                break;
        };
            
        switch (parity) {
        case UART_PARITY_NONE:
                ctrlc |= USART_PMODE_DISABLED_gc;
                break;
        case UART_PARITY_EVEN:
                ctrlc |= USART_PMODE_EVEN_gc;
                break;
        case UART_PARITY_ODD:
                ctrlc |= USART_PMODE_ODD_gc;
                break;
        default:
                return OWL_ERR_NOTSUPP;
        }
        
        switch (stopbits) {
        case UART_STOPBITS_1:
                break;
        case UART_STOPBITS_2:
                ctrlc |= USART_SBMODE_bm;
                break;
        };
 
        ctrlb = USART_TXEN_bm | USART_RXEN_bm;
        
        if (sysclk_get_per_hz() < (16UL * baudrate)) {
                /* need double transmission speed */
                ctrlb |= USART_CLK2X_bm;
                bsel = sysclk_get_per_hz() / (16UL * baudrate) - 1;
                
        } else {
                bsel = sysclk_get_per_hz() / (16UL * baudrate) - 1;
        }

        port->CTRLB = 0; /* disable transceiver while confiuring */
        port->CTRLC = ctrlc;
        port->BAUDCTRLA = bsel & 0xff;
        port->BAUDCTRLB = bsel >> 8;
        port->CTRLB = ctrlb;
        return 0;
}

void
owluart_outb(void *uart, int c)
{
        USART_t *port = uart;
        while (!(port->STATUS & USART_DREIF_bm));
        port->DATA = c;
        
#if BOARD == XPLAIN
        /* usb-to-serial hacks */
        if (port == &USARTC0) {
                while (!(port->STATUS & USART_TXCIF_bm));
                port->STATUS |= USART_TXCIF_bm;
                _delay_loop_2(1000);
        }
#endif
}

int
owluart_inb(void *uart, unsigned int timeout)
{
        unsigned int ms = owltmr_get_ms();
        USART_t *port = uart;

        if (timeout > 0)
                return OWL_ERR_NOTSUPP;
        
        for (;;) {
                if (owluart_poll(port)) {
                        int i = port->DATA;
                        owl_dbg("0x%x %c", i, (char) i); 
                        return i;
                }
                
                if (timeout > 0 &&
                    owltmr_intvl_passed(ms, owltmr_get_ms(), timeout)) {
                        owl_info("got timeout (%d)", timeout);
                        return OWL_ERR_TIMEOUT;
                }
        }
}

int
owluart_read(void *uart, void *buf, int len)
{
        int i;
        USART_t *port = uart;
        char *in = buf;
                
        /* We disable interrupts here to make sure that we can read all the
         * expected data without risking buffer overruns.
         *
         * This should really be handled by a interrupt-driven uart driver and
         * a ringbuf. 
         *
         */
        for (i = 0; i < len; i++) {
                while (!(port->STATUS & USART_RXCIF_bm));
                in[i] = port->DATA;
        }

        return len;
}

int
owluart_write(void *uart, const void *buf, int len)
{
        int i;
        const char *out = buf;

        /* see wlp_api.c */
        if (buf == NULL) {
                if (len)
                        iflags = cpu_irq_save();
                else
                        cpu_irq_restore(iflags);

                return 0;
        }

        for (i = 0; i < len; i++)
                owluart_outb(uart, out[i]);
        
        return len;
}

int
owluart_poll(void *uart)
{
        USART_t *port = uart;
        owl_dbg("port:%x STATUS:%x CTRLA:%x CTRLB:%x CTRLC:%x",
                port, port->STATUS, port->CTRLA, port->CTRLB, port->CTRLC);
        return port->STATUS & USART_RXCIF_bm;
}

const char *
owluart_parity(enum uart_parity parity)
{
        switch (parity) {
        case UART_PARITY_NONE:
                return "N";
        case UART_PARITY_EVEN:
                return "E";
        case UART_PARITY_ODD:
                return "O";
        case UART_PARITY_MARK:
                return "M";
        case UART_PARITY_SPACE:
                return "S";
        };

        owl_assert(0);
        owl_err("invalid parity:%d", parity);
        return "";
}
