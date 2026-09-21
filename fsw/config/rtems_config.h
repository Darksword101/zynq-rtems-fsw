#ifndef RTEMS_CONFIG_H
#define RTEMS_CONFIG_H

#include "fsw_config.h"      /* SB_PIPE_DEPTH_MAX, SB_MSG_MAX_SIZE, ... */

/* Drivers */
#define CONFIGURE_APPLICATION_NEEDS_CLOCK_DRIVER
#define CONFIGURE_APPLICATION_NEEDS_CONSOLE_DRIVER

/* 1 ms tick: 10 ms sensor period = 10 ticks, jitter resolution 1 ms */
#define CONFIGURE_MICROSECONDS_PER_TICK        1000

/* Static object budget — count them, don't guess. Over-budget = rtems_*_create fails at init. */
#define CONFIGURE_MAXIMUM_TASKS                10   /* Init + 4 apps + 3 pi_demo + isr_demo + spare */
#define CONFIGURE_MAXIMUM_SEMAPHORES           10   /* hk mutex, sb mutex, isr_demo sem, pi_demo mutex, termios */
#define CONFIGURE_MAXIMUM_MESSAGE_QUEUES        6   /* FDIR, TLM, CMD_SENSOR, CMD_FDIR, CMD_TLM + spare */
#define CONFIGURE_MAXIMUM_PARTITIONS            2   /* tlm packet buffers */
#define CONFIGURE_MAXIMUM_PERIODS               4   /* sensor, fdir, tlm + spare */
#define CONFIGURE_MAXIMUM_TIMERS                4   /* isr_demo */
#define CONFIGURE_MAXIMUM_FILE_DESCRIPTORS      8   /* stdin/out/err + /dev/ttyS0 + spare */

/* Message queue storage is carved out of the workspace at boot */
#define CONFIGURE_MESSAGE_BUFFER_MEMORY \
  ( CONFIGURE_MESSAGE_BUFFERS_FOR_QUEUE(SB_PIPE_DEPTH_MAX, SB_MSG_MAX_SIZE) * CONFIGURE_MAXIMUM_MESSAGE_QUEUES )

/* Catch stack overflows in development builds — a classic flight-software bug class */
#define CONFIGURE_STACK_CHECKER_ENABLED

/* Init task */
#define CONFIGURE_RTEMS_INIT_TASKS_TABLE
#define CONFIGURE_INIT_TASK_STACK_SIZE         (32 * 1024)
#define CONFIGURE_INIT_TASK_PRIORITY           1
#define CONFIGURE_INIT_TASK_ATTRIBUTES         RTEMS_FLOATING_POINT
#define CONFIGURE_INIT_TASK_INITIAL_MODES      RTEMS_DEFAULT_MODES
#define CONFIGURE_EXTRA_TASK_STACKS  (3 * STACK_APP)

#include <rtems/confdefs.h>
#endif
