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

#include <shared/str.h>
#include <shared/attr.h>
#include <shared/debug.h>
#include <shared/macro.h>
#include <shared/bit_buffer.h>

#include <stdint.h>
#include <string.h>


void attr_test_fn
test_normal_pass__bit_buffer_empty_functions(
	void
	)
{
	bool status;

	bit_buffer_t buffer;
	bit_buffer_set(&buffer, NULL, 0);
	bit_buffer_reset(&buffer);

	assert_false(bit_buffer_available_bits(&buffer));
	assert_false(bit_buffer_available_bytes(&buffer));
	assert_false(bit_buffer_consumed_bits(&buffer));
	assert_false(bit_buffer_consumed_bytes(&buffer));

	bit_buffer_skip_bits(&buffer, 0);
	bit_buffer_skip_bits_safe(&buffer, 0, &status);
	assert_true(status);

	bit_buffer_skip_bits_safe(&buffer, 1, &status);
	assert_false(status);

	bit_buffer_skip_bytes(&buffer, 0);
	bit_buffer_skip_bytes_safe(&buffer, 0, &status);
	assert_true(status);

	bit_buffer_skip_bytes_safe(&buffer, 1, &status);
	assert_false(status);

	assert_false(bit_buffer_available_bits(&buffer));
	assert_false(bit_buffer_available_bytes(&buffer));
	assert_false(bit_buffer_consumed_bits(&buffer));
	assert_false(bit_buffer_consumed_bytes(&buffer));

	bit_buffer_ctx_t ctx = bit_buffer_save(&buffer);
	bit_buffer_restore(&buffer, &ctx);

	assert_false(bit_buffer_available_bits(&buffer));
	assert_false(bit_buffer_available_bytes(&buffer));
	assert_false(bit_buffer_consumed_bits(&buffer));
	assert_false(bit_buffer_consumed_bytes(&buffer));

	bit_buffer_set_bits(&buffer, 0, 0);
	assert_false(bit_buffer_get_bits(&buffer, 0));
	assert_false(bit_buffer_get_bits_safe(&buffer, 0, &status));
	assert_true(status);

	bit_buffer_set_fixed_point(&buffer, 0, 0, 0);
	assert_false(bit_buffer_get_fixed_point(&buffer, 0, 0));
	assert_false(bit_buffer_get_fixed_point_safe(&buffer, 0, 0, &status));
	assert_true(status);

	bit_buffer_set_bytes(&buffer, NULL, 0);
	bit_buffer_get_bytes(&buffer, NULL, 0);
	bit_buffer_get_bytes_safe(&buffer, NULL, 0, &status);
	assert_true(status);
}


void attr_test_fn
test_normal_pass__bit_buffer_set_get_bits(
	void
	)
{
	uint8_t data[16] = {0};
	bit_buffer_t buffer;
	bit_buffer_set(&buffer, data, sizeof(data));

	bit_buffer_set_bits(&buffer, 0, 0);
	bit_buffer_set_bits(&buffer, 0, 1);
	bit_buffer_set_bits(&buffer, 1, 1);
	bit_buffer_set_bits(&buffer, 0xFF, 2);
	bit_buffer_set_bits(&buffer, 0xFEDCBA9876543210, 64);

	assert_eq(bit_buffer_len_bits(0), 0);
	assert_eq(bit_buffer_len_bits(64), 64);

	assert_eq(bit_buffer_consumed_bytes(&buffer), 9);
	assert_eq(bit_buffer_consumed_bits(&buffer), 68);
	assert_eq(bit_buffer_available_bytes(&buffer), 8);
	assert_eq(bit_buffer_available_bits(&buffer), 60);

	bit_buffer_reset(&buffer);

	assert_eq(bit_buffer_consumed_bytes(&buffer), 0);
	assert_eq(bit_buffer_consumed_bits(&buffer), 0);
	assert_eq(bit_buffer_available_bytes(&buffer), 16);
	assert_eq(bit_buffer_available_bits(&buffer), 128);

	assert_eq(bit_buffer_get_bits(&buffer, 0), 0);
	assert_eq(bit_buffer_get_bits(&buffer, 1), 0);
	assert_eq(bit_buffer_get_bits(&buffer, 1), 1);
	assert_eq(bit_buffer_get_bits(&buffer, 2), 0x03);
	assert_eq(bit_buffer_get_bits(&buffer, 64), 0xFEDCBA9876543210);

	assert_eq(bit_buffer_consumed_bytes(&buffer), 9);
	assert_eq(bit_buffer_consumed_bits(&buffer), 68);
	assert_eq(bit_buffer_available_bytes(&buffer), 8);
	assert_eq(bit_buffer_available_bits(&buffer), 60);

	uint8_t data2[64] = {0};
	bit_buffer_set(&buffer, data2, sizeof(data2));

	bit_buffer_set_signed_bits(&buffer, 0, 0);
	bit_buffer_set_signed_bits(&buffer, 0, 1);
	bit_buffer_set_signed_bits(&buffer, -1, 1);
	bit_buffer_set_signed_bits(&buffer, -1, 2);
	bit_buffer_set_signed_bits(&buffer, 0x7EDCBA9876543210, 63);
	bit_buffer_set_signed_bits(&buffer, -0x7EDCBA9876543210, 63);

	assert_eq(bit_buffer_len_signed_bits(0), 0);
	assert_eq(bit_buffer_len_signed_bits(63), 64);

	assert_eq(bit_buffer_consumed_bits(&buffer), 135);
	bit_buffer_reset(&buffer);

	assert_eq(bit_buffer_get_signed_bits(&buffer, 0), 0);
	assert_eq(bit_buffer_get_signed_bits(&buffer, 1), 0);
	assert_eq(bit_buffer_get_signed_bits(&buffer, 1), -1);
	assert_eq(bit_buffer_get_signed_bits(&buffer, 2), -1);
	assert_eq(bit_buffer_get_signed_bits(&buffer, 63), 0x7EDCBA9876543210);
	assert_eq(bit_buffer_get_signed_bits(&buffer, 63), -0x7EDCBA9876543210);

	assert_eq(bit_buffer_consumed_bits(&buffer), 135);
	memset(data2, 0, sizeof(data2));
	bit_buffer_reset(&buffer);

	bit_buffer_set_bits_var(&buffer, 0, 1);
	bit_buffer_set_bits_var(&buffer, 1, 1);
	bit_buffer_set_bits_var(&buffer, 5, 1);
	bit_buffer_set_bits_var(&buffer, 5, 2);
	bit_buffer_set_bits_var(&buffer, 5, 3);
	bit_buffer_set_bits_var(&buffer, 0xFEDCBA9876543210, 63);

	assert_eq(bit_buffer_len_bits_var(0, 1), 2);
	assert_eq(bit_buffer_len_bits_var(0, 2), 3);
	assert_eq(bit_buffer_len_bits_var(1, 1), 2);
	assert_eq(bit_buffer_len_bits_var(5, 1), 6);
	assert_eq(bit_buffer_len_bits_var(5, 2), 6);
	assert_eq(bit_buffer_len_bits_var(5, 3), 4);
	assert_eq(bit_buffer_len_bits_var(0xFEDCBA9876543210, 63), 128);
	assert_eq(bit_buffer_len_bits_var(0xFEDCBA9876543210, 31), 96);

	assert_eq(bit_buffer_consumed_bits(&buffer), 148);
	bit_buffer_reset(&buffer);

	assert_eq(bit_buffer_get_bits_var(&buffer, 1), 0);
	assert_eq(bit_buffer_get_bits_var(&buffer, 1), 1);
	assert_eq(bit_buffer_get_bits_var(&buffer, 1), 5);
	assert_eq(bit_buffer_get_bits_var(&buffer, 2), 5);
	assert_eq(bit_buffer_get_bits_var(&buffer, 3), 5);
	assert_eq(bit_buffer_get_bits_var(&buffer, 63), 0xFEDCBA9876543210);

	assert_eq(bit_buffer_consumed_bits(&buffer), 148);
	memset(data2, 0, sizeof(data2));
	bit_buffer_reset(&buffer);

	bit_buffer_set_signed_bits_var(&buffer, 0, 1);
	bit_buffer_set_signed_bits_var(&buffer, 1, 1);
	bit_buffer_set_signed_bits_var(&buffer, -1, 1);
	bit_buffer_set_signed_bits_var(&buffer, -1, 2);
	bit_buffer_set_signed_bits_var(&buffer, 0x7EDCBA9876543210, 32);
	bit_buffer_set_signed_bits_var(&buffer, -0x7EDCBA9876543210, 32);

	assert_eq(bit_buffer_len_signed_bits_var(0, 1), 2);
	assert_eq(bit_buffer_len_signed_bits_var(2, 1), 6);
	assert_eq(bit_buffer_len_signed_bits_var(-2, 1), 4);
	assert_eq(bit_buffer_len_signed_bits_var(0x7EDCBA9876543210, 32), 66);
	assert_eq(bit_buffer_len_signed_bits_var(-0x7EDCBA9876543210, 32), 66);

	assert_eq(bit_buffer_consumed_bits(&buffer), 143);
	bit_buffer_reset(&buffer);

	assert_eq(bit_buffer_get_signed_bits_var(&buffer, 1), 0);
	assert_eq(bit_buffer_get_signed_bits_var(&buffer, 1), 1);
	assert_eq(bit_buffer_get_signed_bits_var(&buffer, 1), -1);
	assert_eq(bit_buffer_get_signed_bits_var(&buffer, 2), -1);
	assert_eq(bit_buffer_get_signed_bits_var(&buffer, 32), 0x7EDCBA9876543210);
	assert_eq(bit_buffer_get_signed_bits_var(&buffer, 32), -0x7EDCBA9876543210);

	assert_eq(bit_buffer_consumed_bits(&buffer), 143);
}


void attr_test_fn
test_normal_pass__bit_buffer_set_get_float(
	void
	)
{
	uint8_t data[32] = {0};
	bit_buffer_t buffer;
	bit_buffer_set(&buffer, data, sizeof(data));

	bit_buffer_set_fixed_point(&buffer, 0.0f, 0, 0);
	bit_buffer_set_fixed_point(&buffer, 0.0f, 0, 1);
	bit_buffer_set_fixed_point(&buffer, 0.0f, 1, 0);
	bit_buffer_set_fixed_point(&buffer, 0.0f, 1, 1);
	bit_buffer_set_fixed_point(&buffer, 2.5f, 0, 1);
	bit_buffer_set_fixed_point(&buffer, 2.5f, 1, 0);
	bit_buffer_set_fixed_point(&buffer, 2.5f, 1, 1);
	bit_buffer_set_fixed_point(&buffer, 2.5f, 2, 2);
	bit_buffer_set_fixed_point(&buffer, 127.181718171817181718f, 7, 23);
	bit_buffer_set_fixed_point(&buffer, 0.181718171817181718f, 0, 23);

	assert_eq(bit_buffer_len_fixed_point(0, 0), 0);
	assert_eq(bit_buffer_len_fixed_point(0, 1), 1);
	assert_eq(bit_buffer_len_fixed_point(1, 0), 1);
	assert_eq(bit_buffer_len_fixed_point(1, 1), 2);
	assert_eq(bit_buffer_len_fixed_point(10, 10), 20);

	assert_eq(bit_buffer_consumed_bits(&buffer), 65);
	bit_buffer_reset(&buffer);

	assert_eq(bit_buffer_get_fixed_point(&buffer, 0, 0), 0.0f);
	assert_eq(bit_buffer_get_fixed_point(&buffer, 0, 1), 0.0f);
	assert_eq(bit_buffer_get_fixed_point(&buffer, 1, 0), 0.0f);
	assert_eq(bit_buffer_get_fixed_point(&buffer, 1, 1), 0.0f);
	assert_eq(bit_buffer_get_fixed_point(&buffer, 0, 1), 0.5f);
	assert_eq(bit_buffer_get_fixed_point(&buffer, 1, 0), 1.0f);
	assert_eq(bit_buffer_get_fixed_point(&buffer, 1, 1), 0.5f);
	assert_eq(bit_buffer_get_fixed_point(&buffer, 2, 2), 2.5f);
	assert_eq(bit_buffer_get_fixed_point(&buffer, 7, 23), 127.181718171817181718f);
	assert_eq(bit_buffer_get_fixed_point(&buffer, 0, 23), 0.18171823f);

	assert_eq(bit_buffer_consumed_bits(&buffer), 65);
	memset(data, 0, sizeof(data));
	bit_buffer_reset(&buffer);

	bit_buffer_set_signed_fixed_point(&buffer, 0.0f, 0, 0);
	bit_buffer_set_signed_fixed_point(&buffer, -0.0f, 0, 0);
	bit_buffer_set_signed_fixed_point(&buffer, 0.0f, 1, 1);
	bit_buffer_set_signed_fixed_point(&buffer, -0.0f, 1, 1);
	bit_buffer_set_signed_fixed_point(&buffer, -2.5f, 0, 1);
	bit_buffer_set_signed_fixed_point(&buffer, -2.5f, 1, 0);
	bit_buffer_set_signed_fixed_point(&buffer, -2.5f, 1, 1);
	bit_buffer_set_signed_fixed_point(&buffer, -2.5f, 2, 2);
	bit_buffer_set_signed_fixed_point(&buffer, -127.181718171817181718f, 7, 23);
	bit_buffer_set_signed_fixed_point(&buffer, -0.181718171817181718f, 0, 23);

	assert_eq(bit_buffer_len_signed_fixed_point(0, 0), 0);
	assert_eq(bit_buffer_len_signed_fixed_point(0, 1), 2);
	assert_eq(bit_buffer_len_signed_fixed_point(1, 1), 3);
	assert_eq(bit_buffer_len_signed_fixed_point(10, 10), 21);

	assert_eq(bit_buffer_consumed_bits(&buffer), 73);
	bit_buffer_reset(&buffer);

	assert_eq(bit_buffer_get_signed_fixed_point(&buffer, 0, 0), 0.0f);
	assert_eq(bit_buffer_get_signed_fixed_point(&buffer, 0, 0), 0.0f);
	assert_eq(bit_buffer_get_signed_fixed_point(&buffer, 1, 1), 0.0f);
	assert_eq(bit_buffer_get_signed_fixed_point(&buffer, 1, 1), 0.0f);
	assert_eq(bit_buffer_get_signed_fixed_point(&buffer, 0, 1), -0.5f);
	assert_eq(bit_buffer_get_signed_fixed_point(&buffer, 1, 0), -1.0f);
	assert_eq(bit_buffer_get_signed_fixed_point(&buffer, 1, 1), -0.5f);
	assert_eq(bit_buffer_get_signed_fixed_point(&buffer, 2, 2), -2.5f);
	assert_eq(bit_buffer_get_signed_fixed_point(&buffer, 7, 23), -127.181718171817181718f);
	assert_eq(bit_buffer_get_signed_fixed_point(&buffer, 0, 23), -0.18171823f);

	assert_eq(bit_buffer_consumed_bits(&buffer), 73);
}


void attr_test_fn
test_normal_pass__bit_buffer_set_get_bytes(
	void
	)
{
	uint8_t data[32];
	bit_buffer_t buffer;
	bit_buffer_set(&buffer, data, sizeof(data));

	uint8_t data2[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	for(uint32_t i = 0; i < 8; ++i)
	{
		memset(data, 0, sizeof(data));
		bit_buffer_reset(&buffer);
		bit_buffer_set_bits(&buffer, 0, i);
		bit_buffer_set_bytes(&buffer, data2, sizeof(data2));

		assert_eq(bit_buffer_len_bytes(0), 0);
		assert_eq(bit_buffer_len_bytes(8), 64);

		assert_eq(bit_buffer_consumed_bits(&buffer), i + 64);
		bit_buffer_reset(&buffer);

		assert_eq(bit_buffer_get_bits(&buffer, i), 0);

		uint8_t data3[8];
		bit_buffer_get_bytes(&buffer, data3, sizeof(data2));

		assert_false(memcmp(data2, data3, sizeof(data2)));

		assert_eq(bit_buffer_consumed_bits(&buffer), i + 64);
	}

	const char* str_data = "Hello, World!";
	for(uint32_t i = 0; i < 8; ++i)
	{
		memset(data, 0, sizeof(data));
		bit_buffer_reset(&buffer);
		bit_buffer_set_bits(&buffer, 0, i);

		str_t str = str_init_copy_cstr(str_data);
		bit_buffer_set_str(&buffer, str);

		assert_eq(bit_buffer_len_bytes(0), 0);
		assert_eq(bit_buffer_len_str(str->len), 7 + str->len * 8);

		assert_eq(bit_buffer_consumed_bits(&buffer), i + 7 + str->len * 8);
		bit_buffer_reset(&buffer);

		assert_eq(bit_buffer_get_bits(&buffer, i), 0);

		str_t str_out = bit_buffer_get_str(&buffer);
		assert_not_null(str_out->str);

		assert_true(str_cmp(str, str_out));
		str_free(str_out);

		assert_eq(bit_buffer_consumed_bits(&buffer), i + 7 + str->len * 8);
		str_free(str);
	}
}


void attr_test_fn
test_normal_fail__bit_buffer_set_null_buffer(
	void
	)
{
	bit_buffer_set(NULL, NULL, 0);
}


void attr_test_fn
test_normal_fail__bit_buffer_set_null_ptr(
	void
	)
{
	bit_buffer_t bit_buffer;
	bit_buffer_set(&bit_buffer, NULL, 4);
}


void attr_test_fn
test_normal_fail__bit_buffer_reset_null(
	void
	)
{
	bit_buffer_reset(NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_available_bits_null(
	void
	)
{
	bit_buffer_available_bits(NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_available_bytes_null(
	void
	)
{
	bit_buffer_available_bytes(NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_consumed_bits_null(
	void
	)
{
	bit_buffer_consumed_bits(NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_consumed_bytes_null(
	void
	)
{
	bit_buffer_consumed_bytes(NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_skip_bits_null(
	void
	)
{
	bit_buffer_skip_bits(NULL, 0);
}


void attr_test_fn
test_normal_fail__bit_buffer_skip_bits_safe_null_buffer(
	void
	)
{
	bool status;
	bit_buffer_skip_bits_safe(NULL, 0, &status);
}


void attr_test_fn
test_normal_fail__bit_buffer_skip_bits_safe_null_status(
	void
	)
{
	bit_buffer_t bit_buffer;
	bit_buffer_skip_bits_safe(&bit_buffer, 0, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_skip_bits_safe_null(
	void
	)
{
	bit_buffer_skip_bits_safe(NULL, 0, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_skip_bytes_null(
	void
	)
{
	bit_buffer_skip_bytes(NULL, 0);
}


void attr_test_fn
test_normal_fail__bit_buffer_skip_bytes_safe_null_buffer(
	void
	)
{
	bool status;
	bit_buffer_skip_bytes_safe(NULL, 0, &status);
}


void attr_test_fn
test_normal_fail__bit_buffer_skip_bytes_safe_null_status(
	void
	)
{
	bit_buffer_t bit_buffer;
	bit_buffer_skip_bytes_safe(&bit_buffer, 0, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_skip_bytes_safe_null(
	void
	)
{
	bit_buffer_skip_bytes_safe(NULL, 0, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_save_null(
	void
	)
{
	bit_buffer_save(NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_restore_null_buffer(
	void
	)
{
	bit_buffer_ctx_t ctx = {0};
	bit_buffer_restore(NULL, &ctx);
}


void attr_test_fn
test_normal_fail__bit_buffer_restore_null_ctx(
	void
	)
{
	bit_buffer_t bit_buffer;
	bit_buffer_restore(&bit_buffer, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_restore_null(
	void
	)
{
	bit_buffer_restore(NULL, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_set_bits_null(
	void
	)
{
	bit_buffer_set_bits(NULL, 0, 0);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_bits_null(
	void
	)
{
	bit_buffer_get_bits(NULL, 0);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_bits_safe_null_buffer(
	void
	)
{
	bool status;
	bit_buffer_get_bits_safe(NULL, 0, &status);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_bits_safe_null_status(
	void
	)
{
	bit_buffer_t bit_buffer;
	bit_buffer_get_bits_safe(&bit_buffer, 0, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_bits_safe_null(
	void
	)
{
	bit_buffer_get_bits_safe(NULL, 0, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_set_signed_bits_null(
	void
	)
{
	bit_buffer_set_signed_bits(NULL, 0, 0);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_signed_bits_null(
	void
	)
{
	bit_buffer_get_signed_bits(NULL, 0);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_signed_bits_safe_null_buffer(
	void
	)
{
	bool status;
	bit_buffer_get_signed_bits_safe(NULL, 0, &status);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_signed_bits_safe_null_status(
	void
	)
{
	bit_buffer_t bit_buffer;
	bit_buffer_get_signed_bits_safe(&bit_buffer, 0, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_signed_bits_safe_null(
	void
	)
{
	bit_buffer_get_signed_bits_safe(NULL, 0, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_set_bits_var_null(
	void
	)
{
	bit_buffer_set_bits_var(NULL, 0, 0);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_bits_var_null(
	void
	)
{
	bit_buffer_get_bits_var(NULL, 0);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_bits_var_safe_null_buffer(
	void
	)
{
	bool status;
	bit_buffer_get_bits_var_safe(NULL, 0, &status);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_bits_var_safe_null_status(
	void
	)
{
	bit_buffer_t bit_buffer;
	bit_buffer_get_bits_var_safe(&bit_buffer, 0, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_bits_var_safe_null(
	void
	)
{
	bit_buffer_get_bits_var_safe(NULL, 0, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_set_signed_bits_var_null(
	void
	)
{
	bit_buffer_set_signed_bits_var(NULL, 0, 0);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_signed_bits_var_null(
	void
	)
{
	bit_buffer_get_signed_bits_var(NULL, 0);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_signed_bits_var_safe_null_buffer(
	void
	)
{
	bool status;
	bit_buffer_get_signed_bits_var_safe(NULL, 0, &status);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_signed_bits_var_safe_null_status(
	void
	)
{
	bit_buffer_t bit_buffer;
	bit_buffer_get_signed_bits_var_safe(&bit_buffer, 0, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_signed_bits_var_safe_null(
	void
	)
{
	bit_buffer_get_signed_bits_var_safe(NULL, 0, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_set_fixed_point_null(
	void
	)
{
	bit_buffer_set_fixed_point(NULL, 0, 0, 0);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_fixed_point_null(
	void
	)
{
	bit_buffer_get_fixed_point(NULL, 0, 0);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_fixed_point_safe_null_buffer(
	void
	)
{
	bool status;
	bit_buffer_get_fixed_point_safe(NULL, 0, 0, &status);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_fixed_point_safe_null_status(
	void
	)
{
	bit_buffer_t bit_buffer;
	bit_buffer_get_fixed_point_safe(&bit_buffer, 0, 0, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_fixed_point_safe_null(
	void
	)
{
	bit_buffer_get_fixed_point_safe(NULL, 0, 0, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_set_signed_fixed_point_null(
	void
	)
{
	bit_buffer_set_signed_fixed_point(NULL, 0, 0, 0);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_signed_fixed_point_null(
	void
	)
{
	bit_buffer_get_signed_fixed_point(NULL, 0, 0);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_signed_fixed_point_safe_null_buffer(
	void
	)
{
	bool status;
	bit_buffer_get_signed_fixed_point_safe(NULL, 0, 0, &status);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_signed_fixed_point_safe_null_status(
	void
	)
{
	bit_buffer_t bit_buffer;
	bit_buffer_get_signed_fixed_point_safe(&bit_buffer, 0, 0, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_signed_fixed_point_safe_null(
	void
	)
{
	bit_buffer_get_signed_fixed_point_safe(NULL, 0, 0, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_set_bytes_null_buffer(
	void
	)
{
	bit_buffer_set_bytes(NULL, NULL, 0);
}


void attr_test_fn
test_normal_fail__bit_buffer_set_bytes_null_data(
	void
	)
{
	bit_buffer_t bit_buffer;
	bit_buffer_set_bytes(&bit_buffer, NULL, 4);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_bytes_null_buffer(
	void
	)
{
	bit_buffer_get_bytes(NULL, NULL, 0);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_bytes_null_data(
	void
	)
{
	bit_buffer_t bit_buffer;
	bit_buffer_get_bytes(&bit_buffer, NULL, 4);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_bytes_safe_null_buffer(
	void
	)
{
	bool status;
	bit_buffer_get_bytes_safe(NULL, NULL, 0, &status);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_bytes_safe_null_data(
	void
	)
{
	bit_buffer_t bit_buffer;
	bool status;
	bit_buffer_get_bytes_safe(&bit_buffer, NULL, 4, &status);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_bytes_safe_null_status(
	void
	)
{
	bit_buffer_t bit_buffer;
	bit_buffer_get_bytes_safe(&bit_buffer, NULL, 4, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_bytes_safe_null(
	void
	)
{
	bit_buffer_get_bytes_safe(NULL, NULL, 0, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_set_str_null_buffer(
	void
	)
{
	str_t str = str_init_copy_len("hello", 5);
	bit_buffer_set_str(NULL, str);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_str_null_buffer(
	void
	)
{
	bit_buffer_get_str(NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_str_safe_null_buffer(
	void
	)
{
	bool status;
	bit_buffer_get_str_safe(NULL, 0, &status);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_str_safe_null_status(
	void
	)
{
	bit_buffer_t bit_buffer;
	bit_buffer_get_str_safe(&bit_buffer, 0, NULL);
}


void attr_test_fn
test_normal_fail__bit_buffer_get_str_safe_null(
	void
	)
{
	bit_buffer_get_str_safe(NULL, 0, NULL);
}

