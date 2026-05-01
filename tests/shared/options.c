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
#include <shared/color.h>
#include <shared/debug.h>
#include <shared/macro.h>
#include <shared/options.h>

#include <stddef.h>
#include <stdint.h>


void attr_test_fn
test_normal_pass__options_init_free(
	void
	)
{
	options_t options = options_init(0, NULL);
	options_free(options);
}


void attr_test_fn
test_normal_pass__options_parse(
	void
	)
{
	const char* argv[] =
	{
		"program",
		"--key=value",
		"--foo",
		"--ba",
		"--barr=",
		"123",
		"--",
		"-",
		"",
		"--=1",
		"--=2"
	};

	options_t options = options_init(MACRO_ARRAY_LEN(argv), argv);

	assert_false(options_exists(options, "program"));

	assert_true(options_exists(options, "key"));
	str_t value;
	bool status = options_get_str(options, "key", &value);
	assert_true(status);
	assert_true(str_cmp_cstr(value, "value"));

	assert_true(options_exists(options, "foo"));
	status = options_get_str(options, "foo", &value);
	assert_true(status);
	assert_null(value);

	assert_true(options_exists(options, "ba"));
	status = options_get_str(options, "ba", &value);
	assert_true(status);
	assert_null(value);

	assert_false(options_exists(options, "bar"));

	status = options_get_str(options, "barr", &value);
	assert_true(status);
	assert_true(str_cmp_cstr(value, ""));

	status = options_get_str(options, "grr", &value);
	assert_false(status);

	assert_false(options_exists(options, "123"));

	assert_true(options_exists(options, ""));
	status = options_get_str(options, "", &value);
	assert_true(status);
	assert_true(str_cmp_cstr(value, "2"));

	assert_false(options_exists(options, "-"));

	options_free(options);
}


void attr_test_fn
test_normal_pass__options_get_typed(
	void
	)
{
	const char* argv[] =
	{
		"program",
		"--int_valid=42",
		"--int_min=-100",
		"--int_max=999",
		"--int_out_of_range=1000",
		"--int_invalid=abc",
		"--float_valid=3.14",
		"--float_invalid=xyz",
		"--bool_true=true",
		"--bool_yes=YES",
		"--bool_one=1",
		"--bool_false=false",
		"--bool_invalid=maybe",
		"--color_short=#ABC",
		"--color_short_alpha=#ABCD",
		"--color_mixed=#ABC12",
		"--color_full=#AABBCC",
		"--color_full_alpha=#AABBCCDD",
		"--color_no_hash=123456",
		"--color_invalid=#GG"
	};

	options_t options = options_init(MACRO_ARRAY_LEN(argv), argv);

	int64_t int_val;
	bool status = options_get_i64(options, "int_valid", 0, 100, &int_val);
	assert_true(status);
	assert_eq(int_val, 42);

	status = options_get_i64(options, "int_min", -200, 100, &int_val);
	assert_true(status);
	assert_eq(int_val, -100);

	status = options_get_i64(options, "int_max", 0, 1000, &int_val);
	assert_true(status);
	assert_eq(int_val, 999);

	status = options_get_i64(options, "int_out_of_range", 0, 100, &int_val);
	assert_false(status);

	status = options_get_i64(options, "int_invalid", 0, 100, &int_val);
	assert_false(status);

	status = options_get_i64(options, "nonexistent", 0, 100, &int_val);
	assert_false(status);

	float float_val;
	status = options_get_f32(options, "float_valid", 0.0f, 10.0f, &float_val);
	assert_true(status);
	assert_eq(float_val, 3.14f);

	status = options_get_f32(options, "float_invalid", 0.0f, 10.0f, &float_val);
	assert_false(status);

	bool bool_val;
	status = options_get_boolean(options, "bool_true", &bool_val);
	assert_true(status);
	assert_true(bool_val);

	status = options_get_boolean(options, "bool_yes", &bool_val);
	assert_true(status);
	assert_true(bool_val);

	status = options_get_boolean(options, "bool_one", &bool_val);
	assert_true(status);
	assert_true(bool_val);

	status = options_get_boolean(options, "bool_false", &bool_val);
	assert_true(status);
	assert_false(bool_val);

	status = options_get_boolean(options, "bool_invalid", &bool_val);
	assert_true(status);
	assert_false(bool_val);

	color_argb_t color_val;
	status = options_get_color(options, "color_short", &color_val);
	assert_true(status);
	assert_eq(color_val.r, 0xAA);
	assert_eq(color_val.g, 0xBB);
	assert_eq(color_val.b, 0xCC);
	assert_eq(color_val.a, 255);

	status = options_get_color(options, "color_short_alpha", &color_val);
	assert_true(status);
	assert_eq(color_val.r, 0xAA);
	assert_eq(color_val.g, 0xBB);
	assert_eq(color_val.b, 0xCC);
	assert_eq(color_val.a, 0xDD);

	status = options_get_color(options, "color_mixed", &color_val);
	assert_true(status);
	assert_eq(color_val.r, 0xAA);
	assert_eq(color_val.g, 0xBB);
	assert_eq(color_val.b, 0xCC);
	assert_eq(color_val.a, 0x12);

	status = options_get_color(options, "color_full", &color_val);
	assert_true(status);
	assert_eq(color_val.r, 0xAA);
	assert_eq(color_val.g, 0xBB);
	assert_eq(color_val.b, 0xCC);
	assert_eq(color_val.a, 255);

	status = options_get_color(options, "color_full_alpha", &color_val);
	assert_true(status);
	assert_eq(color_val.r, 0xAA);
	assert_eq(color_val.g, 0xBB);
	assert_eq(color_val.b, 0xCC);
	assert_eq(color_val.a, 0xDD);

	status = options_get_color(options, "color_no_hash", &color_val);
	assert_true(status);
	assert_eq(color_val.r, 0x12);
	assert_eq(color_val.g, 0x34);
	assert_eq(color_val.b, 0x56);
	assert_eq(color_val.a, 255);

	status = options_get_color(options, "color_invalid", &color_val);
	assert_false(status);

	options_free(options);
}


void attr_test_fn
test_normal_fail__options_free_null(
	void
	)
{
	options_free(NULL);
}


void attr_test_fn
test_normal_fail__options_exists_null_options(
	void
	)
{
	options_exists(NULL, "key");
}


void attr_test_fn
test_normal_fail__options_exists_null_key(
	void
	)
{
	options_t options = options_init(0, NULL);
	options_exists(options, NULL);
}


void attr_test_fn
test_normal_fail__options_exists_null(
	void
	)
{
	options_exists(NULL, NULL);
}


void attr_test_fn
test_normal_fail__options_get_i64_null_options(
	void
	)
{
	int64_t value;
	options_get_i64(NULL, "key", 0, 100, &value);
}


void attr_test_fn
test_normal_fail__options_get_i64_null_key(
	void
	)
{
	options_t options = options_init(0, NULL);
	int64_t value;
	options_get_i64(options, NULL, 0, 100, &value);
}


void attr_test_fn
test_normal_fail__options_get_i64_null_out_value(
	void
	)
{
	options_t options = options_init(0, NULL);
	options_get_i64(options, "key", 0, 100, NULL);
}


void attr_test_fn
test_normal_fail__options_get_i64_null(
	void
	)
{
	options_get_i64(NULL, NULL, 0, 100, NULL);
}


void attr_test_fn
test_normal_fail__options_get_f32_null_options(
	void
	)
{
	float value;
	options_get_f32(NULL, "key", 0.0f, 100.0f, &value);
}


void attr_test_fn
test_normal_fail__options_get_f32_null_key(
	void
	)
{
	options_t options = options_init(0, NULL);
	float value;
	options_get_f32(options, NULL, 0.0f, 100.0f, &value);
}


void attr_test_fn
test_normal_fail__options_get_f32_null_out_value(
	void
	)
{
	options_t options = options_init(0, NULL);
	options_get_f32(options, "key", 0.0f, 100.0f, NULL);
}


void attr_test_fn
test_normal_fail__options_get_f32_null(
	void
	)
{
	options_get_f32(NULL, NULL, 0.0f, 100.0f, NULL);
}


void attr_test_fn
test_normal_fail__options_get_boolean_null_options(
	void
	)
{
	bool value;
	options_get_boolean(NULL, "key", &value);
}


void attr_test_fn
test_normal_fail__options_get_boolean_null_key(
	void
	)
{
	options_t options = options_init(0, NULL);
	bool value;
	options_get_boolean(options, NULL, &value);
}


void attr_test_fn
test_normal_fail__options_get_boolean_null_out_value(
	void
	)
{
	options_t options = options_init(0, NULL);
	options_get_boolean(options, "key", NULL);
}


void attr_test_fn
test_normal_fail__options_get_boolean_null(
	void
	)
{
	options_get_boolean(NULL, NULL, NULL);
}


void attr_test_fn
test_normal_fail__options_get_str_null_options(
	void
	)
{
	str_t value;
	options_get_str(NULL, "key", &value);
}


void attr_test_fn
test_normal_fail__options_get_str_null_key(
	void
	)
{
	options_t options = options_init(0, NULL);
	str_t value;
	options_get_str(options, NULL, &value);
}


void attr_test_fn
test_normal_fail__options_get_str_null_out_value(
	void
	)
{
	options_t options = options_init(0, NULL);
	options_get_str(options, "key", NULL);
}


void attr_test_fn
test_normal_fail__options_get_str_null(
	void
	)
{
	options_get_str(NULL, NULL, NULL);
}


void attr_test_fn
test_normal_fail__options_get_color_null_options(
	void
	)
{
	color_argb_t value;
	options_get_color(NULL, "key", &value);
}


void attr_test_fn
test_normal_fail__options_get_color_null_key(
	void
	)
{
	options_t options = options_init(0, NULL);
	color_argb_t value;
	options_get_color(options, NULL, &value);
}


void attr_test_fn
test_normal_fail__options_get_color_null_out_value(
	void
	)
{
	options_t options = options_init(0, NULL);
	options_get_color(options, "key", NULL);
}


void attr_test_fn
test_normal_fail__options_get_color_null(
	void
	)
{
	options_get_color(NULL, NULL, NULL);
}
