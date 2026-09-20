#ifndef SB_H
#define SB_H

#include <rtems.h>
#include <stdint.h>
#include "fsw_config.h"

typedef struct {
    uint16_t msg_id;
    uint16_t length;
    uint32_t sequence;
    uint64_t timestamp_ns;
    uint8_t payload[SB_MAX_PAYLOAD];
} sb_msg_t;

_Static_assert(sizeof(sb_msg_t) == SB_MSG_MAX_SIZE, "update SB_MSG_MAX_SIZE in fsw_config.h");

typedef struct {rtems_id queue;} sb_pipe_t;

rtems_status_code sb_init(void);
rtems_status_code sb_create_pipe(const char name[4], uint32_t depth, sb_pipe_t *pipe);
rtems_status_code sb_subscribe(uint16_t msg_id, const sb_pipe_t *pipe);          /* init-time only */
rtems_status_code sb_publish(uint16_t msg_id, const void *payload, uint16_t len); /* never blocks */
rtems_status_code sb_receive(const sb_pipe_t *pipe, sb_msg_t *out, rtems_interval timeout); /* RTEMS_NO_TIMEOUT or ticks; 0 = poll */
uint32_t          sb_drop_count(void);
#endif  // SB_H