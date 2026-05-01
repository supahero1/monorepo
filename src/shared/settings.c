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
#include <shared/file.h>
#include <shared/hash.h>
#include <shared/sync.h>
#include <shared/time.h>
#include <shared/color.h>
#include <shared/debug.h>
#include <shared/event.h>
#include <shared/macro.h>
#include <shared/options.h>
#include <shared/threads.h>
#include <shared/settings.h>
#include <shared/alloc/base.h>
#include <shared/bit_buffer.h>

#include <zstd.h>
#include <stddef.h>
#include <stdint.h>


struct setting
{
	sync_mtx_t mtx;
	setting_type_t type;
	const char* name;
	setting_value_t value;
	setting_constraint_t constraint;
	event_target_t* change_target;
};

struct settings
{
	sync_rwlock_t rwlock;

	hash_table_t table;
	bool dirty;
	bool use_timers;
	bool sealed;

	const char* path;

	time_timers_t timers;
	time_timer_t save_timer;

	settings_event_table_t event_table;
};


void
settings_value_free_fn(
	setting_t* setting
	)
{
	if(setting->type == SETTING_TYPE_STR)
	{
		str_free(setting->value.str);
	}

	sync_mtx_free(&setting->mtx);

	alloc_free(setting, 1);
}


settings_t
settings_init(
	const char* path,
	time_timers_t timers
	)
{
	assert_not_null(path);

	settings_t settings = alloc_malloc(settings, 1);
	assert_ptr(settings, 1);

	sync_rwlock_init(&settings->rwlock);

	settings->table = hash_table_init(256, NULL, (void*) settings_value_free_fn);
	settings->dirty = false;
	settings->use_timers = !!timers;
	settings->sealed = false;

	settings->path = path;

	settings->timers = timers;
	time_timer_init(&settings->save_timer);

	event_target_init(&settings->event_table.save_target);
	event_target_init(&settings->event_table.load_target);

	return settings;
}


void
settings_free(
	settings_t settings
	)
{
	assert_not_null(settings);

	if(
		settings->use_timers &&
		time_timers_cancel_timeout(settings->timers, &settings->save_timer)
		)
	{
		settings_save(settings);
	}

	event_target_free(&settings->event_table.load_target);
	event_target_free(&settings->event_table.save_target);

	time_timer_free(&settings->save_timer);

	hash_table_free(settings->table);

	sync_rwlock_free(&settings->rwlock);

	alloc_free(settings, 1);
}


void
settings_for_each_sum_fn(
	str_t name,
	setting_t* setting,
	uint64_t* sum
	)
{
	*sum += bit_buffer_len_str(name->len);
	*sum += bit_buffer_len_bits_var(setting->type, SETTING_TYPE__BITS);


	switch(setting->type)
	{

	case SETTING_TYPE_I64:
	{
		*sum += bit_buffer_len_signed_bits_var(setting->value.i64, 7);
		break;
	}

	case SETTING_TYPE_F32:
	{
		*sum += bit_buffer_len_bytes(sizeof(float));
		break;
	}

	case SETTING_TYPE_BOOLEAN:
	{
		*sum += bit_buffer_len_bits(1);
		break;
	}

	case SETTING_TYPE_STR:
	{
		*sum += bit_buffer_len_str(setting->value.str->len);
		break;
	}

	case SETTING_TYPE_COLOR:
	{
		*sum += bit_buffer_len_bytes(sizeof(color_argb_t));
		break;
	}

	default: assert_unreachable();

	}
}


void
settings_for_each_set_fn(
	str_t name,
	setting_t* setting,
	bit_buffer_t* buffer
	)
{
	bit_buffer_set_str(buffer, name);
	bit_buffer_set_bits_var(buffer, setting->type, SETTING_TYPE__BITS);


	switch(setting->type)
	{

	case SETTING_TYPE_I64:
	{
		bit_buffer_set_signed_bits_var(buffer, setting->value.i64, 7);
		break;
	}

	case SETTING_TYPE_F32:
	{
		bit_buffer_set_bytes(buffer, &setting->value.f32, sizeof(float));
		break;
	}

	case SETTING_TYPE_BOOLEAN:
	{
		bit_buffer_set_bits(buffer, setting->value.boolean, 1);
		break;
	}

	case SETTING_TYPE_STR:
	{
		bit_buffer_set_str(buffer, setting->value.str);
		break;
	}

	case SETTING_TYPE_COLOR:
	{
		bit_buffer_set_bytes(buffer, &setting->value.color, sizeof(color_argb_t));
		break;
	}

	default: assert_unreachable();

	}
}


void
settings_save(
	settings_t settings
	)
{
	assert_not_null(settings);
	assert_true(settings->sealed);

	if(!settings->dirty)
	{
		return;
	}

	sync_rwlock_wrlock(&settings->rwlock);

	uint64_t sum = 0;
	hash_table_for_each(settings->table, (void*) settings_for_each_sum_fn, &sum);
	sum = MACRO_TO_BYTES(sum + 60);

	bit_buffer_t buffer;
	bit_buffer_set(&buffer, alloc_calloc(NULL, sum), sum);
	assert_ptr(buffer.data, sum);

	uint32_t magic = 0x015FF510;
	bit_buffer_set_bits(&buffer, magic, 60);

	hash_table_for_each(settings->table, (void*) settings_for_each_set_fn, &buffer);

	assert_eq(bit_buffer_consumed_bytes(&buffer), sum);

	sync_rwlock_unlock(&settings->rwlock);

	uint64_t compressed_size = ZSTD_compressBound(buffer.len);
	uint8_t* compressed = alloc_malloc(compressed, compressed_size);
	assert_ptr(compressed, compressed_size);

	uint64_t actual_compressed_size = ZSTD_compress(
		compressed, compressed_size, buffer.data, buffer.len, 10);
	int error = ZSTD_isError(actual_compressed_size);
	assert_false(error);

	alloc_free(buffer.data, sum);

	file_t file;
	file.data = compressed;
	file.len = actual_compressed_size;

	bool status = file_write(settings->path, file);
	alloc_free(compressed, compressed_size);

	settings_save_event_data_t save_data =
	{
		.settings = settings,
		.success = status
	};
	event_target_fire(&settings->event_table.save_target, &save_data);
}


void
settings_save_fn(
	settings_t settings
	)
{
	thread_data_t data =
	{
		.fn = (void*) settings_save,
		.data = settings
	};
	thread_init(NULL, data);
}


void
settings_load(
	settings_t settings
	)
{
	assert_not_null(settings);
	assert_true(settings->sealed);

	settings_load_event_data_t load_data =
	{
		.settings = settings,
		.success = false
	};

	file_t file;
	bool status = file_read_cap(settings->path, &file, /* 1MiB */ 0x100000);
	if(!status)
	{
		goto goto_failure;
	}

	uint64_t decompressed_size = ZSTD_getFrameContentSize(file.data, file.len);
	if(decompressed_size > /* 1MiB */ 0x100000)
	{
		goto goto_failure_file;
	}

	uint8_t* decompressed = alloc_malloc(decompressed, decompressed_size);
	if(!decompressed)
	{
		goto goto_failure_file;
	}

	uint64_t actual_decompressed_size = ZSTD_decompress(
		decompressed, decompressed_size, file.data, file.len);
	int error = ZSTD_isError(actual_decompressed_size);
	if(error)
	{
		goto goto_failure_compressed;
	}

	file_free(file);
	file.len = 0;
	file.data = NULL;

	bit_buffer_t buffer;
	bit_buffer_set(&buffer, decompressed, actual_decompressed_size);

	uint32_t magic = bit_buffer_get_bits(&buffer, 60);
	if(magic != 0x015FF510)
	{
		goto goto_failure_compressed;
	}

	uint64_t min_len = bit_buffer_len_str(1) + SETTING_TYPE__BITS + 1;

	while(bit_buffer_available_bits(&buffer) >= min_len)
	{
		str_t name = bit_buffer_get_str_safe(&buffer, 127, &status);
		if(!status)
		{
			goto goto_failure_compressed;
		}

		setting_type_t type = bit_buffer_get_bits_var_safe(&buffer, SETTING_TYPE__BITS, &status);
		if(!status)
		{
			str_free(name);
			goto goto_failure_compressed;
		}


		setting_t* setting = hash_table_get(settings->table, name->str);
		str_free(name);
		if(!setting)
		{
			goto goto_failure_compressed;
		}


		switch(type)
		{

		case SETTING_TYPE_I64:
		{
			int64_t i64 = bit_buffer_get_signed_bits_var_safe(&buffer, 7, &status);
			if(!status)
			{
				goto goto_failure_compressed;
			}

			settings_modify_i64(settings, setting, i64);

			break;
		}

		case SETTING_TYPE_F32:
		{
			float f32;
			bit_buffer_get_bytes_safe(&buffer, &f32, sizeof(float), &status);
			if(!status)
			{
				goto goto_failure_compressed;
			}

			settings_modify_f32(settings, setting, f32);

			break;
		}

		case SETTING_TYPE_BOOLEAN:
		{
			bool boolean = bit_buffer_get_bits_safe(&buffer, 1, &status);
			if(!status)
			{
				goto goto_failure_compressed;
			}

			settings_modify_boolean(settings, setting, boolean);

			break;
		}

		case SETTING_TYPE_STR:
		{
			str_t str = bit_buffer_get_str_safe(&buffer, 16383, &status);
			if(!status)
			{
				goto goto_failure_compressed;
			}

			settings_modify_str(settings, setting, str);

			break;
		}

		case SETTING_TYPE_COLOR:
		{
			color_argb_t color;
			bit_buffer_get_bytes_safe(&buffer, &color, sizeof(color_argb_t), &status);
			if(!status)
			{
				goto goto_failure_compressed;
			}

			settings_modify_color(settings, setting, color);

			break;
		}

		default: assert_unreachable();

		}
	}

	alloc_free(decompressed, decompressed_size);

	load_data.success = true;
	event_target_fire(&settings->event_table.load_target, &load_data);

	return;


	goto_failure_compressed:
	alloc_free(decompressed, decompressed_size);

	goto_failure_file:
	file_free(file);

	goto_failure:
	event_target_fire(&settings->event_table.load_target, &load_data);
}


void
settings_seal(
	settings_t settings
	)
{
	assert_not_null(settings);
	assert_false(settings->sealed);

	settings->sealed = true;
}


settings_event_table_t*
settings_get_event_table(
	settings_t settings
	)
{
	assert_not_null(settings);

	return &settings->event_table;
}


setting_t*
settings_add_i64(
	settings_t settings,
	const char* name,
	int64_t value,
	int64_t min,
	int64_t max,
	event_target_t* change_target
	)
{
	assert_not_null(settings);
	assert_not_null(name);
	assert_false(settings->sealed);

	assert_le(min, max);
	assert_ge(value, min);
	assert_le(value, max);

	setting_t* setting_ptr = alloc_malloc(setting_ptr, 1);
	assert_ptr(setting_ptr, 1);

	*setting_ptr =
	(setting_t)
	{
		.type = SETTING_TYPE_I64,
		.name = name,
		.value.i64 = value,
		.constraint.i64.min = min,
		.constraint.i64.max = max,
		.change_target = change_target
	};

	sync_mtx_init(&setting_ptr->mtx);

	sync_rwlock_rdlock(&settings->rwlock);
		bool status = hash_table_add(settings->table, name, setting_ptr);
		assert_true(status);
	sync_rwlock_unlock(&settings->rwlock);

	return setting_ptr;
}


setting_t*
settings_add_f32(
	settings_t settings,
	const char* name,
	float value,
	float min,
	float max,
	event_target_t* change_target
	)
{
	assert_not_null(settings);
	assert_not_null(name);
	assert_false(settings->sealed);

	assert_le(min, max);
	assert_ge(value, min);
	assert_le(value, max);

	setting_t* setting_ptr = alloc_malloc(setting_ptr, 1);
	assert_ptr(setting_ptr, 1);

	*setting_ptr =
	(setting_t)
	{
		.type = SETTING_TYPE_F32,
		.name = name,
		.value.f32 = value,
		.constraint.f32.min = min,
		.constraint.f32.max = max,
		.change_target = change_target
	};

	sync_mtx_init(&setting_ptr->mtx);

	sync_rwlock_rdlock(&settings->rwlock);
		bool status = hash_table_add(settings->table, name, setting_ptr);
		assert_true(status);
	sync_rwlock_unlock(&settings->rwlock);

	return setting_ptr;
}


setting_t*
settings_add_boolean(
	settings_t settings,
	const char* name,
	bool value,
	event_target_t* change_target
	)
{
	assert_not_null(settings);
	assert_not_null(name);
	assert_false(settings->sealed);

	setting_t* setting_ptr = alloc_malloc(setting_ptr, 1);
	assert_ptr(setting_ptr, 1);

	*setting_ptr =
	(setting_t)
	{
		.type = SETTING_TYPE_BOOLEAN,
		.name = name,
		.value.boolean = value,
		.change_target = change_target
	};

	sync_mtx_init(&setting_ptr->mtx);

	sync_rwlock_rdlock(&settings->rwlock);
		bool status = hash_table_add(settings->table, name, setting_ptr);
		assert_true(status);
	sync_rwlock_unlock(&settings->rwlock);

	return setting_ptr;
}


setting_t*
settings_add_str(
	settings_t settings,
	const char* name,
	str_t value,
	uint64_t max_len,
	event_target_t* change_target
	)
{
	assert_not_null(settings);
	assert_not_null(name);
	assert_false(settings->sealed);

	assert_le(value->len, max_len);

	setting_t* setting_ptr = alloc_malloc(setting_ptr, 1);
	assert_ptr(setting_ptr, 1);

	*setting_ptr =
	(setting_t)
	{
		.type = SETTING_TYPE_STR,
		.name = name,
		.value.str = value,
		.constraint.str.max_len = max_len,
		.change_target = change_target
	};

	sync_mtx_init(&setting_ptr->mtx);

	sync_rwlock_rdlock(&settings->rwlock);
		bool status = hash_table_add(settings->table, name, setting_ptr);
		assert_true(status);
	sync_rwlock_unlock(&settings->rwlock);

	return setting_ptr;
}


setting_t*
settings_add_color(
	settings_t settings,
	const char* name,
	color_argb_t value,
	event_target_t* change_target
	)
{
	assert_not_null(settings);
	assert_not_null(name);
	assert_false(settings->sealed);

	setting_t* setting_ptr = alloc_malloc(setting_ptr, 1);
	assert_ptr(setting_ptr, 1);

	*setting_ptr =
	(setting_t)
	{
		.type = SETTING_TYPE_COLOR,
		.name = name,
		.value.color = value,
		.change_target = change_target
	};

	sync_mtx_init(&setting_ptr->mtx);

	sync_rwlock_rdlock(&settings->rwlock);
		bool status = hash_table_add(settings->table, name, setting_ptr);
		assert_true(status);
	sync_rwlock_unlock(&settings->rwlock);

	return setting_ptr;
}


void
settings_modify(
	settings_t settings
	)
{
	settings->dirty = true;

	if(settings->use_timers)
	{
		time_timers_lock(settings->timers);
			uint64_t time = time_get_with_sec(5);
			if(!time_timers_set_timeout_u(settings->timers, &settings->save_timer, time))
			{
				time_timeout_t timeout =
				{
					.timer = &settings->save_timer,
					.data =
					{
						.fn = (void*) settings_save_fn,
						.data = settings
					},
					.time = time
				};
				time_timers_add_timeout_u(settings->timers, timeout);
			}
		time_timers_unlock(settings->timers);
	}
}


void
settings_modify_i64(
	settings_t settings,
	setting_t* setting,
	int64_t value
	)
{
	assert_not_null(settings);
	assert_not_null(setting);

	value = MACRO_CLAMP(value, setting->constraint.i64.min, setting->constraint.i64.max);

	setting_value_t new_value =
	{
		.i64 = value
	};

	sync_rwlock_rdlock(&settings->rwlock);
		sync_mtx_lock(&setting->mtx);
			if(setting->change_target)
			{
				setting_change_event_data_t change_data =
				{
					.settings = settings,
					.old_value = setting->value,
					.new_value = new_value
				};
				event_target_fire(setting->change_target, &change_data);
			}

			setting->value = new_value;
		sync_mtx_unlock(&setting->mtx);
	sync_rwlock_unlock(&settings->rwlock);

	settings_modify(settings);
}


void
settings_modify_f32(
	settings_t settings,
	setting_t* setting,
	float value
	)
{
	assert_not_null(settings);
	assert_not_null(setting);

	value = MACRO_CLAMP(value, setting->constraint.f32.min, setting->constraint.f32.max);

	setting_value_t new_value =
	{
		.f32 = value
	};

	sync_rwlock_rdlock(&settings->rwlock);
		sync_mtx_lock(&setting->mtx);
			if(setting->change_target)
			{
				setting_change_event_data_t change_data =
				{
					.settings = settings,
					.old_value = setting->value,
					.new_value = new_value
				};
				event_target_fire(setting->change_target, &change_data);
			}

			setting->value = new_value;
		sync_mtx_unlock(&setting->mtx);
	sync_rwlock_unlock(&settings->rwlock);

	settings_modify(settings);
}


void
settings_modify_boolean(
	settings_t settings,
	setting_t* setting,
	bool value
	)
{
	assert_not_null(settings);
	assert_not_null(setting);

	setting_value_t new_value =
	{
		.boolean = value
	};

	sync_rwlock_rdlock(&settings->rwlock);
		sync_mtx_lock(&setting->mtx);
			if(setting->change_target)
			{
				setting_change_event_data_t change_data =
				{
					.settings = settings,
					.old_value = setting->value,
					.new_value = new_value
				};
				event_target_fire(setting->change_target, &change_data);
			}

			setting->value = new_value;
		sync_mtx_unlock(&setting->mtx);
	sync_rwlock_unlock(&settings->rwlock);

	settings_modify(settings);
}


void
settings_modify_str(
	settings_t settings,
	setting_t* setting,
	str_t value
	)
{
	assert_not_null(settings);
	assert_not_null(setting);

	if(value->len > setting->constraint.str.max_len)
	{
		str_resize(value, setting->constraint.str.max_len);
	}

	setting_value_t new_value =
	{
		.str = value
	};

	sync_rwlock_rdlock(&settings->rwlock);
		sync_mtx_lock(&setting->mtx);
			if(setting->change_target)
			{
				setting_change_event_data_t change_data =
				{
					.settings = settings,
					.old_value = setting->value,
					.new_value = new_value
				};
				event_target_fire(setting->change_target, &change_data);
			}

			setting->value = new_value;
		sync_mtx_unlock(&setting->mtx);
	sync_rwlock_unlock(&settings->rwlock);

	settings_modify(settings);
}


void
settings_modify_color(
	settings_t settings,
	setting_t* setting,
	color_argb_t value
	)
{
	assert_not_null(settings);
	assert_not_null(setting);

	setting_value_t new_value =
	{
		.color = value
	};

	sync_rwlock_rdlock(&settings->rwlock);
		sync_mtx_lock(&setting->mtx);
			if(setting->change_target)
			{
				setting_change_event_data_t change_data =
				{
					.settings = settings,
					.old_value = setting->value,
					.new_value = new_value
				};
				event_target_fire(setting->change_target, &change_data);
			}

			setting->value = new_value;
		sync_mtx_unlock(&setting->mtx);
	sync_rwlock_unlock(&settings->rwlock);

	settings_modify(settings);
}


int64_t
setting_get_i64(
	setting_t* setting
	)
{
	assert_not_null(setting);

	if(global_options)
	{
		int64_t options_value;
		if(options_get_i64(global_options, setting->name, setting->constraint.i64.min, setting->constraint.i64.max, &options_value))
		{
			return options_value;
		}
	}

	sync_mtx_lock(&setting->mtx);
		int64_t value = setting->value.i64;
	sync_mtx_unlock(&setting->mtx);

	return value;
}


float
setting_get_f32(
	setting_t* setting
	)
{
	assert_not_null(setting);

	if(global_options)
	{
		float options_value;
		if(options_get_f32(global_options, setting->name, setting->constraint.f32.min, setting->constraint.f32.max, &options_value))
		{
			return options_value;
		}
	}

	sync_mtx_lock(&setting->mtx);
		float value = setting->value.f32;
	sync_mtx_unlock(&setting->mtx);

	return value;
}


bool
setting_get_boolean(
	setting_t* setting
	)
{
	assert_not_null(setting);

	if(global_options)
	{
		bool options_value;
		if(options_get_boolean(global_options, setting->name, &options_value))
		{
			return options_value;
		}
	}

	sync_mtx_lock(&setting->mtx);
		bool value = setting->value.boolean;
	sync_mtx_unlock(&setting->mtx);

	return value;
}


str_t
setting_get_str(
	setting_t* setting
	)
{
	assert_not_null(setting);

	if(global_options)
	{
		str_t options_value;
		if(options_get_str(global_options, setting->name, &options_value) && (!options_value || options_value->len <= setting->constraint.str.max_len))
		{
			return options_value;
		}
	}

	sync_mtx_lock(&setting->mtx);
		str_t value = setting->value.str;
	sync_mtx_unlock(&setting->mtx);

	return value;
}


color_argb_t
setting_get_color(
	setting_t* setting
	)
{
	assert_not_null(setting);

	if(global_options)
	{
		color_argb_t options_value;
		if(options_get_color(global_options, setting->name, &options_value))
		{
			return options_value;
		}
	}

	sync_mtx_lock(&setting->mtx);
		color_argb_t value = setting->value.color;
	sync_mtx_unlock(&setting->mtx);

	return value;
}
