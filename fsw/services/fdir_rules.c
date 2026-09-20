#include "fdir_rules.h"

fdir_event_t fdir_evaluate(const fdir_rules_t *r, fdir_state_t *s, const sensor_sample_t *smp, uint16_t missed_deadlines)
{
    if(s->mode == FSW_MODE_BOOT) 
    {
        s->mode = FSW_MODE_NOMINAL; 
        return FDIR_EVT_ENTER_NOMINAL;
    }

    if (s->mode != FSW_MODE_NOMINAL) return FDIR_EVT_NONE;  /*SAFE latches until commanded*/

    s->temp_high_run = (smp->temp_c > r->temp_limit_c) ? (uint8_t)(s->temp_high_run +1) : 0;

    if (s->temp_high_run >= r->consecutive_needed)
    {
        s->mode = FSW_MODE_SAFE;
        s->fault_flags |= FAULT_TEMP_HIGH;
        return FDIR_EVT_TEMP_HIGH_SAFE;
    }

    if (missed_deadlines > r->missed_deadline_limit)
    {
        s->mode = FSW_MODE_SAFE;
        s->fault_flags |= FAULT_DEADLINE_MISS;
        return FDIR_EVT_DEADLINE_SAFE;
    }

    return FDIR_EVT_NONE;
}



bool fdir_request_exit_safe(const fdir_rules_t *r, fdir_state_t *s, float temp_c)
{
    if (s->mode != FSW_MODE_SAFE) return false;
    if (temp_c > r->temp_limit_c - r->hysteresis_c) return false;          /* still too hot: reject */
    s->mode = FSW_MODE_NOMINAL; s->fault_flags = 0; s->temp_high_run = 0;
    return true;
}