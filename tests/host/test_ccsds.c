#include <stdint.h>
#include <string.h>

#include "ccsds.h"
#include "minitest.h"

int main(void)
{
    static const uint8_t payload[] = {0x10, 0x20, 0x30, 0x40};
    uint8_t frame[CCSDS_MAX_FRAME];
    ccsds_pkt_t packet;
    const uint16_t apid = 0x321;
    const uint16_t seq = 0x1234;
    const uint64_t time_ns = 0x0102030405060708ULL;
    size_t frame_len = ccsds_build_frame(frame, sizeof frame, apid, seq, time_ns,
                                         payload, sizeof payload);

    CHECK(frame_len != 0);
    CHECK(ccsds_parse_packet(frame + CCSDS_ASM_LEN,
                             frame_len - CCSDS_ASM_LEN, &packet) == 0);
    CHECK_EQ_U(packet.apid, apid);
    CHECK_EQ_U(packet.seq, seq);
    CHECK(packet.time_ns == time_ns);
    CHECK_EQ_U(packet.payload_len, sizeof payload);
    CHECK(memcmp(packet.payload, payload, sizeof payload) == 0);

    frame[CCSDS_ASM_LEN + CCSDS_PRI_HDR_LEN + CCSDS_SEC_HDR_LEN] ^= 0x01u;
    CHECK(ccsds_parse_packet(frame + CCSDS_ASM_LEN,
                             frame_len - CCSDS_ASM_LEN, &packet) < 0);

    CHECK(ccsds_build_frame(frame, sizeof frame, apid, seq, time_ns, payload,
                            CCSDS_MAX_PAYLOAD + 1u) == 0);

    MT_REPORT();
}