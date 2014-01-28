#ifndef OWL_UART_H
#define OWL_UART_H

#include <stdint.h>

enum uart_parity {
	UART_PARITY_NONE = 0,
	UART_PARITY_EVEN,
	UART_PARITY_ODD,
	UART_PARITY_MARK,
	UART_PARITY_SPACE
};

enum uart_stopbits {
	UART_STOPBITS_1 = 1,
	UART_STOPBITS_2
};

enum uart_databits {
	UART_DATABITS_5 = 5,
	UART_DATABITS_6,
	UART_DATABITS_7,
	UART_DATABITS_8,
	UART_DATABITS_9
};

#define UART_BAUDRATE_300       300UL
#define UART_BAUDRATE_1200      1200UL
#define UART_BAUDRATE_2400      2400UL
#define UART_BAUDRATE_4800      4800UL
#define UART_BAUDRATE_9600      9600UL
#define UART_BAUDRATE_19200     19200UL
#define UART_BAUDRATE_38400     38400UL
#define UART_BAUDRATE_57600     57600UL
#define UART_BAUDRATE_115200    115200UL
#define UART_BAUDRATE_230400    230400UL
#define UART_BAUDRATE_460800    460800UL
#define UART_BAUDRATE_500000    500000UL
#define UART_BAUDRATE_576000    576000
#define UART_BAUDRATE_921600    921600
#define UART_BAUDRATE_1000000   1000000
#define UART_BAUDRATE_1500000   1500000
#define UART_BAUDRATE_2000000   2000000
#define UART_BAUDRATE_2500000   2500000
#define UART_BAUDRATE_3000000   3000000
#define UART_BAUDRATE_3500000   3500000
#define UART_BAUDRATE_4000000   4000000

int owluart_init(void *uart,
				 uint32_t baudrate, enum uart_databits databits,
				 enum uart_parity parity, enum uart_stopbits stopbits,
				 int rtscts);
void owluart_outb(void *uart, int c);
int owluart_inb(void *uart, unsigned int timeout);
int owluart_poll(void *uart);

int owluart_read(void *uart, uint8_t *buf, int len);
int owluart_write(void *uart, const uint8_t *buf, int len);
const char *owluart_parity(enum uart_parity parity);

int owluart_ioctl(void *uart, int req, void *param);
#define OWLUART_IOC_R 0x00
#define OWLUART_IOC_W 0x80

/* param: int *; set size of input buffer */
#define OWLUART_IOC_RINGBUF (OWLUART_IOC_W | 0x0) 

/* param: int *; set mode (rs232, rs485, etc) */
#define OWLUART_IOC_MODE (OWLUART_IOC_W | 0x1)
#define OWLUART_MODE_RS232 0
#define OWLUART_MODE_RS485 1


#endif
