#include <string.h>

#include "crc16.h"
#include "minitest.h"

int main(void)
{
    const char *text = "123456789";

    CHECK_EQ_U(crc16_ccitt((const uint8_t *)text, strlen(text), 0xFFFFu), 0x29B1u);

    MT_REPORT();
}