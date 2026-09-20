#ifndef FSW_TYPES_H
#define FSW_TYPES_H
#include <stdint.h>

typedef enum {FSW_MODE_BOOT = 0, FSW_MODE_NOMINAL = 1, FSW_MODE_SAFE = 2} fsw_mode_t;

#define FAULT_TEMP_HIGH (1u << 0)
#define FAULT_DEADLINE_MISS (1u << 1)
#define FAULT_CMD_SAFE (1u << 2)

typedef struct {
    uint64_t timestamp_ns;
    uint32_t sequence;
    float gyro_dps[3];
    float accel_g[3];
    float temp_c;
} sensor_sample_t;  /*travels on the software bus  */

typedef struct __attribute__((packed)) {
    uint32_t uptime_ms;
    uint8_t mode;
    uint8_t fault_flags;
    uint16_t cmd_accept_count;
    uint16_t cmd_reject_count;
    uint16_t sensor_missed_deadlines;
    uint32_t sensor_sample_count;
    uint32_t sb_drop_count;
    float gyro_dps[3];
    float accel_g[3];
    float temp_c;
    uint32_t sensor_max_wall_us;
    uint16_t tlm_period_ms;
    uint16_t reserved;

} hk_tlm_t;  /*travels to the ground: 56 bytes*/

_Static_assert(sizeof(hk_tlm_t) == 56, "hk_tlm_t layout changed - update gs/packets.py and docs/icd.md");

#endif // FSW_TYPES_H