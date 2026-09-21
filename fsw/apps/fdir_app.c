#include <string.h>
#include "sb.h"
#include "hk.h"
#include "fdir_rules.h"
#include "sensor_app.h"  /*sensor_period_id()*/
#include "msg_ids.h"
#include "fdir_app.h"

static sb_pipe_t sensor_pipe, cmd_pipe;

static rtems_task fdir_task(rtems_task_argument arg)
{
    (void) arg;
    rtems_id period;
    rtems_rate_monotonic_create(rtems_build_name('F', 'D', 'I', 'P'), &period);
    fdir_state_t st = {.mode = FSW_MODE_BOOT};
    fdir_rules_t rules = {FDIR_TEMP_LIMIT_C, FDIR_TEMP_HYSTERESIS_C, FDIR_TEMP_CONSECUTIVE, FDIR_MISSED_DEADLINE_LIMIT};
    sensor_sample_t last = {0};

    for(; ;)
    {
        rtems_rate_monotonic_period(period, RTEMS_MILLISECONDS_TO_TICKS(FDIR_PERIOD_MS));
        /*1. commands*/
        sb_msg_t m;
        while(sb_receive(&cmd_pipe, &m, 0) == RTEMS_SUCCESSFUL){
            switch (m.msg_id)
            {
                case MSG_ID_CMD_ENTER_SAFE:
                    st.mode = FSW_MODE_SAFE; 
                    st.fault_flags |= FAULT_CMD_SAFE;
                    publish_event("CMD: ENTER_SAFE");
                    break;
                case MSG_ID_CMD_EXIT_SAFE:
                    publish_event(fdir_request_exit_safe(&rules, &st, last.temp_c) ? "CMD: EXIT_SAFE ok": "CMD: EXIT_SAFE rejected");
                    break;
                case MSG_ID_CMD_SET_TEMP_LIM:
                    memcpy(&rules.temp_limit_c, m.payload, sizeof(float));
                    break;
                default:
                    break;
            }
        }

        /*2. drain up to 16 sensor samples that arrived in the last 100 ms (burst-tolerant)*/
        hk_table_t hk;
        hk_snapshot(&hk);

        while(sb_receive(&sensor_pipe, &m, 0) == RTEMS_SUCCESSFUL){
            memcpy(&last, m.payload, sizeof last);
            fdir_event_t ev = fdir_evaluate(&rules, &st, &last, hk.tlm.sensor_missed_deadlines);
            if (ev == FDIR_EVT_TEMP_HIGH_SAFE) publish_event("FDIR: TEMP_HIGH -> SAFE");
            if (ev == FDIR_EVT_DEADLINE_SAFE) publish_event("FDIR: DEADLINE_MISS -> SAFE");
            if (ev == FDIR_EVT_ENTER_NOMINAL) publish_event("FDIR: BOOT -> NOMINAL");
        }
        /*3. deadline/jitter statistics from the kernel*/
        rtems_rate_monotonic_period_statistics ps;
        if(rtems_rate_monotonic_get_statistics(sensor_period_id(), &ps) == RTEMS_SUCCESSFUL)
        {
            uint32_t max_us = (uint32_t)(ps.max_wall_time.tv_sec * 1000000 + ps.max_wall_time.tv_nsec / 1000);
            hk_lock();
            hk_locked_table()->tlm.sensor_max_wall_us = max_us;
            hk_unlock();
        }
        hk_set_mode(st.mode, st.fault_flags);
    }
}

void publish_event(const char *text) 
{
    sb_publish(MSG_ID_EVENT, text, (uint16_t)(strlen(text) + 1)); printf("[EVT] %s\n", text); 
}

rtems_status_code fdir_app_start(void)
{
    rtems_status_code sc;
    rtems_id tid;
    sc = sb_create_pipe("FSEN", 16, &sensor_pipe);
    if (sc != RTEMS_SUCCESSFUL) {
        return sc;
    }

    sc = sb_subscribe(MSG_ID_SENSOR_DATA, &sensor_pipe);
    if ( sc != RTEMS_SUCCESSFUL) {
        return sc;
    }

    sc = sb_create_pipe("FCMD", 4, &cmd_pipe);
    if(sc != RTEMS_SUCCESSFUL){
        return sc;
    }

    sc = sb_subscribe(MSG_ID_CMD_SET_TEMP_LIM, &cmd_pipe);
    if (sc != RTEMS_SUCCESSFUL) {
        return sc;
    }

    sc = sb_subscribe(MSG_ID_CMD_ENTER_SAFE, &cmd_pipe);
    if (sc != RTEMS_SUCCESSFUL) {
        return sc;
    }

    sc = sb_subscribe(MSG_ID_CMD_EXIT_SAFE, &cmd_pipe);
    if (sc != RTEMS_SUCCESSFUL) {
        return sc;
    }

    sc = rtems_task_create(
        rtems_build_name('F', 'D', 'I', 'R'),
        PRIO_FDIR,
        STACK_APP,
        RTEMS_DEFAULT_MODES,
        RTEMS_FLOATING_POINT,
        &tid);

    if (sc != RTEMS_SUCCESSFUL) {
        return sc;
    }

    return rtems_task_start(tid, fdir_task, 0);

}