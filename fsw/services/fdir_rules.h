#ifndef FDIR_RULES_H
#define FDIR_RULES_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include "fsw_types.h"

typedef struct { 
    float temp_limit_c; 
    float hysteresis_c; 
    uint8_t consecutive_needed; 
    uint16_t missed_deadline_limit;
} fdir_rules_t;

typedef struct {
    fsw_mode_t mode; 
    uint8_t fault_flags; 
    uint8_t temp_high_run;
 }  fdir_state_t;

typedef enum {
    FDIR_EVT_NONE,
    FDIR_EVT_ENTER_NOMINAL,
    FDIR_EVT_TEMP_HIGH_SAFE,
    FDIR_EVT_DEADLINE_SAFE
} fdir_event_t;

fdir_event_t fdir_evaluate(const fdir_rules_t *r, fdir_state_t *s, const sensor_sample_t *smp, uint16_t missed_deadlines);

bool fdir_request_exit_safe(const fdir_rules_t *r, fdir_state_t *s, float temp_c);

#endif
