#ifndef FSW_CONFIG_H
#define FSW_CONFIG_H

/* Periods (ms) — harmonic set: 10 / 100 / 1000 */
#define SENSOR_PERIOD_MS        10u
#define FDIR_PERIOD_MS         100u
#define TLM_PERIOD_DEFAULT_MS 1000u
#define TLM_PERIOD_MIN_MS      100u
#define TLM_PERIOD_MAX_MS     5000u

/* Priorities — rate monotonic assignment: shorter period → higher priority (smaller number) */
#define PRIO_SENSOR   20
#define PRIO_CMD      25
#define PRIO_FDIR     30
#define PRIO_TLM      40

/* Stacks */
#define STACK_APP     (16 * 1024)

/* Software bus */
#define SB_MAX_PAYLOAD       64
#define SB_MSG_MAX_SIZE      (16 + SB_MAX_PAYLOAD)    /* header fields + payload, see sb.h */
#define SB_MAX_ROUTES        16
#define SB_MAX_SUBS_PER_MSG   4
#define SB_PIPE_DEPTH_MAX    16

/* FDIR defaults */
#define FDIR_TEMP_LIMIT_C          60.0f
#define FDIR_TEMP_HYSTERESIS_C      5.0f
#define FDIR_TEMP_CONSECUTIVE       5
#define FDIR_MISSED_DEADLINE_LIMIT 10

/* Link */
#define UART_LINK_DEVICE "/dev/ttyS0"
#endif
