/*sb.c - essentials*/
#include <string.h>
#include "sb.h"
#include "timebase.h"

typedef struct {
    uint16_t msg_id;
    uint8_t n_subs;
    rtems_id subs[SB_MAX_SUBS_PER_MSG];
    uint32_t sequence;
} sb_route_t;

static sb_route_t routes[SB_MAX_ROUTES];
static uint8_t n_routes;
static rtems_id route_mutex;
static uint32_t drops;

rtems_status_code sb_init(void)
{
    memset(routes, 0, sizeof routes);
    n_routes = 0; drops = 0;
    return rtems_semaphore_create(rtems_build_name('S','B','M','X'), 1,
        RTEMS_BINARY_SEMAPHORE | RTEMS_PRIORITY | RTEMS_INHERIT_PRIORITY, 0, &route_mutex);
}

rtems_status_code sb_create_pipe(const char name[4], uint32_t depth, sb_pipe_t *pipe)
{
    if(depth > SB_PIPE_DEPTH_MAX ) return RTEMS_INVALID_NUMBER;
    return rtems_message_queue_create(rtems_build_name(name[0], name[1], name[2], name[3]),
        depth, sizeof(sb_msg_t), RTEMS_FIFO | RTEMS_LOCAL, &pipe->queue);
}

static sb_route_t *find_or_add(uint16_t msg_id)
{
    for(uint8_t i = 0; i < n_routes; i++) if (routes[i].msg_id == msg_id) return &routes[i];

    if(n_routes >= SB_MAX_ROUTES) return NULL;
    routes[n_routes].msg_id = msg_id;
    return &routes[n_routes++];
}

rtems_status_code sb_subscribe(uint16_t msg_id, const sb_pipe_t *pipe)
{
    rtems_semaphore_obtain(route_mutex, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
    sb_route_t *r = find_or_add(msg_id);
    rtems_status_code sc = RTEMS_TOO_MANY;
    if(r && r->n_subs < SB_MAX_SUBS_PER_MSG) { r->subs[r->n_subs++] = pipe->queue; sc = RTEMS_SUCCESSFUL; }
    rtems_semaphore_release(route_mutex);
    return sc;
}

rtems_status_code sb_publish(uint16_t msg_id, const void *payload, uint16_t len)
{
    if(len > SB_MAX_PAYLOAD) return RTEMS_INVALID_SIZE;
    sb_route_t *r = NULL;
    for(uint8_t i = 0; i < n_routes; ++i) if (routes[i].msg_id == msg_id) { r = &routes[i]; break;}
    if(!r) return RTEMS_SUCCESSFUL;

    sb_msg_t m = { .msg_id = msg_id, .length = len, .sequence = r->sequence++, .timestamp_ns = now_ns() };
    memcpy(m.payload, payload, len);

    for(uint8_t i = 0; i < r->n_subs; ++i){
        rtems_status_code sc = rtems_message_queue_send(r->subs[i], &m, sizeof(m)); /*non blocking by design*/
        if(sc != RTEMS_SUCCESSFUL) drops++;
    }

    return RTEMS_SUCCESSFUL;
}

rtems_status_code sb_receive(const sb_pipe_t *pipe, sb_msg_t *out, rtems_interval timeout)
{
    size_t got = 0;
    rtems_option opt = (timeout == 0) ? RTEMS_NO_WAIT : RTEMS_WAIT;
    return rtems_message_queue_receive(pipe->queue, out, &got, opt, timeout);
}

uint32_t sb_drop_count(void) { return drops; }