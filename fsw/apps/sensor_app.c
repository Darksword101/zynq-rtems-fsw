#include <rtems.h>
#include <stdio.h>
#include "fsw_config.h"
#include "msg_ids.h"
#include "sb.h"
#include "hk.h"
#include "sim_sensors.h"
#include "timebase.h"

static sb_pipe_t cmd_pipe;   /*receives MSG_ID_CMD_INJECT_FAULT*/
static rtems_id period_id;
rtems_id sensor_period_id(void) {return period_id;} /*fdir_app reads its statistics*/

static void handle_commands(void)
{
    sb_msg_t m;
    while(sb_receive(&cmd_pipe, &m, 0) == RTEMS_SUCCESSFUL){
        if (m.msg_id == MSG_ID_CMD_INJECT_FAULT && m.length == sizeof(float))
        {
            float d; 
            memcpy(&d, m.payload, sizeof(d));
            sim_sensors_inject_temp_offset(d);
        }
    }
}

static rtems_task sensor_task(rtems_task_argument arg)
{
    (void)arg;
    rtems_rate_monotonic_create(rtems_build_name('S', 'E', 'N', 'P'), &period_id);
    sim_sensors_init(0x5EED1234u);
    const rtems_interval period_ticks = RTEMS_MILLISECONDS_TO_TICKS(SENSOR_PERIOD_MS);
}