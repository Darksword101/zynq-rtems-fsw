#include <rtems.h>
#include <stdio.h>
#include <stdlib.h>
#include "sb.h"
#include "hk.h"
#include "fsw_config.h"
#include "timebase.h"
#include "pi_demo.h"
#include "sensor_app.h"
#include "fdir_app.h"
#include "tlm_app.h"
#include "uart_link.h"

/* unused until Day 2 app starts; -Werror=unused-function otherwise rejects it */
static void __attribute__((unused)) fatal(const char *what, rtems_status_code sc)
{
    printf("FATAL: %s failed: %s\n", what, rtems_status_text(sc));
    exit(1);                                   /* triggers BSP reset → QEMU exits (-no-reboot) */
}

rtems_task Init(rtems_task_argument arg)
{
    (void)arg;
    printf("\n=== Zynq-7000 RTEMS FSW demonstrator ===\n");
    printf("RTEMS %s | tick = %u us | uptime %llu ns\n",
           rtems_get_version_string(),
           (unsigned)rtems_configuration_get_microseconds_per_tick(),
           (unsigned long long)now_ns());

    /* Day 2: sb_init(); hk_init(); start apps here */
    rtems_status_code sc;
    if ((sc = sb_init()) != RTEMS_SUCCESSFUL) fatal("sb_init", sc);
    if ((sc = hk_init()) != RTEMS_SUCCESSFUL) fatal("hk_init", sc);
    #ifdef FSW_RUN_PI_DEMO
        pi_demo_run();
    #endif
    if (uart_link_open() != 0) fatal("uart_link_open", RTEMS_IO_ERROR);
    if ((sc = sensor_app_start()) != RTEMS_SUCCESSFUL) fatal("sensor_app_start", sc);
    if ((sc = fdir_app_start())   != RTEMS_SUCCESSFUL) fatal("fdir_app_start", sc);
    if ((sc = tlm_app_start())    != RTEMS_SUCCESSFUL) fatal("tlm_app_start", sc);
    printf("FSW READY\n");
    rtems_task_delete(RTEMS_SELF);             /* Init's job is done; apps carry on */
}

#define CONFIGURE_INIT
#include "rtems_config.h"
