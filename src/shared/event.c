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
#include <shared/debug.h>
#include <shared/event.h>
#include <shared/macro.h>
#include <shared/alloc/base.h>
#include <shared/alloc/types.h>

#include <stddef.h>


void
event_target_init(
	event_target_t* target
	)
{
	assert_not_null(target);

	target->head = NULL;
}


void
event_target_free(
	event_target_t* target
	)
{
	assert_not_null(target);

	if(target->head)
	{
		assert_unreachable();
	}
}


typedef struct event_once_payload
{
	event_target_t* target;
	event_listener_data_t data;
}
event_once_payload_t;

typedef struct event_once_data
{
	event_listener_t listener;
	event_once_payload_t once;
}
event_once_data_t;


void
event_once_fn(
	event_once_data_t* data,
	void* event_data
	)
{
	event_once_payload_t once = data->once;
	event_target_del_once(once.target, &data->listener);
	once.data.fn(once.data.data, event_data);
}


event_listener_t*
event_target_add_common(
	event_target_t* target,
	event_listener_data_t data,
	bool once
	)
{
	assert_not_null(target);
	assert_not_null(data.fn);

	event_listener_t* listener;

	if(once)
	{
		event_once_data_t* once_data = alloc_malloc(once_data, 1);
		assert_ptr(once_data, 1);

		once_data->once =
		(event_once_payload_t)
		{
			.target = target,
			.data = data
		};

		data =
		(event_listener_data_t)
		{
			.fn = (void*) event_once_fn,
			.data = once_data
		};

		listener = &once_data->listener;
	}
	else
	{
		listener = alloc_malloc(listener, 1);
		assert_ptr(listener, 1);
	}

	listener->prev = NULL;
	listener->next = target->head;
	listener->data = data;

	if(target->head)
	{
		target->head->prev = listener;
	}

	target->head = listener;

	return listener;
}


event_listener_t*
event_target_add(
	event_target_t* target,
	event_listener_data_t data
	)
{
	return event_target_add_common(target, data, false);
}


event_listener_t*
event_target_once(
	event_target_t* target,
	event_listener_data_t data
	)
{
	return event_target_add_common(target, data, true);
}


void
event_target_del_common(
	event_target_t* target,
	event_listener_t* listener,
	alloc_t additional_size
	)
{
	assert_not_null(target);

	if(!listener)
	{
		return;
	}

#ifndef NDEBUG
	event_listener_t* current = target->head;
	bool found = false;

	while(current)
	{
		if(current == listener)
		{
			found = true;
			break;
		}

		current = current->next;
	}

	assert_true(found);
#endif

	if(listener->prev)
	{
		listener->prev->next = listener->next;
	}
	else
	{
		target->head = listener->next;
	}

	if(listener->next)
	{
		listener->next->prev = listener->prev;
	}

	alloc_free((void*) listener, sizeof(*listener) + additional_size);
}


void
event_target_del(
	event_target_t* target,
	event_listener_t* listener
	)
{
	event_target_del_common(target, listener, 0);
}


void
event_target_del_once(
	event_target_t* target,
	event_listener_t* listener
	)
{
	event_target_del_common(target, listener, sizeof(event_once_payload_t));
}


void
event_target_fire(
	event_target_t* target,
	void* event_data
	)
{
	assert_not_null(target);

	event_listener_t* listener = target->head;
	while(listener)
	{
		event_listener_t* next = listener->next;
		listener->data.fn(listener->data.data, event_data);
		listener = next;
	}
}


typedef struct event_wait_data
{
	sync_sem_t sem;
	void* event_data;
}
event_wait_data_t;

struct event_wait_state
{
	event_target_t* target;
	event_listener_t* listener;
	event_wait_data_t wait_data;
};


void
event_target_wait_fn(
	event_wait_data_t* data,
	void* event_data
	)
{
	data->event_data = event_data;
	sync_sem_post(&data->sem);
}


event_wait_state_t*
event_target_init_wait(
	event_target_t* target
	)
{
	event_wait_state_t* state = alloc_malloc(state, 1);
	assert_ptr(state, 1);

	state->target = target;
	sync_sem_init(&state->wait_data.sem, 0);

	event_listener_data_t data =
	{
		.fn = (void*) event_target_wait_fn,
		.data = &state->wait_data
	};
	state->listener = event_target_add(target, data);

	return state;
}


void*
event_target_wait(
	event_wait_state_t* state
	)
{
	assert_not_null(state);

	sync_sem_wait(&state->wait_data.sem);

	event_target_del(state->target, state->listener);
	sync_sem_free(&state->wait_data.sem);

	void* event_data = state->wait_data.event_data;
	alloc_free(state, 1);

	return event_data;
}
