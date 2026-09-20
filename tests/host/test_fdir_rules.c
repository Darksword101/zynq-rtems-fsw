#include "fdir_rules.h"
#include "minitest.h"

static const fdir_rules_t test_rules = {
	.temp_limit_c = 60.0f,
	.hysteresis_c = 5.0f,
	.consecutive_needed = 3u,
	.missed_deadline_limit = 2u
};

static sensor_sample_t sample_at(float temp_c)
{
	sensor_sample_t sample = {0};
	sample.temp_c = temp_c;
	return sample;
}

static void test_boot_and_safe_latching(void)
{
	sensor_sample_t sample = sample_at(20.0f);
	fdir_state_t state = {
		.mode = FSW_MODE_BOOT,
		.fault_flags = 0u,
		.temp_high_run = 0u
	};

	CHECK(fdir_evaluate(&test_rules, &state, &sample, 0u) == FDIR_EVT_ENTER_NOMINAL);
	CHECK(state.mode == FSW_MODE_NOMINAL);

	state.mode = FSW_MODE_SAFE;
	state.fault_flags = FAULT_CMD_SAFE;
	state.temp_high_run = 7u;
	CHECK(fdir_evaluate(&test_rules, &state, &sample, 99u) == FDIR_EVT_NONE);
	CHECK(state.mode == FSW_MODE_SAFE);
	CHECK_EQ_U(state.fault_flags, FAULT_CMD_SAFE);
	CHECK_EQ_U(state.temp_high_run, 7u);
}

static void test_temperature_trip_and_counter_reset(void)
{
	sensor_sample_t sample = sample_at(60.0f);
	fdir_state_t state = {
		.mode = FSW_MODE_NOMINAL,
		.fault_flags = 0u,
		.temp_high_run = 0u
	};

	CHECK(fdir_evaluate(&test_rules, &state, &sample, 0u) == FDIR_EVT_NONE);
	CHECK_EQ_U(state.temp_high_run, 0u);
	CHECK(state.mode == FSW_MODE_NOMINAL);

	sample.temp_c = 61.0f;
	CHECK(fdir_evaluate(&test_rules, &state, &sample, 0u) == FDIR_EVT_NONE);
	CHECK_EQ_U(state.temp_high_run, 1u);
	CHECK(fdir_evaluate(&test_rules, &state, &sample, 0u) == FDIR_EVT_NONE);
	CHECK_EQ_U(state.temp_high_run, 2u);

	sample.temp_c = 59.0f;
	CHECK(fdir_evaluate(&test_rules, &state, &sample, 0u) == FDIR_EVT_NONE);
	CHECK_EQ_U(state.temp_high_run, 0u);
	CHECK(state.mode == FSW_MODE_NOMINAL);

	sample.temp_c = 61.0f;
	CHECK(fdir_evaluate(&test_rules, &state, &sample, 0u) == FDIR_EVT_NONE);
	CHECK(fdir_evaluate(&test_rules, &state, &sample, 0u) == FDIR_EVT_NONE);
	CHECK(fdir_evaluate(&test_rules, &state, &sample, 0u) == FDIR_EVT_TEMP_HIGH_SAFE);
	CHECK(state.mode == FSW_MODE_SAFE);
	CHECK((state.fault_flags & FAULT_TEMP_HIGH) != 0u);
}

static void test_deadline_trip_and_temperature_precedence(void)
{
	sensor_sample_t sample = sample_at(20.0f);
	fdir_state_t state = {
		.mode = FSW_MODE_NOMINAL,
		.fault_flags = 0u,
		.temp_high_run = 0u
	};

	CHECK(fdir_evaluate(&test_rules, &state, &sample, 2u) == FDIR_EVT_NONE);
	CHECK(state.mode == FSW_MODE_NOMINAL);
	CHECK(fdir_evaluate(&test_rules, &state, &sample, 3u) == FDIR_EVT_DEADLINE_SAFE);
	CHECK(state.mode == FSW_MODE_SAFE);
	CHECK((state.fault_flags & FAULT_DEADLINE_MISS) != 0u);

	state.mode = FSW_MODE_NOMINAL;
	state.fault_flags = 0u;
	state.temp_high_run = test_rules.consecutive_needed - 1u;
	sample.temp_c = 61.0f;
	CHECK(fdir_evaluate(&test_rules, &state, &sample, 3u) == FDIR_EVT_TEMP_HIGH_SAFE);
	CHECK((state.fault_flags & FAULT_TEMP_HIGH) != 0u);
	CHECK((state.fault_flags & FAULT_DEADLINE_MISS) == 0u);
}

static void test_safe_exit(void)
{
	fdir_state_t state = {
		.mode = FSW_MODE_NOMINAL,
		.fault_flags = FAULT_TEMP_HIGH,
		.temp_high_run = 2u
	};

	CHECK(!fdir_request_exit_safe(&test_rules, &state, 20.0f));
	CHECK(state.mode == FSW_MODE_NOMINAL);

	state.mode = FSW_MODE_SAFE;
	CHECK(!fdir_request_exit_safe(&test_rules, &state, 56.0f));
	CHECK(state.mode == FSW_MODE_SAFE);
	CHECK_EQ_U(state.fault_flags, FAULT_TEMP_HIGH);

	CHECK(fdir_request_exit_safe(&test_rules, &state, 55.0f));
	CHECK(state.mode == FSW_MODE_NOMINAL);
	CHECK_EQ_U(state.fault_flags, 0u);
	CHECK_EQ_U(state.temp_high_run, 0u);
}

int main(void)
{
	test_boot_and_safe_latching();
	test_temperature_trip_and_counter_reset();
	test_deadline_trip_and_temperature_precedence();
	test_safe_exit();

	MT_REPORT();
}
