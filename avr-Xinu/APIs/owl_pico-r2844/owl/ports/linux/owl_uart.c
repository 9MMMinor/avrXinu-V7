#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

#include <owl/core/owl_err.h>
#include <owl/core/owl_debug.h>
#include <owl/drivers/owl_uart.h>

int owluart_init(void *uart,
                 uint32_t baudrate, enum uart_databits databits,
                 enum uart_parity parity, enum uart_stopbits stopbits,
                 int rtscts)
{
        int fd = *((int *) uart);
        int err;
        int spd;
        
        if (databits != 8 || stopbits != 1 || parity != UART_PARITY_NONE)
                return OWL_ERR_NOTSUPP;
        
        struct termios ti;
        if ((err = tcgetattr(fd, &ti)) < 0) {
                owl_err("Failed to tcgetattr:%d", err);
                return OWL_FAILURE;
        }
        
        cfmakeraw(&ti);
        ti.c_oflag = 0;
        if (rtscts)
                ti.c_cflag |= CRTSCTS;
        else
                ti.c_cflag &= ~(CRTSCTS);

        switch (baudrate) {
        case 300: spd = B300; break;
        case 1200: spd = B1200; break;
        case 2400: spd = B2400; break;
        case 4800: spd = B4800; break;
        case 9600: spd = B9600; break;
        case 19200: spd = B19200; break;
        case 38400: spd = B38400; break;
        case 57600: spd = B57600; break;
        case 115200: spd = B115200; break;
        case 230400: spd = B230400; break;
        case 460800: spd = B460800; break;
        case 500000: spd = B500000; break;
        case 576000: spd = B576000; break;
        case 921600: spd = B921600; break;
        case 1000000: spd = B1000000; break;
        case 1500000: spd = B1500000; break;
        case 2000000: spd = B2000000; break;
        case 2500000: spd = B2500000; break;
        case 3000000: spd = B3000000; break;
        case 3500000: spd = B3500000; break;
        case 4000000: spd = B4000000; break;
        default: spd = B57600; break;
        }

        
        if ((err = cfsetispeed(&ti, spd)) < 0) {
                owl_err("cfsetispeed:%d", err);
                return OWL_FAILURE;
        }
        if ((err = cfsetospeed(&ti, spd)) < 0) {
                owl_err("cfsetospeed:%d", err);
                return OWL_FAILURE;
        }
	
        if ((err = tcsetattr(fd, TCSANOW, &ti)) < 0) {
                owl_err("Failed to tcsetattr:%d", err);
                return OWL_FAILURE;
        }

        owl_info("uart configured with %d (%d) 8-N-1 rtscts:%d\n",
                 spd, baudrate, rtscts);
                 
        return fd;
}

int owluart_read(void *ctx, void *buf, int len)
{
        int fd = (int) ctx;
        int err;
        int pos = 0;

        owl_dbg("enter len:%d", len);
        
        while (pos < len) {
                if ((err = read(fd, buf + pos, len - pos)) < 0)
                        owl_err("read failed (fd:%d len:%d err:%d)",
                                fd, len, err);
                else
                        pos += err;
                
        }

        owl_hexdump("uart rx", buf, len);

        return len;
}

int owluart_write(void *ctx, const void *buf, int len)
{
        int fd = (int) ctx;
        int err;

        /* see wlp_api.c */
        if (buf == NULL)
                return 0;
        
        owl_hexdump("uart tx", buf, len);

        if ((err = write(fd, buf, len)) != len)
                owl_err("write failed (fd:%d len:%d res:%d)",
                        fd, len, err);
        
        return err;
}
