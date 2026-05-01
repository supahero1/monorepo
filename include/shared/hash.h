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

#include <shared/str.h>

#include <stdint.h>


typedef struct hash_table* hash_table_t;


typedef void
(*hash_table_key_free_fn_t)(
	str_t key
	);

typedef void
(*hash_table_value_free_fn_t)(
	void* value
	);

typedef void
(*hash_table_for_each_fn_t)(
	str_t key,
	void* value,
	void* data
	);


extern hash_table_t
hash_table_init(
	uint32_t bucket_count,
	hash_table_key_free_fn_t key_free_fn,
	hash_table_value_free_fn_t value_free_fn
	);


extern void
hash_table_free(
	hash_table_t table
	);


extern void
hash_table_clear(
	hash_table_t table
	);


extern void
hash_table_for_each(
	hash_table_t table,
	hash_table_for_each_fn_t fn,
	void* data
	);


/* false if not found */
extern bool
hash_table_has(
	hash_table_t table,
	const char* key
	);


/* false if already exists */
extern bool
hash_table_add(
	hash_table_t table,
	const char* key,
	void* value
	);


/* false if this is a new entry */
extern bool
hash_table_set(
	hash_table_t table,
	const char* key,
	void* value
	);


/* false if not found */
extern bool
hash_table_modify(
	hash_table_t table,
	const char* key,
	void* value
	);


extern void*
hash_table_get(
	hash_table_t table,
	const char* key
	);


/* false if not found */
extern bool
hash_table_del(
	hash_table_t table,
	const char* key
	);
