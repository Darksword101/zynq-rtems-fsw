#include <rtems.h>
#include "timebase.h"
uint64_t now_ns(void) { return rtems_clock_get_uptime_nanoseconds(); }
