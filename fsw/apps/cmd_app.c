#include <stddef.h>
#include <string.h>
#include "fsw_config.h"
#include "fsw_types.h"
#include "cmd_app.h"
#include "msg_ids.h"
#include "crc16.h"
#include "hk.h"
#include "uart_link.h"
#include "sb.h"
#include "fdir_app.h"

typedef enum {HUNT1, HUNT2, OPCODE, LEN, PAYLOAD, CRC1, CRC2} pstate_t;

typedef struct {
    pstate_t st;
    uint8_t op, len, idx;
    uint8_t payload[32];
    uint16_t crc_rx;

} parser_t;

typedef struct {
    uint8_t opcode;
    uint8_t expected_len;
    uint16_t sb_msg_id;
} cmd_def_t;

static const cmd_def_t CMDS[] = {
    { CMD_NOOP,           0, 0 },
    { CMD_RESET_COUNTERS, 0, MSG_ID_CMD_RESET_CTRS },
    { CMD_SET_TLM_PERIOD, 2, MSG_ID_CMD_SET_TLM_PER },
    { CMD_SET_TEMP_LIMIT, 4, MSG_ID_CMD_SET_TEMP_LIM },
    { CMD_ENTER_SAFE,     0, MSG_ID_CMD_ENTER_SAFE },
    { CMD_EXIT_SAFE,      0, MSG_ID_CMD_EXIT_SAFE },
    { CMD_INJECT_FAULT,   4, MSG_ID_CMD_INJECT_FAULT },
};

static void dispatch(const parser_t *p)
{
    uint8_t hdr[2] = {p->op, p->len};
    uint16_t crc = crc16_ccitt(hdr, 2, 0xFFFF);
    crc = crc16_ccitt(p->payload, p->len, crc);
    if (crc != p->crc_rx) {
        hk_count_cmd(false);
        publish_event("CMD: bad CRC");
        return;
    }

    for (size_t i = 0; i < sizeof CMDS / sizeof CMDS[0]; ++i){
        if (CMDS[i].opcode != p->op) continue;
        if (CMDS[i].expected_len != p->len) break;
        hk_count_cmd(true);
        if (p->op == CMD_RESET_COUNTERS) hk_reset_counters();
        if (CMDS[i].sb_msg_id) sb_publish(CMDS[i].sb_msg_id, p->payload, p->len);
        return;
    }

    hk_count_cmd(false);
    publish_event("CMD: unknown opcode/len");

}

static void feed(parser_t *p, uint8_t b)
{
    switch (p->st){
        case HUNT1: p->st = (b == 0xEB) ? HUNT2 : HUNT1;
            break;
        case HUNT2: p->st = (b == 0x90) ? OPCODE : (b == 0xEB ? HUNT2 : HUNT1);
            break;
        case OPCODE: p->op = b; p->st = LEN;
            break;
        case LEN: if (b > sizeof p->payload) { p->st = HUNT1; break; } p->len = b; p->idx = 0; p->st = b ? PAYLOAD : CRC1; 
            break;
        case PAYLOAD: p->payload[p->idx++] = b; if (p->idx == p->len) p->st = CRC1;
            break;
        case CRC1: p->crc_rx = (uint16_t)(b << 8); p->st = CRC2;
            break;
        case CRC2: p->crc_rx |= b; dispatch(p); p->st = HUNT1;
            break;
        default:
            break;

    }
}

static rtems_task cmd_task(rtems_task_argument arg)
{
    (void)arg;
    parser_t p = {.st = HUNT1};
    uint8_t buf[64];

    for(;;) {
        int n = uart_link_read(buf, sizeof buf);
        for (int i = 0; i < n; ++i){
            feed(&p, buf[i]);
        }
    }
}

rtems_status_code cmd_app_start(void)
{
    rtems_id tid;
    rtems_status_code sc = rtems_task_create(
        rtems_build_name('C', 'M', 'D', 'T'),
        PRIO_CMD,
        STACK_APP,
        RTEMS_DEFAULT_MODES,
        RTEMS_FLOATING_POINT,
        &tid
    );
    if (sc != RTEMS_SUCCESSFUL) return sc;

    return rtems_task_start(tid, cmd_task, 0);
}