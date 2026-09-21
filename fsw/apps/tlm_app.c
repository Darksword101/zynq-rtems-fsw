#define TLM_BUFFERS 4
#include <stdint.h>
#include <string.h>
#include "ccsds.h"
#include "rtems.h"
#include "fsw_config.h"
#include "sb.h"
#include "hk.h"
#include "msg_ids.h"
#include "timebase.h"
#include "uart_link.h"


static uint8_t tlm_pool[TLM_BUFFERS][CCSDS_MAX_FRAME] __attribute__((aligned(8)));
static rtems_id part_id;
static volatile uint32_t tlm_period_ms = TLM_PERIOD_DEFAULT_MS;

static uint16_t seq_hk, seq_evt;
static sb_pipe_t cmd_pipe, evt_pipe;

static void send_frame(uint16_t apid, uint16_t *seq, const void *payload, uint16_t len)
{
    void *buf;
    if (rtems_partition_get_buffer(part_id, &buf) != RTEMS_SUCCESSFUL)
    {
        return; /*pool exhausted -> skip, count*/
    }
    size_t n = ccsds_build_frame(buf, CCSDS_MAX_FRAME, apid, (*seq)++, now_ns(), payload, len);
    if(n) uart_link_write(buf, n);
    rtems_partition_return_buffer(part_id, buf);
}

static rtems_task tlm_task(rtems_task_argument arg)
{
    (void) arg;
    rtems_id period;
    rtems_rate_monotonic_create(rtems_build_name('T', 'L', 'M', 'P'), &period);

    for(;;)
    {
        rtems_rate_monotonic_period(period, RTEMS_MILLISECONDS_TO_TICKS(tlm_period_ms));

        sb_msg_t m;
        while (sb_receive(&cmd_pipe, &m, 0) == RTEMS_SUCCESSFUL){
            if (m.msg_id == MSG_ID_CMD_SET_TLM_PER && m.length == 2) {
                uint16_t p; memcpy(&p, m.payload, 2);
                if (p >= TLM_PERIOD_MIN_MS && p <= TLM_PERIOD_MAX_MS) {
                    tlm_period_ms = p;
                    rtems_rate_monotonic_cancel(period);        /* restart with the new period on next call */
                    hk_lock(); hk_locked_table()->tlm.tlm_period_ms = p; hk_unlock();
                }
            }
        }

        while (sb_receive(&evt_pipe, &m, 0) == RTEMS_SUCCESSFUL)
        {
            send_frame(APID_EVENT, &seq_evt, m.payload, m.length);    /* handle event message */
        }
        
        hk_table_t hk;
        hk_snapshot(&hk);
        hk.tlm.uptime_ms = (uint32_t)(now_ns() / 1000000ull);
        hk.tlm.sb_drop_count = sb_drop_count();
        send_frame(APID_HK, &seq_hk, &hk.tlm, sizeof hk.tlm);
    }
}

rtems_status_code tlm_app_start(void)
{
    rtems_status_code sc = rtems_partition_create(rtems_build_name('T', 'L', 'M', 'B'), tlm_pool, sizeof tlm_pool,
        CCSDS_MAX_FRAME, RTEMS_DEFAULT_ATTRIBUTES, &part_id);
    if (sc != RTEMS_SUCCESSFUL) return sc;

    sc = sb_create_pipe("TEVT", 8, &evt_pipe);
    if (sc != RTEMS_SUCCESSFUL) return sc;

    sc = sb_subscribe(MSG_ID_EVENT, &evt_pipe);
    if (sc != RTEMS_SUCCESSFUL) return sc;

    sc = sb_create_pipe("TCMD", 4, &cmd_pipe);
    if (sc != RTEMS_SUCCESSFUL) return sc;

    sc = sb_subscribe(MSG_ID_CMD_SET_TLM_PER, &cmd_pipe);
    if (sc != RTEMS_SUCCESSFUL) return sc;

    rtems_id tid;
    sc = rtems_task_create(
        rtems_build_name('T', 'L', 'M', 'T'),
        PRIO_TLM,
        STACK_APP,
        RTEMS_DEFAULT_MODES,
        RTEMS_FLOATING_POINT,
        &tid
    );

    if (sc != RTEMS_SUCCESSFUL) return sc;

    return rtems_task_start(tid, tlm_task, 0);
}
