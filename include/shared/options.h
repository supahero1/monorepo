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

#include <shared/str.h>
#include <shared/color.h>

#include <stdint.h>


typedef struct options* options_t;


extern options_t global_options;


extern options_t
options_init(
	int argc,
	const char* const* argv
	);


extern void
options_free(
	options_t options
	);


extern bool
options_get_i64(
	options_t options,
	const char* key,
	int64_t min_value,
	int64_t max_value,
	int64_t* out_value
	);


extern bool
options_get_f32(
	options_t options,
	const char* key,
	float min_value,
	float max_value,
	float* out_value
	);


extern bool
options_get_boolean(
	options_t options,
	const char* key,
	bool* out_value
	);


extern bool
options_get_str(
	options_t options,
	const char* key,
	str_t* out_value
	);


extern bool
options_get_color(
	options_t options,
	const char* key,
	color_argb_t* out_value
	);


extern bool
options_exists(
	options_t options,
	const char* key
	);
