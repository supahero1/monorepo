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

#pragma once

#include <stdint.h>


extern void*
cstr_alloc(
	uint64_t len
	);


extern void*
cstr_resize(
	void* cstr,
	uint64_t new_len
	);


extern void*
cstr_resize_len(
	void* cstr,
	uint64_t old_len,
	uint64_t new_len
	);


extern void*
cstr_init(
	const void* cstr
	);


extern void*
cstr_init_len(
	const void* cstr,
	uint64_t len
	);


extern void
cstr_free(
	const void* cstr
	);


extern void
cstr_free_len(
	const void* cstr,
	uint64_t len
	);


extern bool
cstr_cmp(
	const void* cstr1,
	const void* cstr2
	);


extern bool
cstr_case_cmp(
	const void* cstr1,
	const void* cstr2
	);





typedef struct str
{
	void* str;
	uint64_t len;
}*
str_t;


extern str_t
str_init(
	void
	);


extern str_t
str_init_copy_cstr(
	const void* cstr
	);


extern str_t
str_init_move_cstr(
	void* cstr
	);


extern str_t
str_init_copy_len(
	const void* cstr,
	uint64_t len
	);


extern str_t
str_init_move_len(
	void* cstr,
	uint64_t len
	);


extern str_t
str_init_copy(
	const str_t other
	);


extern str_t
str_init_move(
	str_t other
	);


extern void
str_free(
	str_t str
	);


extern void
str_clear(
	str_t str
	);


extern void
str_reset(
	str_t str
	);


extern bool
str_is_empty(
	const str_t str
	);


extern void
str_set_copy_cstr(
	str_t str,
	const void* cstr
	);


extern void
str_set_move_cstr(
	str_t str,
	void* cstr
	);


extern void
str_set_copy_len(
	str_t str,
	const void* cstr,
	uint64_t len
	);


extern void
str_set_move_len(
	str_t str,
	void* cstr,
	uint64_t len
	);


extern void
str_set_copy(
	str_t str,
	const str_t other
	);


extern void
str_set_move(
	str_t str,
	str_t other
	);


extern void
str_resize(
	str_t str,
	uint64_t len
	);


extern bool
str_cmp(
	const str_t str1,
	const str_t str2
	);


extern bool
str_case_cmp(
	const str_t str1,
	const str_t str2
	);


extern bool
str_cmp_cstr(
	const str_t str,
	const void* cstr
	);


extern bool
str_case_cmp_cstr(
	const str_t str,
	const void* cstr
	);


extern bool
str_cmp_len(
	const str_t str,
	const void* cstr,
	uint64_t len
	);


extern bool
str_case_cmp_len(
	const str_t str,
	const void* cstr,
	uint64_t len
	);
