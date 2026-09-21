#ifndef UART_LINK_H
#define UART_LINK_H
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include "fsw_config.h"

int uart_link_open(void);

int uart_link_write(const void *buf, size_t len);

int uart_link_read(void *buf, size_t max);

#endif /* UART_LINK_H */    