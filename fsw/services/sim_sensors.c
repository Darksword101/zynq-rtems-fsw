#include <math.h>
#include "sim_sensors.h"

static uint32_t lcg; static uint32_t seq; static volatile float temp_offset;
static float noise(void) {lcg = lcg * 1664525u + 101304223u; return ((float)(lcg >> 8) / 16777216.0f - 
    0.5f);}

void sim_sensors_init(uint32_t seed) {lcg = seed; seq = 0; temp_offset = 0.0f;}
void sim_sensors_inject_temp_offset(float d) {temp_offset += d;}

void sim_sensors_read(sensor_sample_t *s, uint64_t t_ns)
{
    const float offset = temp_offset;
    float t = (float)t_ns * 1e-9f;
    s->timestamp_ns = t_ns; s->sequence = seq++;
    for(int i = 0; i < 3; i++)
    {
        s->gyro_dps[i] = 5.0f * sinf(0.5f * t + (float)i) + 0.05f * noise();
        s->accel_g[i] = (i == 2 ? 1.0f : 0.0f) + 0.01f * noise();
    }

    s->temp_c = 25.0f + 3.0f * sinf(0.01f * t) + 0.1f * noise() + offset;
}