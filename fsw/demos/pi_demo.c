#include <rtems.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "pi_demo.h"
#include "timebase.h"

static rtems_id mtx;
static volatile uint64_t h_blocked_ns;
static rtems_id tid_l, tid_m, tid_h;

static void busy_ms(uint32_t ms) {
    uint64_t t0 = now_ns();
    while( now_ns() - t0 < (uint64_t)ms * 1000000ull)
    {
    }
}

static rtems_task low_task(rtems_task_argument a)
{
    (void)a;
    rtems_semaphore_obtain(mtx, RTEMS_WAIT, RTEMS_NO_TIMEOUT);

    busy_ms(50);  /* long critical section - on purpose */
    rtems_semaphore_release(mtx);
    rtems_task_exit();

}

static rtems_task med_task(rtems_task_argument a)
{
    (void) a;
    busy_ms(300);
    rtems_task_exit();
}

static rtems_task high_task(rtems_task_argument a)
{
    (void) a;
    uint64_t t0 = now_ns();
    rtems_semaphore_obtain(mtx, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    h_blocked_ns = now_ns() - t0;
    rtems_semaphore_release(mtx);
    rtems_task_exit();
}

static void run_once(bool inherit)
{
    rtems_attribute attr = RTEMS_BINARY_SEMAPHORE | RTEMS_PRIORITY | (inherit ? RTEMS_INHERIT_PRIORITY : RTEMS_NO_INHERIT_PRIORITY);
    rtems_semaphore_create(rtems_build_name('P','I','M','X'), 1, attr, 0, &mtx);

    rtems_task_create(rtems_build_name('L','O','W',' '), 30, 8192, RTEMS_DEFAULT_MODES, RTEMS_DEFAULT_ATTRIBUTES, &tid_l);
    rtems_task_create(rtems_build_name('M','E','D',' '), 20, 8192, RTEMS_DEFAULT_MODES, RTEMS_DEFAULT_ATTRIBUTES, &tid_m);
    rtems_task_create(rtems_build_name('H','I','G','H'), 10, 8192, RTEMS_DEFAULT_MODES, RTEMS_DEFAULT_ATTRIBUTES, &tid_h);

    rtems_task_start(tid_l, low_task, 0);
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(10));   /* Init (prio 1) sleeps → L runs, grabs mutex */
    rtems_task_start(tid_m, med_task, 0);
    rtems_task_start(tid_h, high_task, 0);                    /* both ready; H runs first when Init blocks */
    rtems_task_wake_after(RTEMS_MILLISECONDS_TO_TICKS(1000));

    printf("PI demo: inherit=%s  H blocked for %llu ms\n", inherit ? "yes" : "no ", (unsigned long long)(h_blocked_ns / 1000000ull));
    rtems_semaphore_delete(mtx);
}

void pi_demo_run(void) {run_once(false); run_once(true);}