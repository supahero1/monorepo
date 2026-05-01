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

#include <tests/base.h>
#include <shared/attr.h>
#include <shared/sync.h>
#include <shared/time.h>
#include <shared/debug.h>
#include <shared/macro.h>
#include <shared/atomic.h>
#include <shared/threads.h>

#include <stddef.h>
#include <stdint.h>


uint32_t _Atomic thread_once_counter;


void
thread_once_fn(
	void
	)
{
	atomic_fetch_add_rx(&thread_once_counter, 1);
}


void
thread_once_thread_fn(
	void* data
	)
{
	thread_once_t* once = data;

	thread_once(once, thread_once_fn);
	thread_once(once, thread_once_fn);
}


void attr_test_fn
test_normal_pass__thread_once(
	void
	)
{
	thread_once_t once = THREAD_ONCE_INIT;
	atomic_store_rx(&thread_once_counter, 0);

	thread_t threads[32];

	for(uint32_t i = 0; i < 32; i++)
	{
		thread_data_t data =
		{
			.fn = (void*) thread_once_thread_fn,
			.data = &once
		};

		thread_init(&threads[i], data);
	}

	for(uint32_t i = 0; i < 32; i++)
	{
		thread_join(threads[i]);
		thread_free(&threads[i]);
	}

	assert_eq(thread_once_counter, 1);
}


void attr_test_fn
test_normal_pass__thread_once_reuse(
	void
	)
{
	thread_once_t once = THREAD_ONCE_INIT;
	atomic_store_rx(&thread_once_counter, 0);

	for(uint32_t i = 0; i < 1024; i++)
	{
		thread_once(&once, thread_once_fn);
	}

	assert_eq(thread_once_counter, 1);
}


typedef struct thread_key_data
{
	thread_key_t key;
	void* value;
	bool keep;
}
thread_key_data_t;

uint32_t _Atomic thread_key_dtor_count;
uintptr_t _Atomic thread_key_dtor_sum;


void
thread_key_dtor_fn(
	void* value
	)
{
	hard_assert_not_null(value);
	atomic_fetch_add_rx(&thread_key_dtor_count, 1);
	atomic_fetch_add_rx(&thread_key_dtor_sum, (uintptr_t) value);
}


void
thread_key_thread_fn(
	thread_key_data_t* data
	)
{
	assert_null(thread_key_get(data->key));

	thread_key_set(data->key, data->value);
	assert_eq(thread_key_get(data->key), data->value);

	thread_key_set(data->key, data->value);
	assert_eq(thread_key_get(data->key), data->value);

	if(!data->keep)
	{
		thread_key_set(data->key, NULL);
		assert_null(thread_key_get(data->key));
	}
}


void attr_test_fn
test_normal_pass__thread_key_local(
	void
	)
{
	thread_key_t key = thread_key_create(NULL);

	assert_null(thread_key_get(key));

	uint32_t local = 7;
	thread_key_set(key, &local);
	assert_eq(thread_key_get(key), &local);

	thread_key_set(key, NULL);
	assert_null(thread_key_get(key));

	thread_key_free(key);
}


void attr_test_fn
test_normal_pass__thread_key_xthread_isolation(
	void
	)
{
	thread_key_t key = thread_key_create(thread_key_dtor_fn);
	thread_t threads[32];
	thread_key_data_t data[32];
	uint32_t values[32];

	atomic_store_rx(&thread_key_dtor_count, 0);
	atomic_store_rx(&thread_key_dtor_sum, 0);

	for(uint32_t i = 0; i < 32; i++)
	{
		values[i] = i + 1;
		data[i] =
		(thread_key_data_t)
		{
			.key = key,
			.value = &values[i],
			.keep = i & 1
		};

		thread_data_t thread_data =
		{
			.fn = (void*) thread_key_thread_fn,
			.data = &data[i]
		};

		thread_init(&threads[i], thread_data);
	}

	for(uint32_t i = 0; i < 32; i++)
	{
		thread_join(threads[i]);
		thread_free(&threads[i]);
	}

	assert_null(thread_key_get(key));
	assert_eq(thread_key_dtor_count, 16);

	uintptr_t expected_sum = 0;
	for(uint32_t i = 0; i < 32; i++)
	{
		if(data[i].keep)
		{
			expected_sum += (uintptr_t) data[i].value;
		}
	}

	assert_eq(thread_key_dtor_sum, expected_sum);

	thread_key_free(key);
}


void attr_test_fn
test_normal_fail__thread_key_free_freed(
	void
	)
{
	thread_key_t key = thread_key_create(NULL);
	thread_key_free(key);
	thread_key_free(key);
}


void attr_test_fn
test_normal_fail__thread_key_set_freed(
	void
	)
{
	thread_key_t key = thread_key_create(NULL);
	thread_key_free(key);
	thread_key_set(key, NULL);
}


void attr_test_fn
test_normal_fail__thread_init_null_fn(
	void
	)
{
	thread_data_t data = {0};
	thread_init(NULL, data);
}


void
dummy_thread_fn(
	void* data
	)
{
	(void) data;
}


void attr_test_fn
test_normal_pass__thread_init_free(
	void
	)
{
	thread_t thread;
	thread_data_t data =
	{
		.fn = dummy_thread_fn
	};
	thread_init(&thread, data);
	thread_join(thread);
	thread_free(&thread);
}


void attr_test_fn
test_normal_fail__thread_free_null(
	void
	)
{
	thread_free(NULL);
}


void
thread_cancel_off_fn(
	void* data
	)
{
	(void) data;

	thread_cancel_off();
	thread_sleep(time_sec_to_ns(99));
	assert_unreachable();
}


void attr_test_fn
test_normal_timeout__thread_cancel_off(
	void
	)
{
	test_set_timeout(1);

	thread_t thread;
	thread_data_t data =
	{
		.fn = thread_cancel_off_fn
	};
	thread_init(&thread, data);

	thread_sleep(time_ms_to_ns(10));
	thread_cancel_sync(thread);

	thread_free(&thread);
}


void
thread_cancel_on_fn(
	void* data
	)
{
	(void) data;

	thread_cancel_on();
	thread_sleep(time_sec_to_ns(99));
}


void attr_test_fn
test_normal_pass__thread_cancel_on(
	void
	)
{
	thread_t thread;
	thread_data_t data =
	{
		.fn = thread_cancel_on_fn
	};
	thread_init(&thread, data);

	thread_sleep(time_ms_to_ns(10));
	thread_cancel_sync(thread);

	thread_free(&thread);
}


void attr_test_fn
test_normal_timeout__thread_cancel_off_self(
	void
	)
{
	test_set_timeout(1);

	thread_cancel_off();
	thread_cancel_async(thread_self());
	thread_sleep(time_sec_to_ns(99));
	assert_unreachable();
}


void attr_test_fn
test_normal_pass__thread_cancel_on_self(
	void
	)
{
	thread_cancel_on();
	thread_cancel_async(thread_self());
	thread_sleep(time_sec_to_ns(99));
}


void attr_test_fn
test_normal_fail__thread_detach_freed(
	void
	)
{
	thread_t thread;
	thread_data_t data =
	{
		.fn = dummy_thread_fn
	};
	thread_init(&thread, data);
	thread_free(&thread);

	thread_detach(thread);
}


void attr_test_fn
test_normal_fail__thread_join_freed(
	void
	)
{
	thread_t thread;
	thread_data_t data =
	{
		.fn = dummy_thread_fn
	};
	thread_init(&thread, data);
	thread_free(&thread);

	thread_join(thread);
}


void attr_test_fn
test_normal_fail__thread_cancel_freed(
	void
	)
{
	thread_t thread;
	thread_data_t data =
	{
		.fn = dummy_thread_fn
	};
	thread_init(&thread, data);
	thread_free(&thread);

	thread_cancel_sync(thread);
}


void
sync_thread_fn(
	sync_sem_t* sem
	)
{
	sync_sem_post(sem);
}


void attr_test_fn
test_normal_pass__thread_auto_detach(
	void
	)
{
	sync_sem_t sem;
	sync_sem_init(&sem, 0);

	thread_data_t data =
	{
		.fn = (void*) sync_thread_fn,
		.data = &sem
	};

	thread_init(NULL, data);

	sync_sem_wait(&sem);
	sync_sem_free(&sem);
}


typedef struct thread_pool_work_data
{
	uint32_t _Atomic counter;
	uint32_t max;
	sync_sem_t sem;
}
thread_pool_work_data_t;


void
thread_pool_work_fn(
	thread_pool_work_data_t* data
	)
{
	if(atomic_fetch_add_rx(&data->counter, 1) + 1 == data->max)
	{
		sync_sem_post(&data->sem);
	}
}


void attr_test_fn
test_normal_pass__thread_pool_and_threads(
	void
	)
{
	thread_pool_work_data_t data =
	{
		.counter = 0,
		.max = 100
	};
	sync_sem_init(&data.sem, 0);

	thread_pool_t pool;
	thread_pool_init(&pool);

	thread_data_t thread_data =
	{
		.fn = (void*) thread_pool_work_fn,
		.data = &data
	};

	thread_pool_lock(&pool);

	for(uint32_t i = 0; i < data.max; i++)
	{
		thread_pool_add_u(&pool, thread_data);
	}

	thread_pool_unlock(&pool);

	threads_t threads;
	threads_init(&threads);

	thread_data_t threads_data =
	{
		.fn = thread_pool_fn,
		.data = &pool
	};

	threads_add(&threads, threads_data, 16);

	sync_sem_wait(&data.sem);
	sync_sem_free(&data.sem);

	threads_cancel_all_sync(&threads);
	threads_free(&threads);

	thread_pool_free(&pool);

	assert_eq(data.counter, data.max);
}


void
thread_pool_work_manually_fn(
	uint32_t* counter
	)
{
	(*counter)++;
}


void attr_test_fn
test_normal_pass__thread_pool_try_work(
	void
	)
{
	thread_pool_t pool;
	thread_pool_init(&pool);

	uint32_t counter = 0;

	thread_data_t thread_data =
	{
		.fn = (void*) thread_pool_work_manually_fn,
		.data = &counter
	};

	for(uint32_t i = 0; i < 100; i++)
	{
		thread_pool_add(&pool, thread_data);
	}

	while(thread_pool_try_work(&pool));

	assert_eq(counter, 100);

	thread_pool_add_u(&pool, thread_data);
	bool status = thread_pool_try_work_u(&pool);
	assert_true(status);

	thread_pool_lock(&pool);
	thread_pool_unlock(&pool);

	thread_pool_free(&pool);

	assert_eq(counter, 101);
}


void attr_test_fn
test_normal_fail__threads_init_null(
	void
	)
{
	threads_init(NULL);
}


void attr_test_fn
test_normal_fail__threads_free_null(
	void
	)
{
	threads_free(NULL);
}


void attr_test_fn
test_normal_fail__threads_add_null(
	void
	)
{
	threads_add(NULL, (thread_data_t){0}, 0);
}


void attr_test_fn
test_normal_fail__threads_cancel_sync_null(
	void
	)
{
	threads_cancel_sync(NULL, 0);
}


void attr_test_fn
test_normal_fail__threads_cancel_sync_too_many(
	void
	)
{
	threads_t threads;
	threads_init(&threads);

	threads_cancel_sync(&threads, 1);
}


void attr_test_fn
test_normal_fail__threads_cancel_async_null(
	void
	)
{
	threads_cancel_async(NULL, 0);
}


void attr_test_fn
test_normal_fail__threads_cancel_async_too_many(
	void
	)
{
	threads_t threads;
	threads_init(&threads);

	threads_cancel_async(&threads, 1);
}


void attr_test_fn
test_normal_pass__threads_cancel_zero(
	void
	)
{
	threads_t threads;
	threads_init(&threads);

	threads_cancel_sync(&threads, 0);
	threads_cancel_async(&threads, 0);

	threads_free(&threads);
}





void attr_test_fn
test_normal_fail__thread_pool_init_null(
	void
	)
{
	thread_pool_init(NULL);
}


void attr_test_fn
test_normal_fail__thread_pool_free_null(
	void
	)
{
	thread_pool_free(NULL);
}


void attr_test_fn
test_normal_fail__thread_pool_lock_null(
	void
	)
{
	thread_pool_lock(NULL);
}


void attr_test_fn
test_normal_fail__thread_pool_unlock_null(
	void
	)
{
	thread_pool_unlock(NULL);
}


void attr_test_fn
test_normal_fail__thread_pool_add_null(
	void
	)
{
	thread_pool_add(NULL, (thread_data_t){0});
}


void attr_test_fn
test_normal_fail__thread_pool_add_u_null(
	void
	)
{
	thread_pool_add_u(NULL, (thread_data_t){0});
}


void attr_test_fn
test_normal_fail__thread_pool_add_null_fn(
	void
	)
{
	thread_pool_t pool;
	thread_pool_init(&pool);

	thread_pool_add(&pool, (thread_data_t){0});
}


void attr_test_fn
test_normal_fail__thread_pool_add_u_null_fn(
	void
	)
{
	thread_pool_t pool;
	thread_pool_init(&pool);

	thread_pool_add_u(&pool, (thread_data_t){0});
}


void attr_test_fn
test_normal_fail__thread_pool_try_work_null(
	void
	)
{
	thread_pool_try_work(NULL);
}


void attr_test_fn
test_normal_fail__thread_pool_try_work_u_null(
	void
	)
{
	thread_pool_try_work_u(NULL);
}


void attr_test_fn
test_normal_fail__thread_pool_work_null(
	void
	)
{
	thread_pool_work(NULL);
}


void attr_test_fn
test_normal_fail__thread_pool_work_u_null(
	void
	)
{
	thread_pool_work_u(NULL);
}
