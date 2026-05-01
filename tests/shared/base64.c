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
#include <shared/debug.h>
#include <shared/macro.h>
#include <shared/base64.h>
#include <shared/alloc/base.h>

#include <stdint.h>
#include <string.h>


uint8_t table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


void
run_encode_test(
	const uint8_t* data,
	uint64_t in_len,
	const uint8_t* expected,
	uint64_t expected_len
	)
{
	uint64_t out_len;
	uint8_t* encoded = base64_encode(data, in_len, &out_len);
	assert_ptr(encoded, out_len);

	assert_eq(out_len, expected_len);
	assert_false(memcmp(encoded, expected, out_len));

	alloc_free(encoded, out_len);
}


void
run_decode_test(
	const uint8_t* data,
	uint64_t in_len,
	const uint8_t* expected,
	uint64_t expected_len
	)
{
	uint64_t out_len;
	uint8_t* decoded = base64_decode(data, in_len, &out_len);
	assert_ptr(decoded, out_len);

	assert_eq(out_len, expected_len);
	assert_false(memcmp(decoded, expected, out_len));

	alloc_free(decoded, out_len);
}


void attr_test_fn
test_normal_pass__base64_encode_empty(
	void
	)
{
	run_encode_test((uint8_t[]){}, 0,
		(uint8_t[]){}, 0);
}


void attr_test_fn
test_normal_pass__base64_encode_mod_0(
	void
	)
{
	run_encode_test((uint8_t[]){ 0x00, 0x00, 0x00 }, 3,
		(uint8_t[]){ 'A', 'A', 'A', 'A' }, 4);
}


void attr_test_fn
test_normal_pass__base64_encode_mod_1(
	void
	)
{
	run_encode_test((uint8_t[]){ 0x00, 0x00 }, 2,
		(uint8_t[]){ 'A', 'A', 'A', '=' }, 4);
}


void attr_test_fn
test_normal_pass__base64_encode_all_mod_1(
	void
	)
{
	for(uint8_t i = 0; i < 0xFF; ++i)
	{
		for(uint8_t j = 0; j < 0xFF; ++j)
		{
			uint8_t byte1 = i >> 2;
			uint8_t byte2 = ((i & 0x03) << 4) | (j >> 4);
			uint8_t byte3 = (j & 0x0F) << 2;
			run_encode_test((uint8_t[]){ i, j }, 2,
				(uint8_t[]){ table[byte1], table[byte2], table[byte3], '=' }, 4);
		}
	}
}


void attr_test_fn
test_normal_pass__base64_encode_mod_2(
	void
	)
{
	run_encode_test((uint8_t[]){ 0x00 }, 1,
		(uint8_t[]){ 'A', 'A', '=', '=' }, 4);
}


void attr_test_fn
test_normal_pass__base64_encode_all_mod_2(
	void
	)
{
	for(uint8_t i = 0; i < 0xFF; ++i)
	{
		uint8_t byte1 = i >> 2;
		uint8_t byte2 = (i & 0x03) << 4;
		run_encode_test((uint8_t[]){ i }, 1,
			(uint8_t[]){ table[byte1], table[byte2], '=', '=' }, 4);
	}
}


void attr_test_fn
test_normal_pass__base64_encode_f(
	void
	)
{
	run_encode_test((uint8_t[]){ 'f' }, 1,
		(uint8_t[]){ 'Z', 'g', '=', '=' }, 4);
}


void attr_test_fn
test_normal_pass__base64_encode_fo(
	void
	)
{
	run_encode_test((uint8_t[]){ 'f', 'o' }, 2,
		(uint8_t[]){ 'Z', 'm', '8', '=' }, 4);
}


void attr_test_fn
test_normal_pass__base64_encode_foo(
	void
	)
{
	run_encode_test((uint8_t[]){ 'f', 'o', 'o' }, 3,
		(uint8_t[]){ 'Z', 'm', '9', 'v' }, 4);
}


void attr_test_fn
test_normal_pass__base64_encode_foob(
	void
	)
{
	run_encode_test((uint8_t[]){ 'f', 'o', 'o', 'b' }, 4,
		(uint8_t[]){ 'Z', 'm', '9', 'v', 'Y', 'g', '=', '=' }, 8);
}


void attr_test_fn
test_normal_pass__base64_encode_fooba(
	void
	)
{
	run_encode_test((uint8_t[]){ 'f', 'o', 'o', 'b', 'a' }, 5,
		(uint8_t[]){ 'Z', 'm', '9', 'v', 'Y', 'm', 'E', '=' }, 8);
}


void attr_test_fn
test_normal_pass__base64_encode_foobar(
	void
	)
{
	run_encode_test((uint8_t[]){ 'f', 'o', 'o', 'b', 'a', 'r' }, 6,
		(uint8_t[]){ 'Z', 'm', '9', 'v', 'Y', 'm', 'F', 'y' }, 8);
}


void attr_test_fn
test_normal_pass__base64_encode_no_out_len(
	void
	)
{
	uint8_t* encoded = base64_encode((uint8_t[]){ 'f', 'o', 'o', 'b', 'a', 'r' }, 6, NULL);
	assert_not_null(encoded);

	alloc_free(encoded, 8);
}


void attr_test_fn
test_normal_pass__base64_encode_null_zero_len(
	void
	)
{
	uint8_t* encoded = base64_encode(NULL, 0, NULL);
	assert_null(encoded);
}


void attr_test_fn
test_normal_fail__base64_encode_null_nonzero_len(
	void
	)
{
	base64_encode(NULL, 1, NULL);
}


void attr_test_fn
test_normal_pass__base64_decode_empty(
	void
	)
{
	run_decode_test((uint8_t[]){}, 0,
		(uint8_t[]){}, 0);
}


void attr_test_fn
test_normal_pass__base64_decode_mod_0(
	void
	)
{
	run_decode_test((uint8_t[]){ 'A', 'A', 'A', 'A' }, 4,
		(uint8_t[]){ 0x00, 0x00, 0x00 }, 3);
}


void attr_test_fn
test_normal_pass__base64_decode_mod_1(
	void
	)
{
	run_decode_test((uint8_t[]){ 'A', 'A', 'A', '=' }, 4,
		(uint8_t[]){ 0x00, 0x00 }, 2);
}


void attr_test_fn
test_normal_pass__base64_decode_some_mod_1(
	void
	)
{
	for(uint8_t i = 0; i < 0x10; ++i)
	{
		for(uint8_t j = 0; j < 0x10; ++j)
		{
			uint8_t byte1 = i >> 2;
			uint8_t byte2 = ((i & 0x03) << 4) | (j >> 4);
			uint8_t byte3 = (j & 0x0F) << 2;
			run_decode_test((uint8_t[]){ table[byte1], table[byte2], table[byte3], '=' }, 4,
				(uint8_t[]){ i, j }, 2);
		}
	}
}


void attr_test_fn
test_normal_pass__base64_decode_mod_2(
	void
	)
{
	run_decode_test((uint8_t[]){ 'A', 'A', '=', '=' }, 4,
		(uint8_t[]){ 0x00 }, 1);
}


void attr_test_fn
test_normal_pass__base64_decode_all_mod_2(
	void
	)
{
	for(uint8_t i = 0; i < 0xFF; ++i)
	{
		uint8_t byte1 = i >> 2;
		uint8_t byte2 = (i & 0x03) << 4;
		run_decode_test((uint8_t[]){ table[byte1], table[byte2], '=', '=' }, 4,
			(uint8_t[]){ i }, 1);
	}
}


void attr_test_fn
test_normal_pass__base64_decode_f(
	void
	)
{
	run_decode_test((uint8_t[]){ 'Z', 'g', '=', '=' }, 4,
		(uint8_t[]){ 'f' }, 1);
}


void attr_test_fn
test_normal_pass__base64_decode_fo(
	void
	)
{
	run_decode_test((uint8_t[]){ 'Z', 'm', '8', '=' }, 4,
		(uint8_t[]){ 'f', 'o' }, 2);
}


void attr_test_fn
test_normal_pass__base64_decode_foo(
	void
	)
{
	run_decode_test((uint8_t[]){ 'Z', 'm', '9', 'v' }, 4,
		(uint8_t[]){ 'f', 'o', 'o' }, 3);
}


void attr_test_fn
test_normal_pass__base64_decode_foob(
	void
	)
{
	run_decode_test((uint8_t[]){ 'Z', 'm', '9', 'v', 'Y', 'g', '=', '=' }, 8,
		(uint8_t[]){ 'f', 'o', 'o', 'b' }, 4);
}


void attr_test_fn
test_normal_pass__base64_decode_fooba(
	void
	)
{
	run_decode_test((uint8_t[]){ 'Z', 'm', '9', 'v', 'Y', 'm', 'E', '=' }, 8,
		(uint8_t[]){ 'f', 'o', 'o', 'b', 'a' }, 5);
}


void attr_test_fn
test_normal_pass__base64_decode_foobar(
	void
	)
{
	run_decode_test((uint8_t[]){ 'Z', 'm', '9', 'v', 'Y', 'm', 'F', 'y' }, 8,
		(uint8_t[]){ 'f', 'o', 'o', 'b', 'a', 'r' }, 6);
}


void attr_test_fn
test_normal_pass__base64_decode_no_out_len(
	void
	)
{
	uint8_t* decoded = base64_decode((uint8_t[]){ 'Z', 'm', '9', 'v', 'Y', 'm', 'F', 'y' }, 8, NULL);
	assert_not_null(decoded);

	alloc_free(decoded, 6);
}


void attr_test_fn
test_normal_pass__base64_decode_null_zero_len(
	void
	)
{
	uint8_t* decoded = base64_decode(NULL, 0, NULL);
	assert_null(decoded);
}


void attr_test_fn
test_normal_fail__base64_decode_null_nonzero_len(
	void
	)
{
	base64_decode(NULL, 1, NULL);
}


void attr_test_fn
test_normal_fail__base64_decode_bad_input(
	void
	)
{
	base64_decode((uint8_t[]){ ' ', ' ', ' ', ' ' }, 4, NULL);
}


void attr_test_fn
test_normal_fail__base64_decode_bad_len_mod_1(
	void
	)
{
	base64_decode((uint8_t[]){ 'A', 'A', 'A', 'A', 'A', 'A', 'A' }, 7, NULL);
}


void attr_test_fn
test_normal_fail__base64_decode_bad_len_mod_2(
	void
	)
{
	base64_decode((uint8_t[]){ 'A', 'A', 'A', 'A', 'A', 'A' }, 6, NULL);
}


void attr_test_fn
test_normal_fail__base64_decode_bad_len_mod_3(
	void
	)
{
	base64_decode((uint8_t[]){ 'A', 'A', 'A', 'A', 'A' }, 5, NULL);
}
