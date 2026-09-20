#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "ccsds.h"
#include "crc16.h"
static const uint8_t ASM[4] = {0x1A, 0xCF, 0xFC, 0x1D};
static void put_be16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v >> 8);
    p[1] = (uint8_t)v;
}

static void put_le64(uint8_t *p, uint64_t v)
{
    for (int i = 0; i < 8; i++) {
        p[i] = (uint8_t)(v >> (8 * i));
    }
}

size_t ccsds_build_frame(uint8_t *buf, size_t cap, uint16_t apid, uint16_t seq, uint64_t time_ns, 
    const void *payload, uint16_t payload_len)
{
    size_t pkt_len = CCSDS_PRI_HDR_LEN + CCSDS_SEC_HDR_LEN + payload_len + CCSDS_CRC_LEN;
    if (payload_len > CCSDS_MAX_PAYLOAD || cap < CCSDS_ASM_LEN + pkt_len) return 0;

    memcpy(buf, ASM, 4);
    uint8_t *p = buf + 4;
    put_be16(p + 0, (uint16_t)((1u << 11) | (apid & 0x7FF)));           /* ver 0, type TM, sec-hdr flag 1 */
    put_be16(p + 2, (uint16_t)((3u << 14) | (seq & 0x3FFF)));           /* unsegmented, seq count */
    put_be16(p + 4, (uint16_t)(pkt_len - CCSDS_PRI_HDR_LEN - 1));        /* CCSDS "length − 1" convention */
    put_le64(p + 6, time_ns);
    memcpy(p + 14, payload, payload_len);
    put_be16(p + 14 + payload_len, crc16_ccitt(p, pkt_len - CCSDS_CRC_LEN, 0xFFFF));
    return CCSDS_ASM_LEN + pkt_len;
}