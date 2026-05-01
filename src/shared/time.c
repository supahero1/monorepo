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

#include <shared/sync.h>
#include <shared/time.h>
#include <shared/debug.h>
#include <shared/macro.h>
#include <shared/atomic.h>
#include <shared/threads.h>
#include <shared/alloc/base.h>

#include <time.h>
#include <stdatomic.h>


uint64_t
time_sec_to_ms(
	uint64_t sec
	)
{
	return sec * 1000;
}


uint64_t
time_sec_to_us(
	uint64_t sec
	)
{
	return sec * 1000000;
}


uint64_t
time_sec_to_ns(
	uint64_t sec
	)
{
	return sec * 1000000000;
}


uint64_t
time_ms_to_sec(
	uint64_t ms
	)
{
	return ms / 1000;
}


uint64_t
time_ms_to_us(
	uint64_t ms
	)
{
	return ms * 1000;
}


uint64_t
time_ms_to_ns(
	uint64_t ms
	)
{
	return ms * 1000000;
}


uint64_t
time_us_to_sec(
	uint64_t us
	)
{
	return us / 1000000;
}


uint64_t
time_us_to_ms(
	uint64_t us
	)
{
	return us / 1000;
}


uint64_t
time_us_to_ns(
	uint64_t us
	)
{
	return us * 1000;
}


uint64_t
time_ns_to_sec(
	uint64_t ns
	)
{
	return ns / 1000000000;
}


uint64_t
time_ns_to_ms(
	uint64_t ns
	)
{
	return ns / 1000000;
}


uint64_t
time_ns_to_us(
	uint64_t ns
	)
{
	return ns / 1000;
}


uint64_t
time_get(
	void
	)
{
	struct timespec time;
	int status = clock_gettime(CLOCK_REALTIME, &time);
	hard_assert_eq(status, 0);

	return time.tv_sec * 1000000000 + time.tv_nsec;
}


uint64_t
time_get_with_sec(
	uint64_t sec
	)
{
	return time_get() + time_sec_to_ns(sec);
}


uint64_t
time_get_with_ms(
	uint64_t ms
	)
{
	return time_get() + time_ms_to_ns(ms);
}


uint64_t
time_get_with_us(
	uint64_t us
	)
{
	return time_get() + time_us_to_ns(us);
}


uint64_t
time_get_with_ns(
	uint64_t ns
	)
{
	return time_get() + ns;
}





struct time_timers
{
	time_timeout_t* timeouts;
	uint32_t timeouts_used;
	uint32_t timeouts_size;

	time_interval_t* intervals;
	uint32_t intervals_used;
	uint32_t intervals_size;

	thread_t thread;
	sync_mtx_t mtx;
	sync_sem_t work_sem;
	sync_sem_t updates_sem;

	time_timer_t* current_timer;

	uint64_t _Atomic latest;
};


uint64_t
time_timers_get_latest(
	time_timers_t timers
	)
{
	return atomic_load_acq(&timers->latest);
}


#define time_of_timeout_idx(idx)			\
(											\
	timers->timeouts[idx].time				\
	)

#define time_of_interval_idx(idx)			\
(											\
	timers->intervals[idx].base_time		\
		+ timers->intervals[idx].interval	\
		* timers->intervals[idx].count		\
	)


void
time_timers_set_latest(
	time_timers_t timers
	)
{
	uint64_t old = time_timers_get_latest(timers);
	uint64_t latest = UINT64_MAX;

	if(timers->timeouts_used > 1)
	{
		latest = MACRO_MIN(latest, time_of_timeout_idx(1) & (~1));
	}

	if(timers->intervals_used > 1)
	{
		latest = MACRO_MIN(latest, time_of_interval_idx(1) | 1);
	}

	if(latest == UINT64_MAX)
	{
		latest = 0;
	}

	atomic_store_rel(&timers->latest, latest);

	if(old != latest)
	{
		sync_sem_post(&timers->updates_sem);
	}
}


#define TIME_TIMER_DEF(name, names)																\
																								\
void																							\
time_timers_swap_##names (																		\
	time_timers_t timers,																		\
	uint32_t idx_a,																				\
	uint32_t idx_b																				\
	)																							\
{																								\
	timers-> names [idx_a] = timers-> names [idx_b];											\
																								\
	if(timers-> names [idx_a].timer != NULL)													\
	{																							\
		timers-> names [idx_a].timer-> idx = idx_a;												\
	}																							\
}																								\
																								\
																								\
void																							\
time_timers_##names##_down (																	\
	time_timers_t timers,																		\
	uint32_t timer_idx																			\
	)																							\
{																								\
	uint32_t save_idx = timer_idx;																\
																								\
	timers-> names [0] = timers-> names [timer_idx];											\
																								\
	while(1)																					\
	{																							\
		uint32_t left_child_idx = timer_idx << 1;												\
		uint32_t right_child_idx = left_child_idx + 1;											\
		uint32_t smallest_idx = 0;																\
																								\
		if(left_child_idx < timers-> names##_used &&											\
			time_of_##name##_idx (left_child_idx) < time_of_##name##_idx (smallest_idx))		\
		{																						\
			smallest_idx = left_child_idx;														\
		}																						\
																								\
		if(right_child_idx < timers-> names##_used &&											\
			time_of_##name##_idx (right_child_idx) < time_of_##name##_idx (smallest_idx))		\
		{																						\
			smallest_idx = right_child_idx;														\
		}																						\
																								\
		if(smallest_idx == 0)																	\
		{																						\
			break;																				\
		}																						\
																								\
		time_timers_swap_##names (timers, timer_idx, smallest_idx);								\
		timer_idx = smallest_idx;																\
	}																							\
																								\
	if(save_idx != timer_idx)																	\
	{																							\
		time_timers_swap_##names (timers, timer_idx, 0);										\
	}																							\
}																								\
																								\
																								\
bool																							\
time_timers_##names##_up (																		\
	time_timers_t timers,																		\
	uint32_t timer_idx																			\
	)																							\
{																								\
	uint32_t save_idx = timer_idx;																\
	uint32_t parent_idx = timer_idx >> 1;														\
																								\
	timers-> names [0] = timers-> names [timer_idx];											\
																								\
	while(																						\
		parent_idx > 0 &&																		\
		time_of_##name##_idx (parent_idx) > time_of_##name##_idx (0)							\
		)																						\
	{																							\
		time_timers_swap_##names (timers, timer_idx, parent_idx);								\
		timer_idx = parent_idx;																	\
		parent_idx >>= 1;																		\
	}																							\
																								\
	if(save_idx != timer_idx)																	\
	{																							\
		time_timers_swap_##names (timers, timer_idx, 0);										\
		return true;																			\
	}																							\
																								\
	return false;																				\
}																								\
																								\
																								\
void																							\
time_timers_free_##names (																		\
	time_timers_t timers																		\
	)																							\
{																								\
	alloc_free(timers-> names , timers-> names##_size );										\
}																								\
																								\
																								\
void																							\
time_timers_resize_##names (																	\
	time_timers_t timers,																		\
	uint32_t count																				\
	)																							\
{																								\
	uint32_t new_used = timers-> names##_used + count;											\
																								\
	if((new_used < (timers-> names##_size >> 2)) || (new_used > timers-> names##_size ))		\
	{																							\
		uint32_t new_size = (new_used << 1) | 3;												\
		assert_neq(new_size, timers-> names##_size);											\
																								\
		timers-> names = alloc_remalloc(timers-> names , timers-> names##_size , new_size);		\
		assert_not_null(timers-> names);														\
																								\
		timers-> names##_size = new_size;														\
	}																							\
}																								\
																								\
																								\
void																							\
time_timers_add_##name##_common (																\
	time_timers_t timers,																		\
	time_##name##_t name,																		\
	bool lock																					\
	)																							\
{																								\
	assert_not_null(timers);																	\
	assert_not_null( name .data.fn);															\
																								\
	if(lock)																					\
	{																							\
		time_timers_lock(timers);																\
	}																							\
																								\
	time_timers_resize_##names (timers, 1);														\
																								\
	if( name .timer != NULL)																	\
	{																							\
		name .timer->idx = timers-> names##_used ;												\
	}																							\
																								\
	timers-> names [timers-> names##_used ++] = name ;											\
																								\
	time_timers_##names##_up (timers, timers-> names##_used - 1);								\
																								\
	time_timers_set_latest(timers);																\
																								\
	if(lock)																					\
	{																							\
		time_timers_unlock(timers);																\
	}																							\
																								\
	sync_sem_post(&timers->work_sem);															\
}																								\
																								\
																								\
void																							\
time_timers_add_##name##_u (																	\
	time_timers_t timers,																		\
	time_##name##_t name																		\
	)																							\
{																								\
	time_timers_add_##name##_common (timers, name , false);										\
}																								\
																								\
																								\
void																							\
time_timers_add_##name (																		\
	time_timers_t timers,																		\
	time_##name##_t name																		\
	)																							\
{																								\
	time_timers_add_##name##_common (timers, name , true);										\
}																								\
																								\
																								\
bool																							\
time_timers_cancel_##name##_u (																	\
	time_timers_t timers,																		\
	time_timer_t* timer																			\
	)																							\
{																								\
	if(time_timers_is_timer_expired_u(timers, timer))											\
	{																							\
		return false;																			\
	}																							\
																								\
	timers-> names [timer->idx] = timers-> names [-- timers-> names##_used ];					\
																								\
	if(! time_timers_##names##_up (timers, timer->idx))											\
	{																							\
		time_timers_##names##_down (timers, timer->idx);										\
	}																							\
																								\
	time_timers_set_latest(timers);																\
																								\
	time_timer_init(timer);																		\
																								\
	return true;																				\
}																								\
																								\
																								\
bool																							\
time_timers_cancel_##name (																		\
	time_timers_t timers,																		\
	time_timer_t* timer																			\
	)																							\
{																								\
	time_timers_lock(timers);																	\
		bool status = time_timers_cancel_##name##_u (timers, timer);							\
	time_timers_unlock(timers);																	\
																								\
	return status;																				\
}																								\
																								\
																								\
time_##name##_t *																				\
time_timers_open_##name##_u (																	\
	time_timers_t timers,																		\
	time_timer_t* timer																			\
	)																							\
{																								\
	if(time_timers_is_timer_expired_u(timers, timer))											\
	{																							\
		return NULL;																			\
	}																							\
																								\
	return &timers-> names [timer->idx];														\
}																								\
																								\
																								\
time_##name##_t *																				\
time_timers_open_##name (																		\
	time_timers_t timers,																		\
	time_timer_t* timer																			\
	)																							\
{																								\
	time_timers_lock(timers);																	\
																								\
	time_##name##_t * result = time_timers_open_##name##_u (timers, timer);						\
																								\
	if(!result)																					\
	{																							\
		time_timers_unlock(timers);																\
	}																							\
																								\
	return result;																				\
}																								\
																								\
																								\
void																							\
time_timers_close_##name##_u (																	\
	time_timers_t timers,																		\
	time_timer_t* timer																			\
	)																							\
{																								\
	if(time_timers_is_timer_expired_u(timers, timer))											\
	{																							\
		return;																					\
	}																							\
																								\
	if(! time_timers_##names##_up (timers, timer->idx))											\
	{																							\
		time_timers_##names##_down (timers, timer->idx);										\
	}																							\
																								\
	time_timers_set_latest(timers);																\
}																								\
																								\
																								\
void																							\
time_timers_close_##name (																		\
	time_timers_t timers,																		\
	time_timer_t* timer																			\
	)																							\
{																								\
	time_timers_close_##name##_u (timers, timer);												\
	time_timers_unlock(timers);																	\
}																								\
																								\
																								\
void																							\
time_timers_update_##name##_timer_u (															\
	time_timers_t timers,																		\
	time_##name##_t * name																		\
	)																							\
{																								\
	assert_not_null(timers);																	\
	assert_not_null( name );																	\
																								\
	name ->timer->idx = name - timers-> names ;													\
}


TIME_TIMER_DEF(timeout, timeouts)
TIME_TIMER_DEF(interval, intervals)

#undef TIME_TIMER_DEF





uint64_t
time_timer_get_timeout_u(
	time_timeout_t* timeout
	)
{
	assert_not_null(timeout);

	return timeout->time;
}


uint64_t
time_timers_get_timeout_u(
	time_timers_t timers,
	time_timer_t* timer
	)
{
	time_timeout_t* timeout = time_timers_open_timeout_u(timers, timer);
	if(!timeout)
	{
		return 0;
	}

	uint64_t time = time_timer_get_timeout_u(timeout);
	time_timers_close_timeout_u(timers, timer);

	return time;
}


uint64_t
time_timers_get_timeout(
	time_timers_t timers,
	time_timer_t* timer
	)
{
	time_timers_lock(timers);
		uint64_t time = time_timers_get_timeout_u(timers, timer);
	time_timers_unlock(timers);

	return time;
}


void
time_timer_set_timeout_u(
	time_timeout_t* timeout,
	uint64_t time
	)
{
	assert_not_null(timeout);

	timeout->time = time;
}


bool
time_timers_set_timeout_u(
	time_timers_t timers,
	time_timer_t* timer,
	uint64_t time
	)
{
	time_timeout_t* timeout = time_timers_open_timeout_u(timers, timer);
	if(!timeout)
	{
		return false;
	}

	time_timer_set_timeout_u(timeout, time);
	time_timers_close_timeout_u(timers, timer);

	return true;
}


bool
time_timers_set_timeout(
	time_timers_t timers,
	time_timer_t* timer,
	uint64_t time
	)
{
	time_timers_lock(timers);
		bool status = time_timers_set_timeout_u(timers, timer, time);
	time_timers_unlock(timers);

	return status;
}





uint64_t
time_timer_get_interval_u(
	time_interval_t* interval
	)
{
	assert_not_null(interval);
	assert_gt(interval->base_time, TIME_IMMEDIATELY);

	return interval->base_time + interval->interval * interval->count;
}


uint64_t
time_timers_get_interval_u(
	time_timers_t timers,
	time_timer_t* timer
	)
{
	time_interval_t* interval = time_timers_open_interval_u(timers, timer);
	if(!interval)
	{
		return 0;
	}

	uint64_t time = time_timer_get_interval_u(interval);
	time_timers_close_interval_u(timers, timer);

	return time;
}


uint64_t
time_timers_get_interval(
	time_timers_t timers,
	time_timer_t* timer
	)
{
	time_timers_lock(timers);
		uint64_t time = time_timers_get_interval_u(timers, timer);
	time_timers_unlock(timers);

	return time;
}


void
time_timer_set_interval_u(
	time_interval_t* interval,
	uint64_t base_time,
	uint64_t interval_time,
	uint64_t count
	)
{
	assert_not_null(interval);
	assert_gt(interval->base_time, TIME_IMMEDIATELY);

	interval->base_time = base_time;
	interval->interval = interval_time;
	interval->count = count;
}


bool
time_timers_set_interval_u(
	time_timers_t timers,
	time_timer_t* timer,
	uint64_t base_time,
	uint64_t interval_time,
	uint64_t count
	)
{
	time_interval_t* interval = time_timers_open_interval_u(timers, timer);
	if(!interval)
	{
		return false;
	}

	time_timer_set_interval_u(interval, base_time, interval_time, count);
	time_timers_close_interval_u(timers, timer);

	return true;
}


bool
time_timers_set_interval(
	time_timers_t timers,
	time_timer_t* timer,
	uint64_t base_time,
	uint64_t interval_time,
	uint64_t count
	)
{
	time_timers_lock(timers);
		bool status = time_timers_set_interval_u(timers, timer, base_time, interval_time, count);
	time_timers_unlock(timers);

	return status;
}





void
time_timers_fn(
	void* data
	)
{
	assert_not_null(data);
	time_timers_t timers = data;

	while(1)
	{
		goto_start:

		sync_sem_wait(&timers->work_sem);

		uint64_t time;

		while(1)
		{
			time = time_timers_get_latest(timers);

			if(time == 0)
			{
				goto goto_start;
			}

			if(time_get() >= time)
			{
				break;
			}

			sync_sem_timed_wait(&timers->updates_sem, time);
		}

		time_data_t data;

		time_timers_lock(timers);

		time = time_timers_get_latest(timers);

		if(time == 0 || time_get() < time)
		{
			time_timers_unlock(timers);
			goto goto_start;
		}

		time_timer_t current_timer;
		time_timer_init(&current_timer);

		timers->current_timer = NULL;

		if(time & 1)
		{
			time_interval_t* interval = &timers->intervals[1];
			data = interval->data;

			if(interval->timer)
			{
				timers->current_timer = interval->timer;
			}
			else
			{
				interval->timer = &current_timer;
				time_timers_update_interval_timer_u(timers, interval);
			}

			++interval->count;
			time_timers_intervals_down(timers, 1);
			sync_sem_post(&timers->work_sem);

			if(interval->timer == &current_timer)
			{
				interval->timer = NULL;
				timers->current_timer = &current_timer;
			}
		}
		else
		{
			time_timeout_t* timeout = &timers->timeouts[1];
			data = timeout->data;

			if(timeout->timer != NULL)
			{
				time_timer_init(timeout->timer);
			}

			if(--timers->timeouts_used > 1)
			{
				time_timers_swap_timeouts(timers, 1, timers->timeouts_used);
				time_timers_timeouts_down(timers, 1);
			}
		}

		time_timers_set_latest(timers);

		time_timers_unlock(timers);

		if(data.fn != NULL)
		{
			thread_cancel_off();
			data.fn(data.data);
			thread_cancel_on();
		}
	}
}


time_timers_t
time_timers_init(
	void
	)
{
	time_timers_t timers = alloc_malloc(timers, 1);
	assert_ptr(timers, 1);

	timers->timeouts = NULL;
	timers->timeouts_used = 1;
	timers->timeouts_size = 0;

	timers->intervals = NULL;
	timers->intervals_used = 1;
	timers->intervals_size = 0;

	atomic_init(&timers->latest, 0);

	sync_mtx_init(&timers->mtx);
	sync_sem_init(&timers->work_sem, 0);
	sync_sem_init(&timers->updates_sem, 0);

	timers->current_timer = NULL;

	thread_data_t data =
	{
		.fn = time_timers_fn,
		.data = timers
	};
	thread_init(&timers->thread, data);

	return timers;
}


void
time_timers_free(
	time_timers_t timers
	)
{
	assert_not_null(timers);

	thread_cancel_sync(timers->thread);
	thread_free(&timers->thread);

	sync_sem_free(&timers->updates_sem);
	sync_sem_free(&timers->work_sem);
	sync_mtx_free(&timers->mtx);

	time_timers_free_intervals(timers);
	time_timers_free_timeouts(timers);

	alloc_free(timers, 1);
}


void
time_timers_lock(
	time_timers_t timers
	)
{
	assert_not_null(timers);

	sync_mtx_lock(&timers->mtx);
}


void
time_timers_unlock(
	time_timers_t timers
	)
{
	assert_not_null(timers);

	sync_mtx_unlock(&timers->mtx);
}


time_timer_t*
time_timers_get_current_timer(
	time_timers_t timers
	)
{
	assert_not_null(timers);

	return timers->current_timer;
}





void
time_timer_init(
	time_timer_t* timer
	)
{
	assert_not_null(timer);

	timer->idx = 0;
}


bool
time_timers_is_timer_expired_u(
	time_timers_t timers,
	time_timer_t* timer
	)
{
	assert_not_null(timers);
	assert_not_null(timer);

	return timer->idx == 0;
}


bool
time_timers_is_timer_expired(
	time_timers_t timers,
	time_timer_t* timer
	)
{
	assert_not_null(timers);

	sync_mtx_lock(&timers->mtx);
		bool status = time_timers_is_timer_expired_u(timers, timer);
	sync_mtx_unlock(&timers->mtx);

	return status;
}


void
time_timer_free(
	time_timer_t* timer
	)
{
	assert_not_null(timer);

	assert_eq(timer->idx, 0);
}
