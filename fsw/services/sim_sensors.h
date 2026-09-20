#include "fsw_types.h"
void sim_sensors_init(uint32_t seed);
void sim_sensors_read(sensor_sample_t *out, uint64_t t_ns);
void sim_sensors_inject_temp_offset(float delta_c);