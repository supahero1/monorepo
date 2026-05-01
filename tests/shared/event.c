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

/*
 * Test comments:
 *
 * 1) To promote RAII, do not allow freeing a target with listeners still attached.
 *
 */

#include <tests/base.h>
#include <shared/attr.h>
#include <shared/time.h>
#include <shared/debug.h>
#include <shared/event.h>
#include <shared/macro.h>
#include <shared/threads.h>

#include <stddef.h>


void attr_test_fn
test_normal_pass__event_target_init_free(
	void
	)
{
	event_target_t target;
	event_target_init(&target);
	event_target_free(&target);
}


void attr_test_fn
test_normal_fail__event_target_init_null(
	void
	)
{
	event_target_init(NULL);
}


void attr_test_fn
test_normal_fail__event_target_free_null(
	void
	)
{
	event_target_free(NULL);
}


void attr_test_fn
test_normal_fail__event_target_add_null_fn(
	void
	)
{
	event_target_t target;
	event_target_init(&target);

	event_listener_data_t data = {0};
	event_target_add(&target, data);
}


void attr_test_fn
test_normal_pass__event_target_add_remove_listener(
	void
	)
{
	event_target_t target;
	event_target_init(&target);

	event_listener_data_t data =
	{
		.fn = (void*) 0x1
	};
	event_listener_t* listener = event_target_add(&target, data);
	event_target_del(&target, listener);

	event_target_free(&target);
}


void attr_test_fn
test_normal_fail__event_target_free_while_non_empty(
	void
	)
{
	/* 1) */

	event_target_t target;
	event_target_init(&target);

	event_listener_data_t data =
	{
		.fn = (void*) 0x1
	};
	event_target_add(&target, data);

	event_target_free(&target);
}


void attr_test_fn
test_normal_fail__event_target_add_null_target(
	void
	)
{
	event_listener_data_t data =
	{
		.fn = (void*) 0x1
	};
	event_target_add(NULL, data);
}


void attr_test_fn
test_normal_fail__event_target_del_null_target(
	void
	)
{
	event_listener_t* listener = (void*) 0x1;
	event_target_del(NULL, listener);
}


void attr_test_fn
test_normal_pass__event_target_del_null_listener(
	void
	)
{
	event_target_t target;
	event_target_init(&target);

	event_target_del(&target, NULL);
	event_target_free(&target);
}


void attr_test_fn
test_normal_pass__event_target_fire_empty(
	void
	)
{
	event_target_t target;
	event_target_init(&target);
	event_target_fire(&target, NULL);
	event_target_free(&target);
}


void
event_listener_bool_fn(
	bool* called,
	void* event_data
	)
{
	*called = true;
}


void attr_test_fn
test_normal_pass__event_target_fire(
	void
	)
{
	event_target_t target;
	event_target_init(&target);

	bool called = false;

	event_listener_data_t data =
	{
		.fn = (void*) event_listener_bool_fn,
		.data = &called
	};
	event_listener_t* listener = event_target_add(&target, data);

	event_target_fire(&target, NULL);
	assert_true(called);

	event_target_del(&target, listener);

	event_target_free(&target);
}


void attr_test_fn
test_normal_pass__event_target_fire_once(
	void
	)
{
	event_target_t target;
	event_target_init(&target);

	bool called = false;

	event_listener_data_t data =
	{
		.fn = (void*) event_listener_bool_fn,
		.data = &called
	};
	event_target_once(&target, data);

	event_target_fire(&target, NULL);
	assert_true(called);

	called = false;

	event_target_fire(&target, NULL);
	assert_false(called);

	event_target_free(&target);
}


void attr_test_fn
test_normal_pass__event_target_fire_on_removed_listener(
	void
	)
{
	event_target_t target;
	event_target_init(&target);

	bool called = false;

	event_listener_data_t data =
	{
		.fn = (void*) event_listener_bool_fn,
		.data = &called
	};
	event_listener_t* listener = event_target_add(&target, data);
	event_target_del(&target, listener);

	event_target_fire(&target, NULL);
	assert_false(called);

	event_target_free(&target);
}


void attr_test_fn
test_normal_pass__event_target_fire_on_removed_once_listener(
	void
	)
{
	event_target_t target;
	event_target_init(&target);

	bool called = false;

	event_listener_data_t data =
	{
		.fn = (void*) event_listener_bool_fn,
		.data = &called
	};
	event_listener_t* listener = event_target_once(&target, data);
	event_target_del_once(&target, listener);

	event_target_fire(&target, NULL);
	assert_false(called);

	event_target_free(&target);
}


void
event_listener_remove_itself_fn(
	event_target_t* target,
	event_listener_t* listener
	)
{
	event_target_del(target, listener);
}


void attr_test_fn
test_normal_pass__event_listener_remove_itself(
	void
	)
{
	event_target_t target;
	event_target_init(&target);

	event_listener_data_t data =
	{
		.fn = (void*) event_listener_remove_itself_fn,
		.data = &target
	};
	event_listener_t* listener = event_target_add(&target, data);

	event_target_fire(&target, listener);

	event_target_free(&target);
}


void attr_test_fn
test_normal_fail__event_listener_remove_itself_twice(
	void
	)
{
	event_target_t target;
	event_target_init(&target);

	event_listener_data_t data =
	{
		.fn = (void*) event_listener_remove_itself_fn,
		.data = &target
	};
	event_listener_t* listener = event_target_add(&target, data);

	event_target_del(&target, listener);
	event_target_del(&target, listener);

	event_target_free(&target);
}


void
event_target_wait_thread_fn(
	event_wait_state_t* state
	)
{
	void* data = event_target_wait(state);
	assert_eq(data, (void*) 0x123);
}


void attr_test_fn
test_priority_pass__event_target_wait(
	void
	)
{
	event_target_t target;
	event_target_init(&target);

	event_wait_state_t* state = event_target_init_wait(&target);

	thread_t thread;
	thread_data_t data =
	{
		.fn = (void*) event_target_wait_thread_fn,
		.data = state
	};
	thread_init(&thread, data);

	event_target_fire(&target, (void*) 0x123);

	thread_join(thread);
	thread_free(&thread);

	event_target_free(&target);
}


void attr_test_fn
test_normal_fail__event_target_wait_null(
	void
	)
{
	event_target_wait(NULL);
}


void attr_test_fn
test_normal_timeout__event_target_wait_timeout(
	void
	)
{
	test_set_timeout(1);

	event_target_t target;
	event_target_init(&target);

	event_wait_state_t* state = event_target_init_wait(&target);
	event_target_wait(state);

	assert_unreachable();
}
