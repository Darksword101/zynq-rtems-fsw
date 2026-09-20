/* hk.c essentials*/
#include "hk.h"
#include "fsw_config.h"
#include "fsw_types.h"
#include <string.h>

static hk_table_t table;
static rtems_id mutex;

rtems_status_code hk_init(void)
{
    memset(&table, 0, sizeof table);
    table.temp_limit_c = FDIR_TEMP_LIMIT_C;
    table.tlm.tlm_period_ms = TLM_PERIOD_DEFAULT_MS;
    /* Binary semaphore + priority discipline + priority inheritance = a real mutex that fixes inversion*/

    return rtems_semaphore_create(rtems_build_name('H','K','M','T'), 1,
     RTEMS_BINARY_SEMAPHORE | RTEMS_PRIORITY | RTEMS_INHERIT_PRIORITY, 0, &mutex);
    
}

void hk_lock(void) {rtems_semaphore_obtain(mutex, RTEMS_WAIT, RTEMS_NO_TIMEOUT); }
void hk_unlock(void) {rtems_semaphore_release(mutex); }

void hk_snapshot(hk_table_t *out) {hk_lock(); *out = table; hk_unlock();}

void hk_update_sensor(const sensor_sample_t * s)
{
    hk_lock();
    memcpy(table.tlm.gyro_dps, s->gyro_dps, sizeof s->gyro_dps);
    memcpy(table.tlm.accel_g, s->accel_g, sizeof s->accel_g);
    table.tlm.temp_c = s->temp_c;
    table.tlm.sensor_sample_count++;
    hk_unlock();

}
/* hk_set_mode, hk_count_cmd, hk_reset_counters: same lock/modify/unlock shape*/