#ifndef CCSDS_H
#define CCSDS_H

#include <stdint.h>
#include <stddef.h>

#define CCSDS_ASM_LEN 4
#define CCSDS_PRI_HDR_LEN 6
#define CCSDS_SEC_HDR_LEN  8
#define CCSDS_CRC_LEN      2
#define CCSDS_MAX_PAYLOAD  64
#define CCSDS_MAX_FRAME  (CCSDS_ASM_LEN + CCSDS_PRI_HDR_LEN + CCSDS_SEC_HDR_LEN + CCSDS_MAX_PAYLOAD + CCSDS_CRC_LEN)

typedef struct {uint16_t apid; 
    uint16_t seq;
    uint64_t time_ns; 
    const uint8_t *payload; 
    uint16_t payload_len;
} ccsds_pkt_t;

/*Returns frame length written (incl. ASM), or 0 on error*/
size_t ccsds_build_frame(uint8_t *buf, size_t cap, uint16_t apid, uint16_t seq, uint64_t time_ns, 
    const void *payload, uint16_t payload_len);
/*parses one packet (without ASM). Returns 0 on success, negative on badl length/CRC*/
int ccsds_parse_packet(const uint8_t *pkt, size_t len, ccsds_pkt_t *out); 

#endif // CCSDS_H