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


typedef enum dds_flag
{
	DDS_FLAG_CAPS = 0x1,
	DDS_FLAG_HEIGHT = 0x2,
	DDS_FLAG_WIDTH = 0x4,
	DDS_FLAG_PITCH = 0x8,
	DDS_FLAG_PIXEL_FORMAT = 0x1000,
	DDS_FLAG_MIPMAP_COUNT = 0x20000,
	DDS_FLAG_LINEARSIZE = 0x80000,
	DDS_FLAG_DEPTH = 0x800000,
	DDS_FLAG__COUNT,

	DDS_REQUIRED_FLAGS = DDS_FLAG_CAPS | DDS_FLAG_HEIGHT |
		DDS_FLAG_WIDTH | DDS_FLAG_PIXEL_FORMAT | DDS_FLAG_LINEARSIZE
}
dds_flag_t;


typedef enum dds_pixel_flag
{
	DDS_PIXEL_FLAG_ALPHAPIXELS = 0x1,
	DDS_PIXEL_FLAG_ALPHA = 0x2,
	DDS_PIXEL_FLAG_FOURCC = 0x4,
	DDS_PIXEL_FLAG_RGB = 0x40,
	DDS_PIXEL_FLAG_YUV = 0x200,
	DDS_PIXEL_FLAG_LUMINANCE = 0x20000,
	DDS_PIXEL_FLAG__COUNT,

	DDS_REQUIRED_PIXEL_FLAGS = DDS_PIXEL_FLAG_FOURCC
}
dds_pixel_flag_t;


typedef struct dds_pixel_format
{
	uint32_t size;
	uint32_t flags;
	uint32_t format_code;
	uint32_t rgb_bit_count;
	uint32_t r_bit_mask;
	uint32_t g_bit_mask;
	uint32_t b_bit_mask;
	uint32_t a_bit_mask;
}
dds_pixel_format_t;


typedef struct dds_tex
{
	uint32_t magic;
	uint32_t size;
	dds_flag_t flags;
	uint32_t height;
	uint32_t width;
	uint32_t pitch_or_linear_size;
	uint32_t depth;
	uint32_t mipmap_count;
	uint32_t reserved_1[11];
	dds_pixel_format_t pixel_format;
	uint32_t caps;
	uint32_t caps_2;
	uint32_t caps_3;
	uint32_t caps_4;
	uint32_t reserved_2;
	uint32_t format;
	uint32_t dimension;
	uint32_t misc_flag;
	uint32_t array_size;
	uint32_t misc_flags_2;
	uint8_t data[];
}
dds_tex_t;


extern dds_tex_t*
dds_load(
	const char* path
	);


extern void
dds_free(
	dds_tex_t* tex
	);


extern uint64_t
dds_data_size(
	dds_tex_t* tex
	);


extern uint64_t
dds_offset(
	dds_tex_t* tex,
	uint32_t layer
	);
