#ifndef SENSOR_APP_H
#define SENSOR_APP_H

#include <rtems.h>

rtems_id sensor_period_id(void);

rtems_status_code sensor_app_start(void);

#endif