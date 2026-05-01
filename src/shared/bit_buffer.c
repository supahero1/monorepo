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

#include <shared/str.h>
#include <shared/debug.h>
#include <shared/macro.h>
#include <shared/alloc/base.h>
#include <shared/bit_buffer.h>

#include <math.h>
#include <stddef.h>
#include <stdint.h>


void
bit_buffer_set(
	bit_buffer_t* bit_buffer,
	uint8_t* data,
	uint64_t len
	)
{
	assert_not_null(bit_buffer);
	assert_ptr(data, len);

	bit_buffer->data = data;
	bit_buffer->at = data;

	bit_buffer->len = len;
	bit_buffer->bit = 0;
}


void
bit_buffer_reset(
	bit_buffer_t* bit_buffer
	)
{
	assert_not_null(bit_buffer);

	bit_buffer->at = bit_buffer->data;
	bit_buffer->bit = 0;
}


uint64_t
bit_buffer_available_bits(
	bit_buffer_t* bit_buffer
	)
{
	assert_not_null(bit_buffer);

	return (bit_buffer->len << 3) - bit_buffer_consumed_bits(bit_buffer);
}


uint64_t
bit_buffer_available_bytes(
	bit_buffer_t* bit_buffer
	)
{
	assert_not_null(bit_buffer);

	return MACRO_TO_BYTES(bit_buffer_available_bits(bit_buffer));
}


uint64_t
bit_buffer_consumed_bits(
	bit_buffer_t* bit_buffer
	)
{
	assert_not_null(bit_buffer);

	return ((bit_buffer->at - bit_buffer->data) << 3) + bit_buffer->bit;
}


uint64_t
bit_buffer_consumed_bytes(
	bit_buffer_t* bit_buffer
	)
{
	assert_not_null(bit_buffer);

	return MACRO_TO_BYTES(bit_buffer_consumed_bits(bit_buffer));
}


void
bit_buffer_skip_bits(
	bit_buffer_t* bit_buffer,
	uint64_t bits
	)
{
	assert_not_null(bit_buffer);

	bit_buffer->bit += bits;
	bit_buffer->at += bit_buffer->bit >> 3;
	bit_buffer->bit &= 7;
}


void
bit_buffer_skip_bits_safe(
	bit_buffer_t* bit_buffer,
	uint64_t bits,
	bool* status
	)
{
	assert_not_null(bit_buffer);
	assert_not_null(status);

	if(bit_buffer_available_bits(bit_buffer) < bits)
	{
		*status = false;
		return;
	}

	*status = true;
	bit_buffer_skip_bits(bit_buffer, bits);
}


void
bit_buffer_skip_bytes(
	bit_buffer_t* bit_buffer,
	uint64_t bytes
	)
{
	assert_not_null(bit_buffer);

	bit_buffer->at += bytes;
}


void
bit_buffer_skip_bytes_safe(
	bit_buffer_t* bit_buffer,
	uint64_t bytes,
	bool* status
	)
{
	assert_not_null(bit_buffer);
	assert_not_null(status);

	if(bit_buffer_available_bytes(bit_buffer) < bytes)
	{
		*status = false;
		return;
	}

	*status = true;
	bit_buffer_skip_bytes(bit_buffer, bytes);
}


bit_buffer_ctx_t
bit_buffer_save(
	bit_buffer_t* bit_buffer
	)
{
	assert_not_null(bit_buffer);

	return
	(bit_buffer_ctx_t)
	{
		.at = bit_buffer->at,
		.bit = bit_buffer->bit
	};
}


void
bit_buffer_restore(
	bit_buffer_t* bit_buffer,
	const bit_buffer_ctx_t* ctx
	)
{
	assert_not_null(bit_buffer);
	assert_not_null(ctx);

	bit_buffer->at = ctx->at;
	bit_buffer->bit = ctx->bit;
}


void
bit_buffer_set_bits(
	bit_buffer_t* bit_buffer,
	uint64_t num,
	uint64_t bits
	)
{
	assert_not_null(bit_buffer);
	assert_le(bits, 64);

	num &= UINT64_MAX >> (64 - bits);

	uint8_t* at = bit_buffer->at;

	while(bits)
	{
		int64_t delta = bits - 8 + bit_buffer->bit;

		if(delta < 0)
		{
			*at |= num << -delta;

			bit_buffer->bit += bits;

			break;
		}
		else
		{
			*(at++) |= num >> delta;

			bit_buffer->bit = 0;
			bits = delta;
		}
	}

	bit_buffer->at = at;
}


uint64_t
bit_buffer_get_bits(
	bit_buffer_t* bit_buffer,
	uint64_t bits
	)
{
	assert_not_null(bit_buffer);
	assert_le(bits, 64);

	uint64_t mask = UINT64_MAX >> (64 - bits);
	uint64_t num = 0;

	uint8_t* at = bit_buffer->at;

	while(bits)
	{
		int64_t delta = bits - 8 + bit_buffer->bit;

		if(delta < 0)
		{
			num |= *at >> -delta;

			bit_buffer->bit += bits;

			break;
		}
		else
		{
			num |= (uint64_t) *(at++) << delta;

			bit_buffer->bit = 0;
			bits = delta;
		}
	}

	bit_buffer->at = at;

	num &= mask;

	return num;
}


uint64_t
bit_buffer_get_bits_safe(
	bit_buffer_t* bit_buffer,
	uint64_t bits,
	bool* status
	)
{
	assert_not_null(bit_buffer);
	assert_le(bits, 64);
	assert_not_null(status);

	if(bit_buffer_available_bits(bit_buffer) < bits)
	{
		*status = false;
		return 0;
	}

	*status = true;
	return bit_buffer_get_bits(bit_buffer, bits);
}


uint64_t
bit_buffer_len_bits(
	uint64_t bits
	)
{
	assert_le(bits, 64);

	return bits;
}


void
bit_buffer_set_signed_bits(
	bit_buffer_t* bit_buffer,
	int64_t num,
	uint64_t bits
	)
{
	assert_not_null(bit_buffer);
	assert_le(bits, 63);

	if(!bits)
	{
		return;
	}

	uint64_t unum = num << 1;
	if(num < 0)
	{
		unum = ~unum;
	}

	bit_buffer_set_bits(bit_buffer, unum, bits + 1);
}


int64_t
bit_buffer_get_signed_bits(
	bit_buffer_t* bit_buffer,
	uint64_t bits
	)
{
	assert_not_null(bit_buffer);
	assert_le(bits, 63);

	if(!bits)
	{
		return 0;
	}

	uint64_t unum = bit_buffer_get_bits(bit_buffer, bits + 1);

	bool sign = unum & 1;
	unum >>= 1;
	if(sign)
	{
		unum = ~unum;
	}

	return unum;
}


int64_t
bit_buffer_get_signed_bits_safe(
	bit_buffer_t* bit_buffer,
	uint64_t bits,
	bool* status
	)
{
	assert_not_null(bit_buffer);
	assert_le(bits, 63);
	assert_not_null(status);

	if(!bits)
	{
		*status = true;
		return 0;
	}

	if(bit_buffer_available_bits(bit_buffer) < 1 + bits)
	{
		*status = false;
		return 0;
	}

	*status = true;
	return bit_buffer_get_signed_bits(bit_buffer, bits);
}


uint64_t
bit_buffer_len_signed_bits(
	uint64_t bits
	)
{
	assert_le(bits, 63);

	if(!bits)
	{
		return 0;
	}

	return bit_buffer_len_bits(bits + 1);
}


extern void
bit_buffer_set_bits_var(
	bit_buffer_t* bit_buffer,
	uint64_t num,
	uint64_t segment
	)
{
	assert_not_null(bit_buffer);
	assert_ge(segment, 1);
	assert_le(segment, 63);

	uint64_t segment_bit = (uint64_t) 1 << segment;
	uint64_t segment_mask = segment_bit - 1;

	while(num > segment_mask)
	{
		bit_buffer_set_bits(bit_buffer, (num & segment_mask) | segment_bit, segment + 1);

		num >>= segment;
	}

	bit_buffer_set_bits(bit_buffer, num, segment + 1);
}


extern uint64_t
bit_buffer_get_bits_var(
	bit_buffer_t* bit_buffer,
	uint64_t segment
	)
{
	assert_not_null(bit_buffer);
	assert_ge(segment, 1);
	assert_le(segment, 63);

	uint64_t num = 0;
	uint64_t shift = 0;

	uint64_t segment_bit = (uint64_t) 1 << segment;
	uint64_t segment_mask = segment_bit - 1;

	while(1)
	{
		uint64_t part = bit_buffer_get_bits(bit_buffer, segment + 1);

		num |= (part & segment_mask) << shift;

		if(!(part & segment_bit))
		{
			break;
		}

		shift += segment;
	}

	return num;
}


uint64_t
bit_buffer_get_bits_var_safe(
	bit_buffer_t* bit_buffer,
	uint64_t segment,
	bool* status
	)
{
	assert_not_null(bit_buffer);
	assert_ge(segment, 1);
	assert_le(segment, 63);
	assert_not_null(status);

	uint64_t num = 0;
	uint64_t shift = 0;

	uint64_t segment_bit = (uint64_t) 1 << segment;
	uint64_t segment_mask = segment_bit - 1;

	while(1)
	{
		uint64_t part = bit_buffer_get_bits_safe(bit_buffer, segment + 1, status);
		if(!*status)
		{
			return 0;
		}

		num |= (part & segment_mask) << shift;

		if(!(part & segment_bit))
		{
			break;
		}

		shift += segment;
		if(shift >= 64)
		{
			*status = false;
			return 0;
		}
	}

	return num;
}


uint64_t
bit_buffer_len_bits_var(
	uint64_t num,
	uint64_t segment
	)
{
	assert_ge(segment, 1);
	assert_le(segment, 63);

	uint64_t len = 0;

	uint64_t segment_bit = (uint64_t) 1 << segment;
	uint64_t segment_mask = segment_bit - 1;

	while(num > segment_mask)
	{
		len += segment + 1;
		num >>= segment;
	}

	len += segment + 1;

	return len;
}


void
bit_buffer_set_signed_bits_var(
	bit_buffer_t* bit_buffer,
	int64_t num,
	uint64_t segment
	)
{
	assert_not_null(bit_buffer);
	assert_ge(segment, 1);
	assert_le(segment, 63);

	uint64_t unum = num << 1;
	if(num < 0)
	{
		unum = ~unum;
	}

	bit_buffer_set_bits_var(bit_buffer, unum, segment);
}


int64_t
bit_buffer_get_signed_bits_var(
	bit_buffer_t* bit_buffer,
	uint64_t segment
	)
{
	assert_not_null(bit_buffer);
	assert_ge(segment, 1);
	assert_le(segment, 63);

	uint64_t unum = bit_buffer_get_bits_var(bit_buffer, segment);

	bool sign = unum & 1;
	unum >>= 1;
	if(sign)
	{
		unum = ~unum;
	}

	return unum;
}


int64_t
bit_buffer_get_signed_bits_var_safe(
	bit_buffer_t* bit_buffer,
	uint64_t segment,
	bool* status
	)
{
	assert_not_null(bit_buffer);
	assert_ge(segment, 1);
	assert_le(segment, 63);
	assert_not_null(status);

	uint64_t unum = bit_buffer_get_bits_var_safe(bit_buffer, segment, status);
	if(!*status)
	{
		return 0;
	}

	bool sign = unum & 1;
	unum >>= 1;
	if(sign)
	{
		unum = ~unum;
	}

	return unum;
}


uint64_t
bit_buffer_len_signed_bits_var(
	int64_t num,
	uint64_t segment
	)
{
	assert_ge(segment, 1);
	assert_le(segment, 63);

	uint64_t unum = num << 1;
	if(num < 0)
	{
		unum = ~unum;
	}

	return bit_buffer_len_bits_var(unum, segment);
}


void
bit_buffer_set_fixed_point(
	bit_buffer_t* bit_buffer,
	float value,
	uint64_t integer_bits,
	uint64_t fraction_bits
	)
{
	assert_not_null(bit_buffer);
	assert_le(fraction_bits, 23);
	assert_le(integer_bits + fraction_bits, 31);

	uint32_t mask = ((uint32_t) 1 << (integer_bits + fraction_bits)) - 1;
	uint32_t num = (uint32_t) roundf(value * MACRO_U32_TO_F32((fraction_bits + 127) << 23)) & mask;
	bit_buffer_set_bits(bit_buffer, num, integer_bits + fraction_bits);
}


float
bit_buffer_get_fixed_point(
	bit_buffer_t* bit_buffer,
	uint64_t integer_bits,
	uint64_t fraction_bits
	)
{
	assert_not_null(bit_buffer);
	assert_le(fraction_bits, 23);
	assert_le(integer_bits + fraction_bits, 31);

	uint32_t num = bit_buffer_get_bits(bit_buffer, integer_bits + fraction_bits);
	return num * MACRO_U32_TO_F32((-fraction_bits + 127) << 23);
}


float
bit_buffer_get_fixed_point_safe(
	bit_buffer_t* bit_buffer,
	uint64_t integer_bits,
	uint64_t fraction_bits,
	bool* status
	)
{
	assert_not_null(bit_buffer);
	assert_le(fraction_bits, 23);
	assert_le(integer_bits + fraction_bits, 31);
	assert_not_null(status);

	if(bit_buffer_available_bits(bit_buffer) < integer_bits + fraction_bits)
	{
		*status = false;
		return 0;
	}

	*status = true;
	return bit_buffer_get_fixed_point(bit_buffer, integer_bits, fraction_bits);
}


uint64_t
bit_buffer_len_fixed_point(
	uint64_t integer_bits,
	uint64_t fraction_bits
	)
{
	assert_le(fraction_bits, 23);
	assert_le(integer_bits + fraction_bits, 31);

	return integer_bits + fraction_bits;
}


void
bit_buffer_set_signed_fixed_point(
	bit_buffer_t* bit_buffer,
	float value,
	uint64_t integer_bits,
	uint64_t fraction_bits
	)
{
	assert_not_null(bit_buffer);
	assert_le(fraction_bits, 23);
	assert_le(integer_bits + fraction_bits, 31);

	if(!integer_bits && !fraction_bits)
	{
		return;
	}

	uint32_t num = MACRO_F32_TO_U32(value);
	bit_buffer_set_bits(bit_buffer, num >> 31, 1);
	value = MACRO_U32_TO_F32(num & 0x7FFFFFFF);
	bit_buffer_set_fixed_point(bit_buffer, value, integer_bits, fraction_bits);
}


float
bit_buffer_get_signed_fixed_point(
	bit_buffer_t* bit_buffer,
	uint64_t integer_bits,
	uint64_t fraction_bits
	)
{
	assert_not_null(bit_buffer);
	assert_le(fraction_bits, 23);
	assert_le(integer_bits + fraction_bits, 31);

	if(!integer_bits && !fraction_bits)
	{
		return 0.0f;
	}

	uint64_t sign = bit_buffer_get_bits(bit_buffer, 1);
	uint32_t value = MACRO_F32_TO_U32(bit_buffer_get_fixed_point(bit_buffer, integer_bits, fraction_bits));
	value |= sign << 31;
	return MACRO_U32_TO_F32(value);
}


float
bit_buffer_get_signed_fixed_point_safe(
	bit_buffer_t* bit_buffer,
	uint64_t integer_bits,
	uint64_t fraction_bits,
	bool* status
	)
{
	assert_not_null(bit_buffer);
	assert_le(fraction_bits, 23);
	assert_le(integer_bits + fraction_bits, 31);
	assert_not_null(status);

	if(!integer_bits && !fraction_bits)
	{
		*status = true;
		return 0.0f;
	}

	if(bit_buffer_available_bits(bit_buffer) < 1 + integer_bits + fraction_bits)
	{
		*status = false;
		return 0;
	}

	*status = true;
	return bit_buffer_get_signed_fixed_point(bit_buffer, integer_bits, fraction_bits);
}


uint64_t
bit_buffer_len_signed_fixed_point(
	uint64_t integer_bits,
	uint64_t fraction_bits
	)
{
	if(!integer_bits && !fraction_bits)
	{
		return 0;
	}

	return 1 + bit_buffer_len_fixed_point(integer_bits, fraction_bits);
}


void
bit_buffer_set_bytes(
	bit_buffer_t* bit_buffer,
	const void* data,
	uint64_t len
	)
{
	assert_not_null(bit_buffer);
	assert_ptr(data, len);

	uint64_t bits = MACRO_TO_BITS(len);

	while(bits >= 64)
	{
		bit_buffer_set_bits(bit_buffer, *(uint64_t*) data, 64);

		data += 8;
		bits -= 64;
	}

	while(bits)
	{
		uint64_t size = MACRO_MIN(bits, 8);
		bit_buffer_set_bits(bit_buffer, *(uint8_t*) data, size);

		data += 1;
		bits -= size;
	}
}


void
bit_buffer_get_bytes(
	bit_buffer_t* bit_buffer,
	void* data,
	uint64_t len
	)
{
	assert_not_null(bit_buffer);
	assert_ptr(data, len);

	uint64_t bits = MACRO_TO_BITS(len);

	while(bits >= 64)
	{
		*(uint64_t*) data = bit_buffer_get_bits(bit_buffer, 64);

		data += 8;
		bits -= 64;
	}

	while(bits)
	{
		uint64_t size = MACRO_MIN(bits, 8);
		*(uint8_t*) data = bit_buffer_get_bits(bit_buffer, size);

		data += 1;
		bits -= size;
	}
}


void
bit_buffer_get_bytes_safe(
	bit_buffer_t* bit_buffer,
	void* data,
	uint64_t len,
	bool* status
	)
{
	assert_not_null(bit_buffer);
	assert_ptr(data, len);
	assert_not_null(status);

	if(bit_buffer_available_bytes(bit_buffer) < len)
	{
		*status = false;
		return;
	}

	*status = true;
	bit_buffer_get_bytes(bit_buffer, data, len);
}


uint64_t
bit_buffer_len_bytes(
	uint64_t len
	)
{
	return MACRO_TO_BITS(len);
}


void
bit_buffer_set_str(
	bit_buffer_t* bit_buffer,
	str_t str
	)
{
	assert_not_null(bit_buffer);
	assert_not_null(str);

	bit_buffer_set_bits_var(bit_buffer, str->len, 6);
	bit_buffer_set_bytes(bit_buffer, str->str, str->len);
}


str_t
bit_buffer_get_str(
	bit_buffer_t* bit_buffer
	)
{
	assert_not_null(bit_buffer);

	uint64_t len = bit_buffer_get_bits_var(bit_buffer, 6);

	uint8_t* str = cstr_alloc(len);
	bit_buffer_get_bytes(bit_buffer, str, len);

	return str_init_move_len(str, len);
}


str_t
bit_buffer_get_str_safe(
	bit_buffer_t* bit_buffer,
	uint64_t max_len,
	bool* status
	)
{
	assert_not_null(bit_buffer);
	assert_not_null(status);

	uint64_t len = bit_buffer_get_bits_var_safe(bit_buffer, 6, status);
	if(len > max_len)
	{
		*status = false;
	}
	if(!*status)
	{
		return NULL;
	}

	uint8_t* str = cstr_alloc(len);
	bit_buffer_get_bytes_safe(bit_buffer, str, len, status);
	if(!*status)
	{
		cstr_free_len(str, len);
		return NULL;
	}

	return str_init_move_len(str, len);
}


uint64_t
bit_buffer_len_str(
	uint64_t len
	)
{
	return bit_buffer_len_bits_var(len, 6) + bit_buffer_len_bytes(len);
}
