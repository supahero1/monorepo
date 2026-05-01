/*
 *   Copyright 2025-2026 Franciszek Balcerak
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

#include <shared/attr.h>
#include <shared/time.h>
#include <shared/debug.h>
#include <shared/macro.h>
#include <shared/threads.h>

#include <stddef.h>


void attr_test_fn
test_normal_pass__time_time_conversion(
	void
	)
{
	assert_eq(time_sec_to_ns(0), 0);
	assert_eq(time_sec_to_us(0), 0);
	assert_eq(time_sec_to_ms(0), 0);

	assert_eq(time_ms_to_ns(0), 0);
	assert_eq(time_ms_to_us(0), 0);
	assert_eq(time_ms_to_sec(0), 0);

	assert_eq(time_us_to_ns(0), 0);
	assert_eq(time_us_to_sec(0), 0);
	assert_eq(time_us_to_ms(0), 0);

	assert_eq(time_ns_to_sec(0), 0);
	assert_eq(time_ns_to_ms(0), 0);
	assert_eq(time_ns_to_us(0), 0);

	assert_eq(time_sec_to_ns(1), 1000000000);
	assert_eq(time_sec_to_us(1), 1000000);
	assert_eq(time_sec_to_ms(1), 1000);

	assert_eq(time_ms_to_ns(1), 1000000);
	assert_eq(time_ms_to_us(1), 1000);
	assert_eq(time_ms_to_sec(999), 0);
	assert_eq(time_ms_to_sec(1999), 1);

	assert_eq(time_us_to_ns(1), 1000);
	assert_eq(time_us_to_sec(999999), 0);
	assert_eq(time_us_to_sec(1999999), 1);
	assert_eq(time_us_to_ms(999), 0);
	assert_eq(time_us_to_ms(1999), 1);

	assert_eq(time_ns_to_sec(999999999), 0);
	assert_eq(time_ns_to_sec(1999999999), 1);
	assert_eq(time_ns_to_ms(999999), 0);
	assert_eq(time_ns_to_ms(1999999), 1);
	assert_eq(time_ns_to_us(999), 0);
	assert_eq(time_ns_to_us(1999), 1);
}


void attr_test_fn
test_normal_pass__time_timers_init_free(
	void
	)
{
	time_timers_t timers = time_timers_init();
	time_timers_free(timers);
}


void attr_test_fn
test_normal_fail__time_timers_free_null(
	void
	)
{
	time_timers_free(NULL);
}


void attr_test_fn
test_normal_fail__time_timers_lock_null(
	void
	)
{
	time_timers_lock(NULL);
}


void attr_test_fn
test_normal_fail__time_timers_unlock_null(
	void
	)
{
	time_timers_unlock(NULL);
}


void attr_test_fn
test_normal_pass__time_timer_init_free(
	void
	)
{
	time_timer_t timer;
	time_timer_init(&timer);
	time_timer_free(&timer);
}


void attr_test_fn
test_normal_fail__time_timer_init_null(
	void
	)
{
	time_timer_init(NULL);
}


void attr_test_fn
test_normal_fail__time_timer_free_null(
	void
	)
{
	time_timer_free(NULL);
}


void attr_test_fn
test_normal_pass__time_timers_is_timer_expired_u(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	assert_true(time_timers_is_timer_expired_u(timers, &timer));

	time_timers_free(timers);
}


void attr_test_fn
test_normal_fail__time_timers_is_timer_expired_u_timers_null(
	void
	)
{
	time_timer_t timer;
	time_timer_init(&timer);

	time_timers_is_timer_expired_u(NULL, &timer);
}


void attr_test_fn
test_normal_fail__time_timers_is_timer_expired_u_timer_null(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timers_is_timer_expired_u(timers, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_is_timer_expired_u_null(
	void
	)
{
	time_timers_is_timer_expired_u(NULL, NULL);
}


void attr_test_fn
test_normal_pass__time_timers_is_timer_expired(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	assert_true(time_timers_is_timer_expired(timers, &timer));

	time_timers_free(timers);
}


void attr_test_fn
test_normal_fail__time_timers_is_timer_expired_timers_null(
	void
	)
{
	time_timer_t timer;
	time_timer_init(&timer);

	time_timers_is_timer_expired(NULL, &timer);
}


void attr_test_fn
test_normal_fail__time_timers_is_timer_expired_timer_null(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timers_is_timer_expired(timers, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_is_timer_expired_null(
	void
	)
{
	time_timers_is_timer_expired(NULL, NULL);
}


void
timer_never_fn(
	void* data
	)
{
	(void) data;
	assert_unreachable();
}


void attr_test_fn
test_priority_pass__time_timers_add_timeout_and_cancel(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	time_timeout_t timeout =
	{
		.timer = &timer,
		.data =
		{
			.fn = timer_never_fn,
			.data = NULL
		},
		.time = time_get_with_sec(2)
	};
	time_timers_add_timeout(timers, timeout);

	assert_false(time_timers_is_timer_expired(timers, &timer));
	assert_eq(time_timers_get_timeout(timers, &timer), timeout.time);

	time_timeout_t* timeout_ptr = time_timers_open_timeout(timers, &timer);
	assert_not_null(timeout_ptr);

	assert_eq(timeout_ptr->data.fn, timeout.data.fn);
	assert_eq(timeout_ptr->data.data, timeout.data.data);
	assert_eq(timeout_ptr->time, timeout.time);

	time_timers_close_timeout(timers, &timer);

	assert_true(time_timers_cancel_timeout(timers, &timer));
	assert_true(time_timers_is_timer_expired(timers, &timer));

	time_timer_free(&timer);

	thread_sleep(time_sec_to_ns(4));

	time_timers_free(timers);
}


void attr_test_fn
test_normal_fail__time_timers_add_timeout_null_fn(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timeout_t timeout = {0};
	time_timers_add_timeout(timers, timeout);
}


void attr_test_fn
test_normal_fail__time_timers_add_timeout_u_null_fn(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timeout_t timeout = {0};
	time_timers_add_timeout_u(timers, timeout);
}


void attr_test_fn
test_normal_fail__time_timers_add_timeout_null_timers(
	void
	)
{
	time_timeout_t timeout =
	{
		.data =
		{
			.fn = timer_never_fn
		}
	};
	time_timers_add_timeout(NULL, timeout);
}


void attr_test_fn
test_normal_fail__time_timers_add_timeout_u_null_timers(
	void
	)
{
	time_timeout_t timeout =
	{
		.data =
		{
			.fn = timer_never_fn
		}
	};
	time_timers_add_timeout_u(NULL, timeout);
}


void attr_test_fn
test_normal_fail__time_timers_add_timeout_null(
	void
	)
{
	time_timers_add_timeout(NULL, (time_timeout_t){0});
}


void attr_test_fn
test_normal_fail__time_timers_add_timeout_u_null(
	void
	)
{
	time_timers_add_timeout_u(NULL, (time_timeout_t){0});
}


void attr_test_fn
test_normal_fail__time_timers_get_timeout_null_timer(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timers_get_timeout(timers, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_get_timeout_u_null_timer(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timers_get_timeout_u(timers, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_get_timeout_null_timers(
	void
	)
{
	time_timer_t timer;
	time_timer_init(&timer);

	time_timers_get_timeout(NULL, &timer);
}


void attr_test_fn
test_normal_fail__time_timers_get_timeout_u_null_timers(
	void
	)
{
	time_timer_t timer;
	time_timer_init(&timer);

	time_timers_get_timeout_u(NULL, &timer);
}


void attr_test_fn
test_normal_fail__time_timers_get_timeout_null(
	void
	)
{
	time_timers_get_timeout(NULL, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_get_timeout_u_null(
	void
	)
{
	time_timers_get_timeout_u(NULL, NULL);
}


void attr_test_fn
test_normal_pass__time_timers_get_timeout_expired(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	assert_eq(time_timers_get_timeout(timers, &timer), 0);

	time_timers_free(timers);
}


void attr_test_fn
test_normal_pass__time_timers_get_timeout_u_expired(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	assert_eq(time_timers_get_timeout_u(timers, &timer), 0);

	time_timers_free(timers);
}


void attr_test_fn
test_normal_fail__time_timers_open_timeout_null_timer(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timers_open_timeout(timers, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_open_timeout_u_null_timer(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timers_open_timeout_u(timers, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_open_timeout_null_timers(
	void
	)
{
	time_timer_t timer;
	time_timer_init(&timer);

	time_timers_open_timeout(NULL, &timer);
}


void attr_test_fn
test_normal_fail__time_timers_open_timeout_u_null_timers(
	void
	)
{
	time_timer_t timer;
	time_timer_init(&timer);

	time_timers_open_timeout_u(NULL, &timer);
}


void attr_test_fn
test_normal_fail__time_timers_open_timeout_null(
	void
	)
{
	time_timers_open_timeout(NULL, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_open_timeout_u_null(
	void
	)
{
	time_timers_open_timeout_u(NULL, NULL);
}


void attr_test_fn
test_normal_pass__time_timers_open_timeout_expired(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	assert_null(time_timers_open_timeout(timers, &timer));

	time_timers_free(timers);
}


void attr_test_fn
test_normal_pass__time_timers_open_timeout_u_expired(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	assert_null(time_timers_open_timeout_u(timers, &timer));

	time_timers_free(timers);
}


void attr_test_fn
test_normal_fail__time_timers_close_timeout_null_timer(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timers_close_timeout(timers, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_close_timeout_u_null_timer(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timers_close_timeout_u(timers, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_close_timeout_null_timers(
	void
	)
{
	time_timer_t timer;
	time_timer_init(&timer);

	time_timers_close_timeout(NULL, &timer);
}


void attr_test_fn
test_normal_fail__time_timers_close_timeout_u_null_timers(
	void
	)
{
	time_timer_t timer;
	time_timer_init(&timer);

	time_timers_close_timeout_u(NULL, &timer);
}


void attr_test_fn
test_normal_fail__time_timers_close_timeout_null(
	void
	)
{
	time_timers_close_timeout(NULL, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_close_timeout_u_null(
	void
	)
{
	time_timers_close_timeout_u(NULL, NULL);
}


void
timer_set_flag_fn(
	bool* flag
	)
{
	*flag = true;
}


void attr_test_fn
test_priority_pass__time_timers_set_timeout_u(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	bool flag = false;

	time_timers_lock(timers);

	time_timeout_t timeout =
	{
		.timer = &timer,
		.data =
		{
			.fn = (void*) timer_set_flag_fn,
			.data = &flag
		},
		.time = time_get_with_sec(2)
	};
	time_timers_add_timeout_u(timers, timeout);

	assert_false(time_timers_is_timer_expired_u(timers, &timer));
	assert_eq(time_timers_get_timeout_u(timers, &timer), timeout.time);

	time_timeout_t* timeout_ptr = time_timers_open_timeout_u(timers, &timer);
	assert_not_null(timeout_ptr);

	assert_eq(timeout_ptr->data.fn, timeout.data.fn);
	assert_eq(timeout_ptr->data.data, timeout.data.data);
	assert_eq(timeout_ptr->time, timeout.time);

	time_timers_set_timeout_u(timers, &timer, TIME_IMMEDIATELY);
	time_timers_close_timeout_u(timers, &timer);

	time_timers_unlock(timers);

	thread_sleep(time_sec_to_ns(4));

	assert_true(flag);
	assert_true(time_timers_is_timer_expired(timers, &timer));

	time_timer_free(&timer);

	time_timers_free(timers);
}


void attr_test_fn
test_priority_pass__time_timers_set_timeout_long_u(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	bool flag = false;

	time_timers_lock(timers);

	time_timeout_t timeout =
	{
		.timer = &timer,
		.data =
		{
			.fn = (void*) timer_set_flag_fn,
			.data = &flag
		},
		.time = time_get_with_sec(4)
	};
	time_timers_add_timeout_u(timers, timeout);

	time_timers_unlock(timers);
	time_timers_lock(timers);

	assert_false(flag);
	assert_false(time_timers_is_timer_expired_u(timers, &timer));

	thread_sleep(time_sec_to_ns(2));

	assert_false(flag);
	assert_false(time_timers_is_timer_expired_u(timers, &timer));
	assert_eq(time_timers_get_timeout_u(timers, &timer), timeout.time);

	time_timeout_t* timeout_ptr = time_timers_open_timeout_u(timers, &timer);
	assert_not_null(timeout_ptr);

	assert_eq(timeout_ptr->data.fn, timeout.data.fn);
	assert_eq(timeout_ptr->data.data, timeout.data.data);
	assert_eq(timeout_ptr->time, timeout.time);

	time_timers_set_timeout_u(timers, &timer, TIME_IMMEDIATELY);
	time_timers_close_timeout_u(timers, &timer);

	time_timers_unlock(timers);

	thread_sleep(time_sec_to_ns(2));

	assert_true(flag);
	assert_true(time_timers_is_timer_expired(timers, &timer));

	time_timer_free(&timer);

	time_timers_free(timers);
}


void attr_test_fn
test_priority_pass__time_timers_open_cancel_timeout(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	time_timeout_t timeout =
	{
		.timer = &timer,
		.data =
		{
			.fn = timer_never_fn,
			.data = NULL
		},
		.time = time_get_with_sec(2)
	};
	time_timers_add_timeout(timers, timeout);

	time_timeout_t* timeout_ptr = time_timers_open_timeout(timers, &timer);
	assert_not_null(timeout_ptr);

	time_timers_cancel_timeout_u(timers, &timer);
	assert_true(time_timers_is_timer_expired_u(timers, &timer));

	time_timers_close_timeout(timers, &timer);
	assert_true(time_timers_is_timer_expired(timers, &timer));

	thread_sleep(time_sec_to_ns(4));

	time_timer_free(&timer);

	time_timers_free(timers);
}


typedef struct timer_cancel_data
{
	bool timeout;
	time_timers_t timers;
	time_timer_t* timer;
}
timer_cancel_data_t;


void
timer_timeout_cancel_timer_fn(
	timer_cancel_data_t* data
	)
{
	if(data->timeout)
	{
		assert_true(time_timers_cancel_timeout(data->timers, data->timer));
	}
	else
	{
		assert_true(time_timers_cancel_interval(data->timers, data->timer));
	}
}


void attr_test_fn
test_priority_pass__time_timers_timeout_cancel_timeout(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	time_timeout_t timeout =
	{
		.timer = &timer,
		.data =
		{
			.fn = timer_never_fn,
			.data = NULL
		},
		.time = time_get_with_sec(2)
	};
	time_timers_add_timeout(timers, timeout);

	timer_cancel_data_t data =
	{
		.timeout = true,
		.timers = timers,
		.timer = &timer
	};
	time_timeout_t cancel_timeout =
	{
		.data =
		{
			.fn = (void*) timer_timeout_cancel_timer_fn,
			.data = &data
		},
		.time = TIME_IMMEDIATELY
	};
	time_timers_add_timeout(timers, cancel_timeout);

	thread_sleep(time_sec_to_ns(4));

	assert_true(time_timers_is_timer_expired(timers, &timer));

	time_timer_free(&timer);

	time_timers_free(timers);
}


void
timer_nothing_fn(
	void* data
	)
{
	(void) data;
}


void attr_test_fn
test_normal_fail__time_timers_timeout_cancel_timeout_too_late(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	time_timeout_t timeout =
	{
		.timer = &timer,
		.data =
		{
			.fn = timer_nothing_fn,
			.data = NULL
		},
		.time = time_get_with_sec(2)
	};
	time_timers_add_timeout(timers, timeout);

	timer_cancel_data_t data =
	{
		.timeout = true,
		.timers = timers,
		.timer = &timer
	};
	time_timeout_t cancel_timeout =
	{
		.data =
		{
			.fn = (void*) timer_timeout_cancel_timer_fn,
			.data = &data
		},
		.time = time_get_with_sec(2)
	};
	time_timers_add_timeout(timers, cancel_timeout);

	thread_sleep(time_sec_to_ns(99));
}


void attr_test_fn
test_priority_pass__time_timers_timeout_cancel_interval(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	time_interval_t interval =
	{
		.timer = &timer,
		.data =
		{
			.fn = timer_never_fn,
			.data = NULL
		},
		.base_time = time_get_with_sec(2),
		.count = 0,
		.interval = 0
	};
	time_timers_add_interval(timers, interval);

	timer_cancel_data_t data =
	{
		.timeout = false,
		.timers = timers,
		.timer = &timer
	};
	time_timeout_t cancel_timeout =
	{
		.data =
		{
			.fn = (void*) timer_timeout_cancel_timer_fn,
			.data = &data
		},
		.time = TIME_IMMEDIATELY
	};
	time_timers_add_timeout(timers, cancel_timeout);

	thread_sleep(time_sec_to_ns(4));

	assert_true(time_timers_is_timer_expired(timers, &timer));

	time_timer_free(&timer);

	time_timers_free(timers);
}


void attr_test_fn
test_priority_pass__time_timers_add_interval_and_cancel(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	time_interval_t interval =
	{
		.timer = &timer,
		.data =
		{
			.fn = timer_never_fn,
			.data = NULL
		},
		.base_time = time_get(),
		.count = 1,
		.interval = time_sec_to_ns(2)
	};
	time_timers_add_interval(timers, interval);

	assert_false(time_timers_is_timer_expired(timers, &timer));
	assert_eq(time_timers_get_interval(timers, &timer), interval.base_time + interval.interval);

	time_interval_t* interval_ptr = time_timers_open_interval(timers, &timer);
	assert_not_null(interval_ptr);

	assert_eq(interval_ptr->data.fn, interval.data.fn);
	assert_eq(interval_ptr->data.data, interval.data.data);
	assert_eq(interval_ptr->base_time + interval_ptr->interval, interval.base_time + interval.interval);

	time_timers_close_interval(timers, &timer);

	assert_true(time_timers_cancel_interval(timers, &timer));
	assert_true(time_timers_is_timer_expired(timers, &timer));

	time_timer_free(&timer);

	thread_sleep(time_sec_to_ns(4));

	time_timers_free(timers);
}


void attr_test_fn
test_normal_fail__time_timers_add_interval_null_fn(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_interval_t interval = {0};
	time_timers_add_interval(timers, interval);
}


void attr_test_fn
test_normal_fail__time_timers_add_interval_u_null_fn(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_interval_t interval = {0};
	time_timers_add_interval_u(timers, interval);
}


void attr_test_fn
test_normal_fail__time_timers_add_interval_null_timers(
	void
	)
{
	time_interval_t interval =
	{
		.data =
		{
			.fn = timer_never_fn
		}
	};
	time_timers_add_interval(NULL, interval);
}


void attr_test_fn
test_normal_fail__time_timers_add_interval_u_null_timers(
	void
	)
{
	time_interval_t interval =
	{
		.data =
		{
			.fn = timer_never_fn
		}
	};
	time_timers_add_interval_u(NULL, interval);
}


void attr_test_fn
test_normal_fail__time_timers_add_interval_null(
	void
	)
{
	time_timers_add_interval(NULL, (time_interval_t){0});
}


void attr_test_fn
test_normal_fail__time_timers_add_interval_u_null(
	void
	)
{
	time_timers_add_interval_u(NULL, (time_interval_t){0});
}


void attr_test_fn
test_normal_fail__time_timers_get_interval_null_timer(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timers_get_interval(timers, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_get_interval_u_null_timer(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timers_get_interval_u(timers, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_get_interval_null_timers(
	void
	)
{
	time_timer_t timer;
	time_timer_init(&timer);

	time_timers_get_interval(NULL, &timer);
}


void attr_test_fn
test_normal_fail__time_timers_get_interval_u_null_timers(
	void
	)
{
	time_timer_t timer;
	time_timer_init(&timer);

	time_timers_get_interval_u(NULL, &timer);
}


void attr_test_fn
test_normal_fail__time_timers_get_interval_null(
	void
	)
{
	time_timers_get_interval(NULL, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_get_interval_u_null(
	void
	)
{
	time_timers_get_interval_u(NULL, NULL);
}


void attr_test_fn
test_normal_pass__time_timers_get_interval_expired(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	assert_eq(time_timers_get_interval(timers, &timer), 0);

	time_timers_free(timers);
}


void attr_test_fn
test_normal_pass__time_timers_get_interval_u_expired(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	assert_eq(time_timers_get_interval_u(timers, &timer), 0);

	time_timers_free(timers);
}


void attr_test_fn
test_normal_fail__time_timers_open_interval_null_timer(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timers_open_interval(timers, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_open_interval_u_null_timer(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timers_open_interval_u(timers, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_open_interval_null_timers(
	void
	)
{
	time_timer_t timer;
	time_timer_init(&timer);

	time_timers_open_interval(NULL, &timer);
}


void attr_test_fn
test_normal_fail__time_timers_open_interval_u_null_timers(
	void
	)
{
	time_timer_t timer;
	time_timer_init(&timer);

	time_timers_open_interval_u(NULL, &timer);
}


void attr_test_fn
test_normal_fail__time_timers_open_interval_null(
	void
	)
{
	time_timers_open_interval(NULL, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_open_interval_u_null(
	void
	)
{
	time_timers_open_interval_u(NULL, NULL);
}


void attr_test_fn
test_normal_pass__time_timers_open_interval_expired(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	assert_null(time_timers_open_interval(timers, &timer));

	time_timers_free(timers);
}


void attr_test_fn
test_normal_pass__time_timers_open_interval_u_expired(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	assert_null(time_timers_open_interval_u(timers, &timer));

	time_timers_free(timers);
}


void attr_test_fn
test_normal_fail__time_timers_close_interval_null_timer(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timers_close_interval(timers, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_close_interval_u_null_timer(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timers_close_interval_u(timers, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_close_interval_null_timers(
	void
	)
{
	time_timer_t timer;
	time_timer_init(&timer);

	time_timers_close_interval(NULL, &timer);
}


void attr_test_fn
test_normal_fail__time_timers_close_interval_u_null_timers(
	void
	)
{
	time_timer_t timer;
	time_timer_init(&timer);

	time_timers_close_interval_u(NULL, &timer);
}


void attr_test_fn
test_normal_fail__time_timers_close_interval_null(
	void
	)
{
	time_timers_close_interval(NULL, NULL);
}


void attr_test_fn
test_normal_fail__time_timers_close_interval_u_null(
	void
	)
{
	time_timers_close_interval_u(NULL, NULL);
}


void attr_test_fn
test_priority_pass__time_timers_set_interval_u(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	bool flag = false;

	time_timers_lock(timers);

	time_interval_t interval =
	{
		.timer = &timer,
		.data =
		{
			.fn = (void*) timer_set_flag_fn,
			.data = &flag
		},
		.base_time = time_get(),
		.count = 99,
		.interval = time_sec_to_ns(1)
	};
	time_timers_add_interval_u(timers, interval);

	assert_false(time_timers_is_timer_expired_u(timers, &timer));
	assert_eq(
		time_timers_get_interval_u(timers, &timer),
		interval.base_time + interval.interval * interval.count
		);

	time_interval_t* interval_ptr = time_timers_open_interval_u(timers, &timer);
	assert_not_null(interval_ptr);

	assert_eq(interval_ptr->data.fn, interval.data.fn);
	assert_eq(interval_ptr->data.data, interval.data.data);
	assert_eq(
		interval_ptr->base_time + interval_ptr->interval * interval_ptr->count,
		interval.base_time + interval.interval * interval.count
		);

	time_timers_set_interval_u(timers, &timer, time_get(), time_sec_to_ns(1), 0);
	time_timers_close_interval_u(timers, &timer);

	time_timers_unlock(timers);

	thread_sleep(time_sec_to_ns(2));

	assert_true(flag);
	assert_false(time_timers_is_timer_expired(timers, &timer));
	assert_true(time_timers_cancel_interval(timers, &timer));
	assert_true(time_timers_is_timer_expired(timers, &timer));

	time_timer_free(&timer);

	time_timers_free(timers);
}


void attr_test_fn
test_priority_pass__time_timers_set_interval_long_u(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	bool flag = false;

	time_timers_lock(timers);

	time_interval_t interval =
	{
		.timer = &timer,
		.data =
		{
			.fn = (void*) timer_set_flag_fn,
			.data = &flag
		},
		.base_time = time_get(),
		.count = 1,
		.interval = time_sec_to_ns(2)
	};
	time_timers_add_interval_u(timers, interval);

	time_timers_unlock(timers);
	time_timers_lock(timers);

	assert_false(flag);
	assert_false(time_timers_is_timer_expired_u(timers, &timer));

	thread_sleep(time_sec_to_ns(1));

	assert_false(flag);
	assert_false(time_timers_is_timer_expired_u(timers, &timer));
	assert_eq(
		time_timers_get_interval_u(timers, &timer),
		interval.base_time + interval.interval
		);

	time_interval_t* interval_ptr = time_timers_open_interval_u(timers, &timer);
	assert_not_null(interval_ptr);

	assert_eq(interval_ptr->data.fn, interval.data.fn);
	assert_eq(interval_ptr->data.data, interval.data.data);
	assert_eq(
		interval_ptr->base_time + interval_ptr->interval * interval_ptr->count,
		interval.base_time + interval.interval
		);

	time_timers_set_interval_u(timers, &timer, time_get(), time_sec_to_ns(1), 0);
	time_timers_close_interval_u(timers, &timer);

	time_timers_unlock(timers);

	thread_sleep(time_sec_to_ns(2));

	assert_true(flag);
	assert_false(time_timers_is_timer_expired(timers, &timer));
	assert_true(time_timers_cancel_interval(timers, &timer));
	assert_true(time_timers_is_timer_expired(timers, &timer));

	time_timer_free(&timer);

	time_timers_free(timers);
}


void attr_test_fn
test_priority_pass__time_timers_open_cancel_interval(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	time_interval_t interval =
	{
		.timer = &timer,
		.data =
		{
			.fn = timer_never_fn,
			.data = NULL
		},
		.base_time = time_get(),
		.count = 1,
		.interval = time_sec_to_ns(4)
	};
	time_timers_add_interval(timers, interval);

	time_interval_t* interval_ptr = time_timers_open_interval(timers, &timer);
	assert_not_null(interval_ptr);

	time_timers_cancel_interval_u(timers, &timer);
	assert_true(time_timers_is_timer_expired_u(timers, &timer));

	time_timers_close_interval(timers, &timer);
	assert_true(time_timers_is_timer_expired(timers, &timer));

	time_timer_free(&timer);

	time_timers_free(timers);
}


void
timer_interval_cancel_timer_fn(
	timer_cancel_data_t* data
	)
{
	assert_true(
		time_timers_cancel_interval(
			data->timers,
			time_timers_get_current_timer(data->timers)
			)
		);

	if(data->timeout)
	{
		assert_true(time_timers_cancel_timeout(data->timers, data->timer));
	}
	else
	{
		assert_true(time_timers_cancel_interval(data->timers, data->timer));
	}
}


void attr_test_fn
test_priority_pass__time_timers_interval_cancel_interval(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	time_interval_t interval =
	{
		.timer = &timer,
		.data =
		{
			.fn = timer_never_fn,
			.data = NULL
		},
		.base_time = time_get(),
		.count = 1,
		.interval = time_sec_to_ns(2)
	};
	time_timers_add_interval(timers, interval);

	timer_cancel_data_t data =
	{
		.timeout = false,
		.timers = timers,
		.timer = &timer
	};
	time_interval_t cancel_interval =
	{
		.data =
		{
			.fn = (void*) timer_interval_cancel_timer_fn,
			.data = &data
		},
		.base_time = TIME_IMMEDIATELY,
		.count = 0,
		.interval = 0
	};
	time_timers_add_interval(timers, cancel_interval);

	thread_sleep(time_sec_to_ns(4));

	assert_true(time_timers_is_timer_expired(timers, &timer));

	time_timer_free(&timer);

	time_timers_free(timers);
}


void
timer_cancel_itself(
	time_timers_t timers
	)
{
	assert_true(
		time_timers_cancel_interval(
			timers,
			time_timers_get_current_timer(timers)
			)
		);
}


void attr_test_fn
test_priority_fail__time_timers_interval_cancel_interval_too_late(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	uint64_t time = time_get_with_sec(2);

	time_interval_t interval =
	{
		.timer = &timer,
		.data =
		{
			.fn = (void*) timer_cancel_itself,
			.data = NULL
		},
		.base_time = time,
		.count = 0,
		.interval = 0
	};
	time_timers_add_interval(timers, interval);

	timer_cancel_data_t data =
	{
		.timeout = false,
		.timers = timers,
		.timer = &timer
	};
	time_interval_t cancel_interval =
	{
		.data =
		{
			.fn = (void*) timer_interval_cancel_timer_fn,
			.data = &data
		},
		.base_time = time,
		.count = 0,
		.interval = 0
	};
	time_timers_add_interval(timers, cancel_interval);

	thread_sleep(time_sec_to_ns(4));
}


void attr_test_fn
test_priority_pass__time_timers_interval_cancel_timeout(
	void
	)
{
	time_timers_t timers = time_timers_init();

	time_timer_t timer;
	time_timer_init(&timer);

	time_timeout_t timeout =
	{
		.timer = &timer,
		.data =
		{
			.fn = timer_never_fn,
			.data = NULL
		},
		.time = time_get_with_sec(2)
	};
	time_timers_add_timeout(timers, timeout);

	timer_cancel_data_t data =
	{
		.timeout = true,
		.timers = timers,
		.timer = &timer
	};
	time_interval_t cancel_interval =
	{
		.data =
		{
			.fn = (void*) timer_interval_cancel_timer_fn,
			.data = &data
		},
		.base_time = TIME_IMMEDIATELY,
		.count = 0,
		.interval = 0
	};
	time_timers_add_interval(timers, cancel_interval);

	thread_sleep(time_sec_to_ns(4));

	assert_true(time_timers_is_timer_expired(timers, &timer));

	time_timer_free(&timer);

	time_timers_free(timers);
}
