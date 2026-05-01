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


typedef struct bit_buffer
{
	uint8_t* data;
	uint8_t* at;

	uint64_t len;
	uint64_t bit;
}
bit_buffer_t;


typedef struct bit_buffer_ctx
{
	uint8_t* at;
	uint64_t bit;
}
bit_buffer_ctx_t;


extern void
bit_buffer_set(
	bit_buffer_t* bit_buffer,
	uint8_t* data,
	uint64_t len
	);


extern void
bit_buffer_reset(
	bit_buffer_t* bit_buffer
	);


extern uint64_t
bit_buffer_available_bits(
	bit_buffer_t* bit_buffer
	);


extern uint64_t
bit_buffer_available_bytes(
	bit_buffer_t* bit_buffer
	);


extern uint64_t
bit_buffer_consumed_bits(
	bit_buffer_t* bit_buffer
	);


extern uint64_t
bit_buffer_consumed_bytes(
	bit_buffer_t* bit_buffer
	);


extern void
bit_buffer_skip_bits(
	bit_buffer_t* bit_buffer,
	uint64_t bits
	);


extern void
bit_buffer_skip_bits_safe(
	bit_buffer_t* bit_buffer,
	uint64_t bits,
	bool* status
	);


extern void
bit_buffer_skip_bytes(
	bit_buffer_t* bit_buffer,
	uint64_t bytes
	);


extern void
bit_buffer_skip_bytes_safe(
	bit_buffer_t* bit_buffer,
	uint64_t bytes,
	bool* status
	);


extern bit_buffer_ctx_t
bit_buffer_save(
	bit_buffer_t* bit_buffer
	);


extern void
bit_buffer_restore(
	bit_buffer_t* bit_buffer,
	const bit_buffer_ctx_t* ctx
	);


extern void
bit_buffer_set_bits(
	bit_buffer_t* bit_buffer,
	uint64_t num,
	uint64_t bits
	);


extern uint64_t
bit_buffer_get_bits(
	bit_buffer_t* bit_buffer,
	uint64_t bits
	);


uint64_t
bit_buffer_get_bits_safe(
	bit_buffer_t* bit_buffer,
	uint64_t bits,
	bool* status
	);


extern uint64_t
bit_buffer_len_bits(
	uint64_t bits
	);


extern void
bit_buffer_set_signed_bits(
	bit_buffer_t* bit_buffer,
	int64_t num,
	uint64_t bits
	);


extern int64_t
bit_buffer_get_signed_bits(
	bit_buffer_t* bit_buffer,
	uint64_t bits
	);


extern int64_t
bit_buffer_get_signed_bits_safe(
	bit_buffer_t* bit_buffer,
	uint64_t bits,
	bool* status
	);


extern uint64_t
bit_buffer_len_signed_bits(
	uint64_t bits
	);


extern void
bit_buffer_set_bits_var(
	bit_buffer_t* bit_buffer,
	uint64_t num,
	uint64_t segment
	);


extern uint64_t
bit_buffer_get_bits_var(
	bit_buffer_t* bit_buffer,
	uint64_t segment
	);


extern uint64_t
bit_buffer_get_bits_var_safe(
	bit_buffer_t* bit_buffer,
	uint64_t segment,
	bool* status
	);


extern uint64_t
bit_buffer_len_bits_var(
	uint64_t num,
	uint64_t segment
	);


extern void
bit_buffer_set_signed_bits_var(
	bit_buffer_t* bit_buffer,
	int64_t num,
	uint64_t segment
	);


extern int64_t
bit_buffer_get_signed_bits_var(
	bit_buffer_t* bit_buffer,
	uint64_t segment
	);


extern int64_t
bit_buffer_get_signed_bits_var_safe(
	bit_buffer_t* bit_buffer,
	uint64_t segment,
	bool* status
	);


extern uint64_t
bit_buffer_len_signed_bits_var(
	int64_t num,
	uint64_t segment
	);


extern void
bit_buffer_set_fixed_point(
	bit_buffer_t* bit_buffer,
	float value,
	uint64_t integer_bits,
	uint64_t fraction_bits
	);


extern float
bit_buffer_get_fixed_point(
	bit_buffer_t* bit_buffer,
	uint64_t integer_bits,
	uint64_t fraction_bits
	);


extern float
bit_buffer_get_fixed_point_safe(
	bit_buffer_t* bit_buffer,
	uint64_t integer_bits,
	uint64_t fraction_bits,
	bool* status
	);


extern uint64_t
bit_buffer_len_fixed_point(
	uint64_t integer_bits,
	uint64_t fraction_bits
	);


extern void
bit_buffer_set_signed_fixed_point(
	bit_buffer_t* bit_buffer,
	float value,
	uint64_t integer_bits,
	uint64_t fraction_bits
	);


extern float
bit_buffer_get_signed_fixed_point(
	bit_buffer_t* bit_buffer,
	uint64_t integer_bits,
	uint64_t fraction_bits
	);


extern float
bit_buffer_get_signed_fixed_point_safe(
	bit_buffer_t* bit_buffer,
	uint64_t integer_bits,
	uint64_t fraction_bits,
	bool* status
	);


extern uint64_t
bit_buffer_len_signed_fixed_point(
	uint64_t integer_bits,
	uint64_t fraction_bits
	);


extern void
bit_buffer_set_bytes(
	bit_buffer_t* bit_buffer,
	const void* data,
	uint64_t len
	);


extern void
bit_buffer_get_bytes(
	bit_buffer_t* bit_buffer,
	void* data,
	uint64_t len
	);


extern void
bit_buffer_get_bytes_safe(
	bit_buffer_t* bit_buffer,
	void* data,
	uint64_t len,
	bool* status
	);


extern uint64_t
bit_buffer_len_bytes(
	uint64_t len
	);


extern void
bit_buffer_set_str(
	bit_buffer_t* bit_buffer,
	str_t str
	);


extern str_t
bit_buffer_get_str(
	bit_buffer_t* bit_buffer
	);


extern str_t
bit_buffer_get_str_safe(
	bit_buffer_t* bit_buffer,
	uint64_t max_len,
	bool* status
	);


extern uint64_t
bit_buffer_len_str(
	uint64_t len
	);
