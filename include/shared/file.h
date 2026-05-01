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

#include <stdint.h>


typedef struct file
{
	uint64_t len;
	uint8_t* data;
}
file_t;


extern bool
file_exists(
	const char* path
	);


extern bool
file_write(
	const char* path,
	file_t file
	);


extern bool
file_read_cap(
	const char* path,
	file_t* file,
	uint64_t cap
	);


extern bool
file_read(
	const char* path,
	file_t* file
	);


extern bool
file_remove(
	const char* path
	);


extern void
file_free(
	file_t file
	);


extern bool
dir_exists(
	const char* path
	);


extern bool
dir_create(
	const char* path
	);


extern bool
dir_remove(
	const char* path
	);
