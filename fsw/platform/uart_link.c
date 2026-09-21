#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include "fsw_config.h"
#include "uart_link.h"

static int fd = -1;

int uart_link_open(void)
{
    fd = open(UART_LINK_DEVICE, O_RDWR);
    if (fd < 0) { perror("uart_link open"); return -1; }
    struct termios t;
    tcgetattr(fd, &t);
    cfmakeraw(&t);                          /* binary: no echo, no CR/LF translation, VMIN=1 VTIME=0 */
    cfsetispeed(&t, B115200); cfsetospeed(&t, B115200);
    tcsetattr(fd, TCSANOW, &t);
    return 0;
}
int uart_link_write(const void *buf, size_t len) { return fd < 0 ? -1 : (int)write(fd, buf, len); }
int uart_link_read(void *buf, size_t max)        { return fd < 0 ? -1 : (int)read(fd, buf, max); }   /* blocks for ≥1 byte */