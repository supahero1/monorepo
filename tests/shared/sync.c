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
#include <shared/sync.h>
#include <shared/time.h>
#include <shared/debug.h>
#include <shared/threads.h>

#include <stddef.h>


void attr_test_fn
test_normal_pass__sync_mtx_init_free(
	void
	)
{
	sync_mtx_t mtx;
	sync_mtx_init(&mtx);
	sync_mtx_free(&mtx);
}


void attr_test_fn
test_normal_pass__sync_mtx_init_recursive_free(
	void
	)
{
	sync_mtx_t mtx;
	sync_mtx_init_recursive(&mtx);
	sync_mtx_free(&mtx);
}


void attr_test_fn
test_normal_pass__sync_mtx_lock_unlock(
	void
	)
{
	sync_mtx_t mtx;
	sync_mtx_init(&mtx);

	sync_mtx_lock(&mtx);
	sync_mtx_unlock(&mtx);

	sync_mtx_free(&mtx);
}


void attr_test_fn
test_normal_pass__sync_mtx_lock_unlock_recursive(
	void
	)
{
	sync_mtx_t mtx;
	sync_mtx_init_recursive(&mtx);

	sync_mtx_lock(&mtx);
	sync_mtx_lock(&mtx);
	sync_mtx_unlock(&mtx);
	sync_mtx_unlock(&mtx);

	sync_mtx_free(&mtx);
}


void attr_test_fn
test_normal_pass__sync_mtx_try_lock_unlock(
	void
	)
{
	sync_mtx_t mtx;
	sync_mtx_init(&mtx);

	sync_mtx_try_lock(&mtx);
	sync_mtx_unlock(&mtx);

	sync_mtx_free(&mtx);
}


void attr_test_fn
test_normal_fail__sync_mtx_init_null(
	void
	)
{
	sync_mtx_init(NULL);
}


void attr_test_fn
test_normal_fail__sync_mtx_init_recursive_null(
	void
	)
{
	sync_mtx_init_recursive(NULL);
}


void attr_test_fn
test_normal_fail__sync_mtx_free_null(
	void
	)
{
	sync_mtx_free(NULL);
}


void attr_test_fn
test_normal_fail__sync_mtx_lock_null(
	void
	)
{
	sync_mtx_lock(NULL);
}


void attr_test_fn
test_normal_fail__sync_mtx_try_lock_null(
	void
	)
{
	sync_mtx_try_lock(NULL);
}


void attr_test_fn
test_normal_fail__sync_mtx_unlock_null(
	void
	)
{
	sync_mtx_unlock(NULL);
}


void attr_test_fn
test_normal_pass__sync_rwlock_init_free(
	void
	)
{
	sync_rwlock_t rwlock;
	sync_rwlock_init(&rwlock);
	sync_rwlock_free(&rwlock);
}


void attr_test_fn
test_normal_pass__sync_rwlock_rdlock_unlock(
	void
	)
{
	sync_rwlock_t rwlock;
	sync_rwlock_init(&rwlock);

	sync_rwlock_rdlock(&rwlock);
	sync_rwlock_unlock(&rwlock);

	sync_rwlock_free(&rwlock);
}


void attr_test_fn
test_normal_pass__sync_rwlock_try_rdlock_unlock(
	void
	)
{
	sync_rwlock_t rwlock;
	sync_rwlock_init(&rwlock);

	sync_rwlock_try_rdlock(&rwlock);
	sync_rwlock_unlock(&rwlock);

	sync_rwlock_free(&rwlock);
}


void attr_test_fn
test_normal_pass__sync_rwlock_wrlock_unlock(
	void
	)
{
	sync_rwlock_t rwlock;
	sync_rwlock_init(&rwlock);

	sync_rwlock_wrlock(&rwlock);
	sync_rwlock_unlock(&rwlock);

	sync_rwlock_free(&rwlock);
}


void attr_test_fn
test_normal_pass__sync_rwlock_try_wrlock_unlock(
	void
	)
{
	sync_rwlock_t rwlock;
	sync_rwlock_init(&rwlock);

	sync_rwlock_try_wrlock(&rwlock);
	sync_rwlock_unlock(&rwlock);

	sync_rwlock_free(&rwlock);
}


void attr_test_fn
test_normal_fail__sync_rwlock_init_null(
	void
	)
{
	sync_rwlock_init(NULL);
}


void attr_test_fn
test_normal_fail__sync_rwlock_free_null(
	void
	)
{
	sync_rwlock_free(NULL);
}


void attr_test_fn
test_normal_fail__sync_rwlock_rdlock_null(
	void
	)
{
	sync_rwlock_rdlock(NULL);
}


void attr_test_fn
test_normal_fail__sync_rwlock_try_rdlock_null(
	void
	)
{
	sync_rwlock_try_rdlock(NULL);
}


void attr_test_fn
test_normal_fail__sync_rwlock_wrlock_null(
	void
	)
{
	sync_rwlock_wrlock(NULL);
}


void attr_test_fn
test_normal_fail__sync_rwlock_try_wrlock_null(
	void
	)
{
	sync_rwlock_try_wrlock(NULL);
}


void attr_test_fn
test_normal_fail__sync_rwlock_unlock_null(
	void
	)
{
	sync_rwlock_unlock(NULL);
}


void attr_test_fn
test_normal_pass__sync_sem_init_free(
	void
	)
{
	sync_sem_t sem;
	sync_sem_init(&sem, 0);
	sync_sem_free(&sem);
}


void attr_test_fn
test_normal_pass__sync_sem_wait_post(
	void
	)
{
	sync_sem_t sem;
	sync_sem_init(&sem, 0);

	sync_sem_post(&sem);
	sync_sem_wait(&sem);

	sync_sem_free(&sem);
}


void attr_test_fn
test_normal_fail__sync_sem_init_null(
	void
	)
{
	sync_sem_init(NULL, 0);
}


void attr_test_fn
test_normal_fail__sync_sem_free_null(
	void
	)
{
	sync_sem_free(NULL);
}


void attr_test_fn
test_normal_fail__sync_sem_wait_null(
	void
	)
{
	sync_sem_wait(NULL);
}


void attr_test_fn
test_normal_fail__sync_sem_post_null(
	void
	)
{
	sync_sem_post(NULL);
}


void attr_test_fn
test_normal_pass__sync_cond_init_free(
	void
	)
{
	sync_cond_t cond;
	sync_cond_init(&cond);
	sync_cond_free(&cond);
}


typedef struct thread_cond_wake_data
{
	sync_cond_t* cond;
	sync_mtx_t* mtx;
}
thread_cond_wake_data_t;


void
thread_cond_wake_fn(
	thread_cond_wake_data_t* data
	)
{
	while(1)
	{
		sync_cond_wake(data->cond);
		thread_sleep(time_ms_to_ns(10));
	}
}


void attr_test_fn
test_normal_pass__sync_cond_wait_wake(
	void
	)
{
	sync_cond_t cond;
	sync_cond_init(&cond);

	sync_mtx_t mtx;
	sync_mtx_init(&mtx);

	thread_cond_wake_data_t thread_data =
	{
		.cond = &cond,
		.mtx = &mtx
	};
	thread_data_t data =
	{
		.fn = (void*) thread_cond_wake_fn,
		.data = &thread_data
	};
	thread_t thread;
	thread_init(&thread, data);

	sync_mtx_lock(&mtx);
	sync_cond_wait(&cond, &mtx);
	sync_mtx_unlock(&mtx);

	thread_cancel_sync(thread);
	thread_free(&thread);

	sync_mtx_free(&mtx);
	sync_cond_free(&cond);
}


void attr_test_fn
test_normal_fail__sync_cond_init_null(
	void
	)
{
	sync_cond_init(NULL);
}


void attr_test_fn
test_normal_fail__sync_cond_free_null(
	void
	)
{
	sync_cond_free(NULL);
}


void attr_test_fn
test_normal_fail__sync_cond_wait_cond_null(
	void
	)
{
	sync_cond_wait(NULL, (void*) 0x1);
}


void attr_test_fn
test_normal_fail__sync_cond_wait_mtx_null(
	void
	)
{
	sync_cond_wait((void*) 0x1, NULL);
}


void attr_test_fn
test_normal_fail__sync_cond_wait_null(
	void
	)
{
	sync_cond_wait(NULL, NULL);
}


void attr_test_fn
test_normal_fail__sync_cond_wake_null(
	void
	)
{
	sync_cond_wake(NULL);
}
