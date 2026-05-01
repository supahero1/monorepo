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


typedef void
(*event_fn_t)(
	void* data,
	void* event_data
	);


typedef struct event_listener_data
{
	event_fn_t fn;
	void* data;
}
event_listener_data_t;


typedef struct event_listener event_listener_t;

struct event_listener
{
	event_listener_t* prev;
	event_listener_t* next;
	event_listener_data_t data;
};


typedef struct event_target
{
	event_listener_t* head;
}
event_target_t;


extern void
event_target_init(
	event_target_t* target
	);


extern void
event_target_free(
	event_target_t* target
	);


extern event_listener_t*
event_target_add(
	event_target_t* target,
	event_listener_data_t data
	);


extern event_listener_t*
event_target_once(
	event_target_t* target,
	event_listener_data_t data
	);


extern void
event_target_del(
	event_target_t* target,
	event_listener_t* listener
	);


extern void
event_target_del_once(
	event_target_t* target,
	event_listener_t* listener
	);


extern void
event_target_fire(
	event_target_t* target,
	void* event_data
	);


typedef struct event_wait_state event_wait_state_t;


extern event_wait_state_t*
event_target_init_wait(
	event_target_t* target
	);


extern void*
event_target_wait(
	event_wait_state_t* state
	);
