/*
 *   Copyright 2024-2026 Franciszek Balcerak
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#pragma once

#include <shared/threads.h>

#include <stdint.h>

#define TIME_IMMEDIATELY ((uint64_t) 2)
#define TIME_STEP ((uint64_t) 2)


extern uint64_t
time_sec_to_ms(
	uint64_t sec
	);


extern uint64_t
time_sec_to_us(
	uint64_t sec
	);


extern uint64_t
time_sec_to_ns(
	uint64_t sec
	);


extern uint64_t
time_ms_to_sec(
	uint64_t ms
	);


extern uint64_t
time_ms_to_us(
	uint64_t ms
	);


extern uint64_t
time_ms_to_ns(
	uint64_t ms
	);


extern uint64_t
time_us_to_sec(
	uint64_t us
	);


extern uint64_t
time_us_to_ms(
	uint64_t us
	);


extern uint64_t
time_us_to_ns(
	uint64_t us
	);


extern uint64_t
time_ns_to_sec(
	uint64_t ns
	);


extern uint64_t
time_ns_to_ms(
	uint64_t ns
	);


extern uint64_t
time_ns_to_us(
	uint64_t ns
	);


extern uint64_t
time_get(
	void
	);


extern uint64_t
time_get_with_sec(
	uint64_t sec
	);


extern uint64_t
time_get_with_ms(
	uint64_t ms
	);


extern uint64_t
time_get_with_us(
	uint64_t us
	);


extern uint64_t
time_get_with_ns(
	uint64_t ns
	);


typedef struct time_timer
{
	uint32_t idx;
}
time_timer_t;


typedef thread_fn_t time_fn_t;

typedef thread_data_t time_data_t;


typedef struct time_timeout
{
	time_timer_t* timer;

	time_data_t data;

	uint64_t time;
}
time_timeout_t;


typedef struct time_interval
{
	time_timer_t* timer;

	time_data_t data;

	uint64_t base_time;
	uint64_t interval;
	uint64_t count;
}
time_interval_t;


typedef struct time_timers* time_timers_t;


extern void
time_timers_fn(
	void* data
	);


extern time_timers_t
time_timers_init(
	void
	);


extern void
time_timers_free(
	time_timers_t timers
	);


extern void
time_timers_lock(
	time_timers_t timers
	);


extern void
time_timers_unlock(
	time_timers_t timers
	);


extern time_timer_t*
time_timers_get_current_timer(
	time_timers_t timers
	);



extern void
time_timer_init(
	time_timer_t* timer
	);


extern bool
time_timers_is_timer_expired_u(
	time_timers_t timers,
	time_timer_t* timer
	);


extern bool
time_timers_is_timer_expired(
	time_timers_t timers,
	time_timer_t* timer
	);


extern void
time_timer_free(
	time_timer_t* timer
	);



extern void
time_timers_add_timeout_u(
	time_timers_t timers,
	time_timeout_t timeout
	);


extern void
time_timers_add_timeout(
	time_timers_t timers,
	time_timeout_t timeout
	);


extern bool
time_timers_cancel_timeout_u(
	time_timers_t timers,
	time_timer_t* timer
	);


extern bool
time_timers_cancel_timeout(
	time_timers_t timers,
	time_timer_t* timer
	);


extern time_timeout_t*
time_timers_open_timeout_u(
	time_timers_t timers,
	time_timer_t* timer
	);


extern time_timeout_t*
time_timers_open_timeout(
	time_timers_t timers,
	time_timer_t* timer
	);


extern void
time_timers_close_timeout_u(
	time_timers_t timers,
	time_timer_t* timer
	);


extern void
time_timers_close_timeout(
	time_timers_t timers,
	time_timer_t* timer
	);


extern uint64_t
time_timer_get_timeout_u(
	time_timeout_t* timeout
	);


extern uint64_t
time_timers_get_timeout_u(
	time_timers_t timers,
	time_timer_t* timer
	);


extern uint64_t
time_timers_get_timeout(
	time_timers_t timers,
	time_timer_t* timer
	);


extern void
time_timer_set_timeout_u(
	time_timeout_t* timeout,
	uint64_t time
	);


extern bool
time_timers_set_timeout_u(
	time_timers_t timers,
	time_timer_t* timer,
	uint64_t time
	);


extern bool
time_timers_set_timeout(
	time_timers_t timers,
	time_timer_t* timer,
	uint64_t time
	);


extern void
time_timers_update_timeout_timer_u(
	time_timers_t timers,
	time_timeout_t* timeout
	);



extern void
time_timers_add_interval_u(
	time_timers_t timers,
	time_interval_t interval
	);


extern void
time_timers_add_interval(
	time_timers_t timers,
	time_interval_t interval
	);


extern bool
time_timers_cancel_interval_u(
	time_timers_t timers,
	time_timer_t* timer
	);


extern bool
time_timers_cancel_interval(
	time_timers_t timers,
	time_timer_t* timer
	);


extern time_interval_t*
time_timers_open_interval_u(
	time_timers_t timers,
	time_timer_t* timer
	);


extern time_interval_t*
time_timers_open_interval(
	time_timers_t timers,
	time_timer_t* timer
	);


extern void
time_timers_close_interval_u(
	time_timers_t timers,
	time_timer_t* timer
	);


extern void
time_timers_close_interval(
	time_timers_t timers,
	time_timer_t* timer
	);


extern uint64_t
time_timer_get_interval_u(
	time_interval_t* interval
	);


extern uint64_t
time_timers_get_interval_u(
	time_timers_t timers,
	time_timer_t* timer
	);


extern uint64_t
time_timers_get_interval(
	time_timers_t timers,
	time_timer_t* timer
	);


extern void
time_timer_set_interval_u(
	time_interval_t* interval,
	uint64_t base_time,
	uint64_t interval_time,
	uint64_t count
	);


extern bool
time_timers_set_interval_u(
	time_timers_t timers,
	time_timer_t* timer,
	uint64_t base_time,
	uint64_t interval_time,
	uint64_t count
	);


extern bool
time_timers_set_interval(
	time_timers_t timers,
	time_timer_t* timer,
	uint64_t base_time,
	uint64_t interval_time,
	uint64_t count
	);


extern void
time_timers_update_interval_timer_u(
	time_timers_t timers,
	time_interval_t* interval
	);
