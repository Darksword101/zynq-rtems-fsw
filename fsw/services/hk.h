#ifndef HK_H
#define HK_H

/* hk.h */
#include <rtems.h>
#include <stdbool.h>
#include "fsw_types.h"
typedef struct {
    hk_tlm_t tlm;                 /* the downlinked view */
    float    temp_limit_c;        /* runtime config living alongside */
} hk_table_t;

rtems_status_code hk_init(void);
void hk_lock(void);
void hk_unlock(void);
void hk_snapshot(hk_table_t *out);                     /* copy under lock */
void hk_update_sensor(const sensor_sample_t *s);       /* called at 100 Hz */
void hk_set_mode(fsw_mode_t mode, uint8_t fault_flags);
void hk_count_cmd(bool accepted);
void hk_reset_counters(void);
hk_table_t *hk_locked_table(void);                     /* only valid between hk_lock/hk_unlock */

#endif