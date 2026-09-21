#ifndef FDIR_APP_H
#define FDIR_APP_H

#include <rtems.h>

void publish_event(const char *text);
rtems_status_code fdir_app_start(void);

#endif