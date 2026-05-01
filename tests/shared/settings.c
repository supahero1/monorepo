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
#include <tests/base.h>
#include <shared/attr.h>
#include <shared/time.h>
#include <shared/color.h>
#include <shared/debug.h>
#include <shared/event.h>
#include <shared/macro.h>
#include <shared/options.h>
#include <shared/settings.h>

#include <stdint.h>
#include <string.h>

#define TEST_FILENAME	\
(test_is_on_valgrind ? "bin/tests/shared/settings.bin.val" : "bin/tests/shared/settings.bin")


void attr_test_fn
test_normal_pass__settings_init_free(
	void
	)
{
	time_timers_t timers = time_timers_init();

	settings_t settings = settings_init(TEST_FILENAME, timers);
	settings_free(settings);

	time_timers_free(timers);
}


void
settings_onsave_fn(
	bool* save,
	settings_save_event_data_t* data
	)
{
	assert_true(data->success);
	*save = true;
}


void
settings_onload_fn(
	bool* load,
	settings_load_event_data_t* data
	)
{
	assert_true(data->success);
	*load = true;
}


void
setting_change_i64_fn(
	int64_t* change,
	setting_change_event_data_t* data
	)
{
	*change = data->new_value.i64;
}


void
setting_change_f32_fn(
	float* change,
	setting_change_event_data_t* data
	)
{
	*change = data->new_value.f32;
}


void attr_test_fn
test_normal_pass__settings_save_load(
	void
	)
{
	time_timers_t timers = time_timers_init();
	settings_t settings = settings_init(TEST_FILENAME, timers);

	settings_event_table_t* event_table = settings_get_event_table(settings);

	bool saved = false;
	event_listener_data_t save_listener_data =
	{
		.fn = (void*) settings_onsave_fn,
		.data = &saved
	};
	event_listener_t* save_listener = event_target_add(&event_table->save_target, save_listener_data);

	bool loaded = false;
	event_listener_data_t load_listener_data =
	{
		.fn = (void*) settings_onload_fn,
		.data = &loaded
	};
	event_listener_t* load_listener = event_target_add(&event_table->load_target, load_listener_data);

	event_target_t i64_change_target;
	event_target_init(&i64_change_target);

	int64_t i64_change = 0;
	event_listener_data_t i64_listener_data =
	{
		.fn = (void*) setting_change_i64_fn,
		.data = &i64_change
	};
	event_listener_t* i64_listener = event_target_add(&i64_change_target, i64_listener_data);

	setting_t* i64_s = settings_add_i64(settings, "foo", 42, 0, 100, &i64_change_target);
	assert_eq(i64_change, 0);
	assert_false(saved);
	assert_false(loaded);

	settings_add_i64(settings, "foobar", 2, 1, 3, NULL);
	assert_eq(i64_change, 0);
	assert_false(saved);
	assert_false(loaded);

	assert_eq(setting_get_i64(i64_s), 42);

	event_target_t f32_change_target;
	event_target_init(&f32_change_target);

	float f32_change = 0.0f;
	event_listener_data_t f32_listener_data =
	{
		.fn = (void*) setting_change_f32_fn,
		.data = &f32_change
	};
	event_listener_t* f32_listener = event_target_add(&f32_change_target, f32_listener_data);

	settings_add_f32(settings, "barfoo", 2.0f, 1.0f, 3.0f, NULL);
	assert_eq(f32_change, 0.0f);
	assert_false(saved);
	assert_false(loaded);

	setting_t* f32_s = settings_add_f32(settings, "bar", 1.23f, 0.0f, 2.0f, &f32_change_target);
	assert_eq(f32_change, 0.0f);
	assert_false(saved);
	assert_false(loaded);

	assert_eq(setting_get_f32(f32_s), 1.23f);

	settings_seal(settings);

	assert_eq(setting_get_i64(i64_s), 42);
	assert_eq(setting_get_f32(f32_s), 1.23f);

	settings_save(settings);
	assert_eq(i64_change, 0);
	assert_eq(f32_change, 0.0f);
	assert_false(saved);
	assert_false(loaded);

	settings_modify_i64(settings, i64_s, 200);
	assert_eq(i64_change, 100); i64_change = 0;
	assert_false(saved);
	assert_false(loaded);

	settings_modify_i64(settings, i64_s, 43);
	assert_eq(i64_change, 43); i64_change = 0;
	assert_false(saved);
	assert_false(loaded);

	settings_modify_f32(settings, f32_s, 2.5f);
	assert_eq(f32_change, 2.0f); f32_change = 0.0f;
	assert_false(saved);
	assert_false(loaded);

	settings_save(settings);
	assert_eq(i64_change, 0);
	assert_eq(f32_change, 0.0f);
	assert_true(saved); saved = false;
	assert_false(loaded);

	event_target_del(&event_table->load_target, load_listener);
	event_target_del(&event_table->save_target, save_listener);

	settings_free(settings);
	time_timers_free(timers);

	settings = settings_init(TEST_FILENAME, NULL);
	assert_eq(i64_change, 0);
	assert_eq(f32_change, 0.0f);
	assert_false(saved);
	assert_false(loaded);

	event_table = settings_get_event_table(settings);

	save_listener = event_target_add(&event_table->save_target, save_listener_data);
	load_listener = event_target_add(&event_table->load_target, load_listener_data);

	settings_add_f32(settings, "barfoo", 2.0f, 1.0f, 3.0f, NULL);
	assert_eq(f32_change, 0.0f);
	assert_false(saved);
	assert_false(loaded);

	i64_s = settings_add_i64(settings, "foo", 44, 40, 45, &i64_change_target);
	assert_eq(i64_change, 0);
	assert_false(saved);
	assert_false(loaded);

	f32_s = settings_add_f32(settings, "bar", 1.5f, 1.0f, 3.0f, &f32_change_target);
	assert_eq(f32_change, 0.0f);
	assert_false(saved);
	assert_false(loaded);

	settings_add_i64(settings, "foobar", 2, 1, 3, NULL);
	assert_eq(i64_change, 0);
	assert_false(saved);
	assert_false(loaded);

	settings_seal(settings);

	settings_load(settings);
	assert_eq(i64_change, 43); i64_change = 0;
	assert_eq(setting_get_i64(i64_s), 43);
	assert_eq(f32_change, 2.0f); f32_change = 0.0f;
	assert_eq(setting_get_f32(f32_s), 2.0f);
	assert_false(saved);
	assert_true(loaded); loaded = false;

	event_target_del(&f32_change_target, f32_listener);
	event_target_free(&f32_change_target);

	event_target_del(&i64_change_target, i64_listener);
	event_target_free(&i64_change_target);

	event_target_del(&event_table->load_target, load_listener);
	event_target_del(&event_table->save_target, save_listener);

	settings_free(settings);
}


void attr_test_fn
test_normal_fail__settings_init_null_path(
	void
	)
{
	settings_init(NULL, NULL);
}


void attr_test_fn
test_normal_fail__settings_free_null(
	void
	)
{
	settings_free(NULL);
}


void attr_test_fn
test_normal_fail__settings_save_null(
	void
	)
{
	settings_save(NULL);
}


void attr_test_fn
test_normal_fail__settings_load_null(
	void
	)
{
	settings_load(NULL);
}


void attr_test_fn
test_normal_fail__settings_load_unsealed(
	void
	)
{
	settings_t settings = settings_init(TEST_FILENAME, NULL);
	settings_load(settings);
}


void attr_test_fn
test_normal_fail__settings_add_i64_null_settings(
	void
	)
{
	settings_add_i64(NULL, "0", 0, 0, 0, NULL);
}


void attr_test_fn
test_normal_fail__settings_add_i64_null_name(
	void
	)
{
	settings_t settings = settings_init(TEST_FILENAME, NULL);

	settings_add_i64(settings, NULL, 0, 0, 0, NULL);
}


void attr_test_fn
test_normal_fail__settings_add_i64_null(
	void
	)
{
	settings_add_i64(NULL, NULL, 0, 0, 0, NULL);
}


void attr_test_fn
test_normal_fail__settings_add_i64_invalid_value(
	void
	)
{
	settings_t settings = settings_init(TEST_FILENAME, NULL);

	settings_add_i64(settings, "0", 1, 0, 0, NULL);
}


void attr_test_fn
test_normal_fail__settings_add_i64_invalid_constaint(
	void
	)
{
	settings_t settings = settings_init(TEST_FILENAME, NULL);

	settings_add_i64(settings, "0", 0, 1, 0, NULL);
}


void attr_test_fn
test_normal_fail__settings_add_f32_null_settings(
	void
	)
{
	settings_add_f32(NULL, "0", 0.0f, 0.0f, 0.0f, NULL);
}


void attr_test_fn
test_normal_fail__settings_add_f32_null_name(
	void
	)
{
	settings_t settings = settings_init(TEST_FILENAME, NULL);

	settings_add_f32(settings, NULL, 0.0f, 0.0f, 0.0f, NULL);
}


void attr_test_fn
test_normal_fail__settings_add_f32_null(
	void
	)
{
	settings_add_f32(NULL, NULL, 0.0f, 0.0f, 0.0f, NULL);
}


void attr_test_fn
test_normal_fail__settings_add_f32_invalid_value(
	void
	)
{
	settings_t settings = settings_init(TEST_FILENAME, NULL);

	settings_add_f32(settings, "0", 1.0f, 0.0f, 0.0f, NULL);
}


void attr_test_fn
test_normal_fail__settings_add_f32_invalid_constaint(
	void
	)
{
	settings_t settings = settings_init(TEST_FILENAME, NULL);

	settings_add_f32(settings, "0", 0.0f, 1.0f, 0.0f, NULL);
}


void attr_test_fn
test_normal_fail__settings_add_boolean_null_settings(
	void
	)
{
	settings_add_boolean(NULL, "0", false, NULL);
}


void attr_test_fn
test_normal_fail__settings_add_boolean_null_name(
	void
	)
{
	settings_t settings = settings_init(TEST_FILENAME, NULL);

	settings_add_boolean(settings, NULL, false, NULL);
}


void attr_test_fn
test_normal_fail__settings_add_boolean_null(
	void
	)
{
	settings_add_boolean(NULL, NULL, false, NULL);
}


void attr_test_fn
test_normal_fail__settings_add_str_null_settings(
	void
	)
{
	str_t str = str_init();

	settings_add_str(NULL, "0", str, 0, NULL);
}


void attr_test_fn
test_normal_fail__settings_add_str_null_name(
	void
	)
{
	settings_t settings = settings_init(TEST_FILENAME, NULL);

	str_t str = str_init();

	settings_add_str(settings, NULL, str, 0, NULL);
}


void attr_test_fn
test_normal_fail__settings_add_str_null(
	void
	)
{
	str_t str = str_init();

	settings_add_str(NULL, NULL, str, 0, NULL);
}


void attr_test_fn
test_normal_fail__settings_add_color_null_settings(
	void
	)
{
	color_argb_t color = {0};
	settings_add_color(NULL, "0", color, NULL);
}


void attr_test_fn
test_normal_fail__settings_add_color_null_name(
	void
	)
{
	settings_t settings = settings_init(TEST_FILENAME, NULL);

	color_argb_t color = {0};
	settings_add_color(settings, NULL, color, NULL);
}


void attr_test_fn
test_normal_fail__settings_add_color_null(
	void
	)
{
	color_argb_t color = {0};
	settings_add_color(NULL, NULL, color, NULL);
}


void attr_test_fn
test_normal_fail__settings_modify_i64_null_settings(
	void
	)
{
	settings_modify_i64(NULL, TEST_PTR, 0);
}


void attr_test_fn
test_normal_fail__settings_modify_i64_null_name(
	void
	)
{
	settings_modify_i64(TEST_PTR, NULL, 0);
}


void attr_test_fn
test_normal_fail__settings_modify_i64_null(
	void
	)
{
	settings_modify_i64(NULL, NULL, 0);
}


void attr_test_fn
test_normal_fail__settings_modify_f32_null_settings(
	void
	)
{
	settings_modify_f32(NULL, TEST_PTR, 0.0f);
}


void attr_test_fn
test_normal_fail__settings_modify_f32_null_name(
	void
	)
{
	settings_modify_f32(TEST_PTR, NULL, 0.0f);
}


void attr_test_fn
test_normal_fail__settings_modify_f32_null(
	void
	)
{
	settings_modify_f32(NULL, NULL, 0.0f);
}


void attr_test_fn
test_normal_fail__settings_modify_boolean_null_settings(
	void
	)
{
	settings_modify_boolean(NULL, TEST_PTR, false);
}


void attr_test_fn
test_normal_fail__settings_modify_boolean_null_name(
	void
	)
{
	settings_modify_boolean(TEST_PTR, NULL, false);
}


void attr_test_fn
test_normal_fail__settings_modify_boolean_null(
	void
	)
{
	settings_modify_boolean(NULL, NULL, false);
}


void attr_test_fn
test_normal_fail__settings_modify_str_null_settings(
	void
	)
{
	str_t str = str_init();

	settings_modify_str(NULL, TEST_PTR, str);
}


void attr_test_fn
test_normal_fail__settings_modify_str_null_name(
	void
	)
{
	settings_t settings = settings_init(TEST_FILENAME, NULL);

	str_t str = str_init();

	settings_modify_str(settings, NULL, str);
}


void attr_test_fn
test_normal_fail__settings_modify_str_null(
	void
	)
{
	str_t str = str_init();

	settings_modify_str(NULL, NULL, str);
}


void attr_test_fn
test_normal_fail__settings_modify_color_null_settings(
	void
	)
{
	color_argb_t color = {0};
	settings_modify_color(NULL, TEST_PTR, color);
}


void attr_test_fn
test_normal_fail__settings_modify_color_null_name(
	void
	)
{
	settings_t settings = settings_init(TEST_FILENAME, NULL);

	color_argb_t color = {0};
	settings_modify_color(settings, NULL, color);
}


void attr_test_fn
test_normal_fail__settings_modify_color_null(
	void
	)
{
	color_argb_t color = {0};
	settings_modify_color(NULL, NULL, color);
}


void attr_test_fn
test_normal_fail__setting_get_i64_null(
	void
	)
{
	setting_get_i64(NULL);
}


void attr_test_fn
test_normal_fail__setting_get_f32_null(
	void
	)
{
	setting_get_f32(NULL);
}


void attr_test_fn
test_normal_fail__setting_get_boolean_null(
	void
	)
{
	setting_get_boolean(NULL);
}


void attr_test_fn
test_normal_fail__setting_get_str_null(
	void
	)
{
	setting_get_str(NULL);
}


void attr_test_fn
test_normal_fail__setting_get_color_null(
	void
	)
{
	setting_get_color(NULL);
}


void attr_test_fn
test_normal_pass__settings_with_options_override(
	void
	)
{
	const char* argv[] =
	{
		"program",
		"--test_i64=99",
		"--test_f32=3.5",
		"--test_bool=true",
		"--test_str=cmdline",
		"--test_color=FF0000"
	};

	options_t options = options_init(MACRO_ARRAY_LEN(argv), argv);
	global_options = options;

	settings_t settings = settings_init(TEST_FILENAME, NULL);

	setting_t* i64_s = settings_add_i64(settings, "test_i64", 42, 0, 100, NULL);
	assert_eq(setting_get_i64(i64_s), 99);

	setting_t* f32_s = settings_add_f32(settings, "test_f32", 1.0f, 0.0f, 10.0f, NULL);
	assert_eq(setting_get_f32(f32_s), 3.5f);

	setting_t* bool_s = settings_add_boolean(settings, "test_bool", false, NULL);
	assert_true(setting_get_boolean(bool_s));

	str_t default_str = str_init_copy_cstr("default");
	setting_t* str_s = settings_add_str(settings, "test_str", default_str, 100, NULL);
	str_t str_val = setting_get_str(str_s);
	assert_true(str_cmp_cstr(str_val, "cmdline"));

	color_argb_t default_color = { .r = 0, .g = 0, .b = 0, .a = 255 };
	setting_t* color_s = settings_add_color(settings, "test_color", default_color, NULL);
	color_argb_t color_val = setting_get_color(color_s);
	assert_eq(color_val.r, 255);
	assert_eq(color_val.g, 0);
	assert_eq(color_val.b, 0);
	assert_eq(color_val.a, 255);

	settings_seal(settings);
	settings_free(settings);

	global_options = NULL;
	options_free(options);
}
