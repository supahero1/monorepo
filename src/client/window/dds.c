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

#include <shared/file.h>
#include <shared/debug.h>
#include <shared/macro.h>
#include <client/window/dds.h>
#include <shared/alloc/base.h>

#include <zstd.h>
#include <stdint.h>


extern dds_tex_t*
dds_load(
	const char* path
	)
{
	file_t file;
	bool status = file_read_cap(path, &file, /* 1GiB*/ 0x40000000);
	hard_assert_true(status);

	uint64_t decompressed_size = ZSTD_getFrameContentSize(file.data, file.len);
	hard_assert_lt(decompressed_size, /* 1GiB*/ 0x40000000);

	uint8_t* content = alloc_malloc(content, decompressed_size);
	hard_assert_not_null(content);

	uint64_t actual_decompressed_size = ZSTD_decompress(content, decompressed_size, file.data, file.len);
	hard_assert_false(ZSTD_isError(actual_decompressed_size));
	hard_assert_ge(actual_decompressed_size, sizeof(dds_tex_t));
	hard_assert_le(actual_decompressed_size, decompressed_size);

	file_free(file);

	dds_tex_t* tex = (dds_tex_t*) content;
	hard_assert_eq(actual_decompressed_size, sizeof(dds_tex_t) + dds_data_size(tex));
	hard_assert_eq(tex->magic, 0x20534444);
	hard_assert_eq(tex->size, 124);
	hard_assert_eq((tex->flags & DDS_REQUIRED_FLAGS), DDS_REQUIRED_FLAGS);
	hard_assert_eq((tex->pixel_format.flags & DDS_REQUIRED_PIXEL_FLAGS), DDS_REQUIRED_PIXEL_FLAGS);
	hard_assert_eq(tex->pixel_format.format_code, 0x30315844);
	hard_assert_eq(tex->dimension, 3);
	hard_assert_eq(tex->format, 72);
	hard_assert_neq(tex->array_size, 0);

	return tex;
}


extern void
dds_free(
	dds_tex_t* tex
	)
{
	alloc_free((void*) tex, sizeof(dds_tex_t) + dds_data_size(tex));
}


uint64_t
dds_data_size(
	dds_tex_t* tex
	)
{
	return dds_offset(tex, tex->array_size);
}


uint64_t
dds_offset(
	dds_tex_t* tex,
	uint32_t layer
	)
{
	return tex->pitch_or_linear_size * layer;
}
