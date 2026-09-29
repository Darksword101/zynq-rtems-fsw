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

static uint16_t get_be16(const uint8_t *p) { return (uint16_t)((p[0] << 8) | p[1]); }
static uint64_t get_le64(const uint8_t *p)
{
    uint64_t v = 0;
    for (int i = 0; i < 8; i++) v |= (uint64_t)p[i] << (8 * i);
    return v;
}

int ccsds_parse_packet(const uint8_t *pkt, size_t len, ccsds_pkt_t *out)
{
    const size_t min_len = CCSDS_PRI_HDR_LEN + CCSDS_SEC_HDR_LEN + CCSDS_CRC_LEN;
    if (len < min_len) return -1;

    /* CCSDS length field is (bytes after primary header) - 1 */
    size_t pkt_len = (size_t)get_be16(pkt + 4) + 1 + CCSDS_PRI_HDR_LEN;
    if (pkt_len < min_len || pkt_len > len) return -1;

    uint16_t crc_rx = get_be16(pkt + pkt_len - CCSDS_CRC_LEN);
    uint16_t crc_calc = crc16_ccitt(pkt, pkt_len - CCSDS_CRC_LEN, 0xFFFF);
    if (crc_rx != crc_calc) return -2;

    out->apid        = get_be16(pkt + 0) & 0x7FF;
    out->seq         = get_be16(pkt + 2) & 0x3FFF;
    out->time_ns     = get_le64(pkt + 6);
    out->payload     = pkt + CCSDS_PRI_HDR_LEN + CCSDS_SEC_HDR_LEN;
    out->payload_len = (uint16_t)(pkt_len - min_len);
    return 0;
}