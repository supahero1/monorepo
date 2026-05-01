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
#include <shared/hash.h>
#include <shared/color.h>
#include <shared/debug.h>
#include <shared/macro.h>
#include <shared/options.h>
#include <shared/alloc/base.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


options_t global_options = NULL;


struct options
{
	hash_table_t table;
};


void
options_key_free_fn(
	str_t key
	)
{
	str_clear(key);
}


void
options_value_free_fn(
	str_t value
	)
{
	str_free(value);
}


options_t
options_init(
	int argc,
	const char* const* argv
	)
{
	options_t options = alloc_malloc(options, 1);
	assert_ptr(options, 1);

	options->table = hash_table_init(64, (void*) options_key_free_fn, (void*) options_value_free_fn);

	const char* const* arg = argv + 1;
	const char* const* arg_end = argv + argc;

	while(arg < arg_end)
	{
		const char* key = *(arg++);
		const char* value = strchrnul(key, '=');

		if(strncmp(key, "--", 2))
		{
			continue;
		}

		key += 2;
		uint32_t len = value - key;
		key = cstr_init_len(key, len);

		str_t value_str = NULL;
		if(*value == '=')
		{
			value_str = str_init_copy_cstr(++value);
		}

		hash_table_set(options->table, key, value_str);
	}

	return options;
}


void
options_free(
	options_t options
	)
{
	assert_not_null(options);

	hash_table_free(options->table);

	alloc_free(options, 1);
}


bool
options_get_i64(
	options_t options,
	const char* key,
	int64_t min_value,
	int64_t max_value,
	int64_t* out_value
	)
{
	assert_not_null(options);
	assert_not_null(key);
	assert_not_null(out_value);

	const str_t value = hash_table_get(options->table, key);
	if(value == NULL || str_is_empty(value))
	{
		return false;
	}

	char* endptr;
	int64_t result = strtoll(value->str, &endptr, 10);
	if(endptr == value->str || *endptr != '\0')
	{
		return false;
	}

	if(result < min_value || result > max_value)
	{
		return false;
	}

	*out_value = result;
	return true;
}


bool
options_get_f32(
	options_t options,
	const char* key,
	float min_value,
	float max_value,
	float* out_value
	)
{
	assert_not_null(options);
	assert_not_null(key);
	assert_not_null(out_value);

	const str_t value = hash_table_get(options->table, key);
	if(value == NULL || str_is_empty(value))
	{
		return false;
	}

	char* endptr;
	float result = strtof(value->str, &endptr);
	if(endptr == value->str || *endptr != '\0')
	{
		return false;
	}

	if(result < min_value || result > max_value)
	{
		return false;
	}

	*out_value = result;
	return true;
}


bool
options_get_boolean(
	options_t options,
	const char* key,
	bool* out_value
	)
{
	assert_not_null(options);
	assert_not_null(key);
	assert_not_null(out_value);

	const str_t value = hash_table_get(options->table, key);
	if(value == NULL || str_is_empty(value))
	{
		return false;
	}

	if(
		str_case_cmp_len(value, "true", 4) ||
		str_case_cmp_len(value, "yes", 3) ||
		str_case_cmp_len(value, "1", 1)
		)
	{
		*out_value = true;
		return true;
	}

	*out_value = false;
	return true;
}


bool
options_get_str(
	options_t options,
	const char* key,
	str_t* out_value
	)
{
	assert_not_null(options);
	assert_not_null(key);
	assert_not_null(out_value);

	if(!hash_table_has(options->table, key))
	{
		return false;
	}

	const str_t value = hash_table_get(options->table, key);
	*out_value = value;
	return true;
}


uint8_t
hex_char_to_value(
	char c
	)
{
	if(c >= '0' && c <= '9')
	{
		return c - '0';
	}

	if(c >= 'a' && c <= 'f')
	{
		return c - 'a' + 10;
	}

	if(c >= 'A' && c <= 'F')
	{
		return c - 'A' + 10;
	}

	return 0;
}


bool
options_get_color(
	options_t options,
	const char* key,
	color_argb_t* out_value
	)
{
	assert_not_null(options);
	assert_not_null(key);
	assert_not_null(out_value);

	const str_t value = hash_table_get(options->table, key);
	if(value == NULL || str_is_empty(value))
	{
		return false;
	}

	const char* hex = value->str;
	uint64_t len = value->len;

	if(hex[0] == '#')
	{
		hex++;
		len--;
	}

	color_argb_t color = {0};


	switch(len)
	{

	case 3:
	{
		uint8_t r = hex_char_to_value(hex[0]);
		uint8_t g = hex_char_to_value(hex[1]);
		uint8_t b = hex_char_to_value(hex[2]);

		color.r = (r << 4) | r;
		color.g = (g << 4) | g;
		color.b = (b << 4) | b;
		color.a = 255;
		break;
	}

	case 4:
	{
		uint8_t r = hex_char_to_value(hex[0]);
		uint8_t g = hex_char_to_value(hex[1]);
		uint8_t b = hex_char_to_value(hex[2]);
		uint8_t a = hex_char_to_value(hex[3]);

		color.r = (r << 4) | r;
		color.g = (g << 4) | g;
		color.b = (b << 4) | b;
		color.a = (a << 4) | a;
		break;
	}

	case 5:
	{
		uint8_t r = hex_char_to_value(hex[0]);
		uint8_t g = hex_char_to_value(hex[1]);
		uint8_t b = hex_char_to_value(hex[2]);
		uint8_t a1 = hex_char_to_value(hex[3]);
		uint8_t a2 = hex_char_to_value(hex[4]);

		color.r = (r << 4) | r;
		color.g = (g << 4) | g;
		color.b = (b << 4) | b;
		color.a = (a1 << 4) | a2;
		break;
	}

	case 6:
	{
		uint8_t r1 = hex_char_to_value(hex[0]);
		uint8_t r2 = hex_char_to_value(hex[1]);
		uint8_t g1 = hex_char_to_value(hex[2]);
		uint8_t g2 = hex_char_to_value(hex[3]);
		uint8_t b1 = hex_char_to_value(hex[4]);
		uint8_t b2 = hex_char_to_value(hex[5]);

		color.r = (r1 << 4) | r2;
		color.g = (g1 << 4) | g2;
		color.b = (b1 << 4) | b2;
		color.a = 255;
		break;
	}

	case 8:
	{
		uint8_t r1 = hex_char_to_value(hex[0]);
		uint8_t r2 = hex_char_to_value(hex[1]);
		uint8_t g1 = hex_char_to_value(hex[2]);
		uint8_t g2 = hex_char_to_value(hex[3]);
		uint8_t b1 = hex_char_to_value(hex[4]);
		uint8_t b2 = hex_char_to_value(hex[5]);
		uint8_t a1 = hex_char_to_value(hex[6]);
		uint8_t a2 = hex_char_to_value(hex[7]);

		color.r = (r1 << 4) | r2;
		color.g = (g1 << 4) | g2;
		color.b = (b1 << 4) | b2;
		color.a = (a1 << 4) | a2;
		break;
	}

	default:
	{
		return false;
	}

	}


	*out_value = color;
	return true;
}


bool
options_exists(
	options_t options,
	const char* key
	)
{
	assert_not_null(options);
	assert_not_null(key);

	return hash_table_has(options->table, key);
}
