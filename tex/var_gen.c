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
#include <shared/color.h>
#include <shared/debug.h>
#include <shared/macro.h>
#include <shared/alloc/base.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <math.h>
#include <stdint.h>
#include <string.h>


void
var_bg_tile(
	void
	)
{
	uint32_t size = 128;
	uint32_t img_size = size * size * 3;
	uint8_t* img = alloc_malloc(img, img_size);
	assert_ptr(img, img_size);

	uint8_t* cur = img;

	for(uint32_t y = 0; y < size; ++y)
	{
		for(uint32_t x = 0; x < size; ++x)
		{
			if((x >= 63 && x <= 64) || (y >= 63 && y <= 64))
			{
				*(cur++) = 0;
				*(cur++) = 0;
				*(cur++) = 0;
			}
			else
			{
				*(cur++) = 255;
				*(cur++) = 255;
				*(cur++) = 255;
			}
		}
	}

	int status = stbi_write_png("tex/var/bg_tile.png", size, size, 3, img, size * 3);
	assert_neq(status, 0);

	alloc_free(img, img_size);
}


void
var_rect(
	void
	)
{
	uint32_t size = 4;
	uint32_t img_size = size * size * 3;
	uint8_t* img = alloc_malloc(img, img_size);
	assert_ptr(img, img_size);

	memset(img, 0xFF, img_size);

	int status = stbi_write_png("tex/var/rect.png", size, size, 3, img, size * 3);
	assert_neq(status, 0);

	alloc_free(img, img_size);
}


void
var_text_cursor(
	void
	)
{
	uint32_t size = 256;
	uint32_t img_size = size * size * 3;
	uint8_t* img = alloc_malloc(img, img_size);
	assert_ptr(img, img_size);

	uint32_t ratio = 6;
	uint32_t y_thickness = 12;
	uint32_t x_thickness = y_thickness * ratio;

	uint8_t* cur = img;

	for(uint32_t y = 0; y < size; ++y)
	{
		for(uint32_t x = 0; x < size; ++x)
		{
			if(y < y_thickness || y >= size - y_thickness || x < x_thickness || x >= size - x_thickness)
			{
				*(cur++) = 0;
				*(cur++) = 0;
				*(cur++) = 0;
			}
			else
			{
				*(cur++) = 255;
				*(cur++) = 255;
				*(cur++) = 255;
			}
		}
	}

	int status = stbi_write_png("tex/var/text_cursor.png", size, size, 3, img, size * 3);
	assert_neq(status, 0);

	alloc_free(img, img_size);
}


void
var_cs_b(
	void
	)
{
	uint32_t size = 256;
	uint32_t img_size = size * size * 3;
	uint8_t* img = alloc_malloc(img, img_size);
	assert_ptr(img, img_size);

	uint8_t* cur = img;

	for(uint32_t y = 0; y < size; ++y)
	{
		for(uint32_t x = 0; x < size; ++x)
		{
			*(cur++) = x;
			*(cur++) = x;
			*(cur++) = x;
		}
	}

	int status = stbi_write_png("tex/var/cs_b.png", size, size, 3, img, size * 3);
	assert_neq(status, 0);

	alloc_free(img, img_size);
}


void
var_cs_red(
	void
	)
{
	uint32_t size = 256;
	uint32_t img_size = size * size * 3;
	uint8_t* img = alloc_malloc(img, img_size);
	assert_ptr(img, img_size);

	uint8_t* cur = img;

	for(uint32_t y = 0; y < size; ++y)
	{
		for(uint32_t x = 0; x < size; ++x)
		{
			*(cur++) = x;
			*(cur++) = 255;
			*(cur++) = 255;
		}
	}

	int status = stbi_write_png("tex/var/cs_red.png", size, size, 3, img, size * 3);
	assert_neq(status, 0);

	alloc_free(img, img_size);
}


void
var_cs_green(
	void
	)
{
	uint32_t size = 256;
	uint32_t img_size = size * size * 3;
	uint8_t* img = alloc_malloc(img, img_size);
	assert_ptr(img, img_size);

	uint8_t* cur = img;

	for(uint32_t y = 0; y < size; ++y)
	{
		for(uint32_t x = 0; x < size; ++x)
		{
			*(cur++) = 255;
			*(cur++) = x;
			*(cur++) = 255;
		}
	}

	int status = stbi_write_png("tex/var/cs_green.png", size, size, 3, img, size * 3);
	assert_neq(status, 0);

	alloc_free(img, img_size);
}


void
var_cs_blue(
	void
	)
{
	uint32_t size = 256;
	uint32_t img_size = size * size * 3;
	uint8_t* img = alloc_malloc(img, img_size);
	assert_ptr(img, img_size);

	uint8_t* cur = img;

	for(uint32_t y = 0; y < size; ++y)
	{
		for(uint32_t x = 0; x < size; ++x)
		{
			*(cur++) = 255;
			*(cur++) = 255;
			*(cur++) = x;
		}
	}

	int status = stbi_write_png("tex/var/cs_blue.png", size, size, 3, img, size * 3);
	assert_neq(status, 0);

	alloc_free(img, img_size);
}


void
var_rect8_t(
	void
	)
{
	uint32_t size = 8;
	uint32_t img_size = size * size * 3;
	uint8_t* img = alloc_malloc(img, img_size);
	assert_ptr(img, img_size);

	uint8_t* cur = img;

	for(uint32_t y = 0; y < size; ++y)
	{
		uint32_t y_xor = y & 1;

		for(uint32_t x = 0; x < size; ++x)
		{
			uint32_t x_xor = x & 1;

			if((x_xor ^ y_xor) == 0)
			{
				*(cur++) = 170;
				*(cur++) = 170;
				*(cur++) = 170;
			}
			else
			{
				*(cur++) = 113;
				*(cur++) = 113;
				*(cur++) = 113;
			}
		}
	}

	int status = stbi_write_png("tex/var/rect8_t.png", size, size, 3, img, size * 3);
	assert_neq(status, 0);

	alloc_free(img, img_size);
}


void
var_rect128_t(
	void
	)
{
	uint32_t size = 128;
	uint32_t img_size = size * size * 3;
	uint8_t* img = alloc_malloc(img, img_size);
	assert_ptr(img, img_size);

	uint8_t* cur = img;

	for(uint32_t y = 0; y < size; ++y)
	{
		uint32_t y_xor = y & 1;

		for(uint32_t x = 0; x < size; ++x)
		{
			uint32_t x_xor = x & 1;

			if((x_xor ^ y_xor) == 0)
			{
				*(cur++) = 170;
				*(cur++) = 170;
				*(cur++) = 170;
			}
			else
			{
				*(cur++) = 113;
				*(cur++) = 113;
				*(cur++) = 113;
			}
		}
	}

	int status = stbi_write_png("tex/var/rect128_t.png", size, size, 3, img, size * 3);
	assert_neq(status, 0);

	alloc_free(img, img_size);
}


void
var_cs_t(
	void
	)
{
	uint32_t size = 256;
	uint32_t img_size = size * size * 3;
	uint8_t* img = alloc_malloc(img, img_size);
	assert_ptr(img, img_size);

	uint8_t* cur = img;

	for(uint32_t y = 0; y < size; ++y)
	{
		uint32_t y_xor = (y >> 6) & 1;

		for(uint32_t x = 0; x < size; ++x)
		{
			uint32_t x_xor = (x >> 2) & 1;

			if((x_xor ^ y_xor) == 0)
			{
				*(cur++) = 170;
				*(cur++) = 170;
				*(cur++) = 170;
			}
			else
			{
				*(cur++) = 113;
				*(cur++) = 113;
				*(cur++) = 113;
			}
		}
	}

	int status = stbi_write_png("tex/var/cs_t.png", size, size, 3, img, size * 3);
	assert_neq(status, 0);

	alloc_free(img, img_size);
}


void
var_circle_t(
	void
	)
{
	int width, height, channels;
	uint8_t* base_img = stbi_load("tex/const/circle.png", &width, &height, &channels, 4);
	assert_not_null(base_img);
	assert_eq(width, 256);
	assert_eq(height, 256);
	assert_eq(channels, 4);

	uint32_t size = 256;
	uint32_t img_size = size * size * 4;
	uint8_t* img = alloc_malloc(img, img_size);
	assert_ptr(img, img_size);

	uint8_t* cur = img;

	for(uint32_t y = 0; y < size; ++y)
	{
		uint32_t y_xor = (y >> 6) & 1;

		for(uint32_t x = 0; x < size; ++x)
		{
			uint32_t x_xor = (x >> 6) & 1;

			uint32_t idx = (y * size + x) * 4;
			if(base_img[idx + 3] == 0)
			{
				*(cur++) = 255;
				*(cur++) = 255;
				*(cur++) = 255;
				*(cur++) = 0;
			}
			else if((x_xor ^ y_xor) == 0)
			{
				*(cur++) = 170;
				*(cur++) = 170;
				*(cur++) = 170;
				*(cur++) = 255;
			}
			else
			{
				*(cur++) = 113;
				*(cur++) = 113;
				*(cur++) = 113;
				*(cur++) = 255;
			}
		}
	}

	int status = stbi_write_png("tex/var/circle_t.png", size, size, 4, img, size * 4);
	assert_neq(status, 0);

	alloc_free(img, img_size);
	stbi_image_free(base_img);
}


void
var_t_mask(
	void
	)
{
	uint32_t size = 256;
	uint32_t img_size = size * size * 4;
	uint8_t* img = alloc_malloc(img, img_size);
	assert_ptr(img, img_size);

	uint8_t* cur = img;

	for(uint32_t y = 0; y < size; ++y)
	{
		for(uint32_t x = 0; x < size; ++x)
		{
			*(cur++) = 255;
			*(cur++) = 255;
			*(cur++) = 255;
			*(cur++) = x;
		}
	}

	int status = stbi_write_png("tex/var/t_mask.png", size, size, 4, img, size * 4);
	assert_neq(status, 0);

	alloc_free(img, img_size);
}


void
var_cs_hs(
	void
	)
{
	uint32_t size = 2048;
	uint32_t img_size = size * size * 4;
	uint8_t* img = alloc_malloc(img, img_size);
	assert_ptr(img, img_size);

	uint8_t* cur = img;
	float center = (size - 1) * 0.5f;
	float center_ceil = ceilf(center);

	for(uint32_t y = 0; y < size; ++y)
	{
		float y_dist = y - center;

		for(uint32_t x = 0; x < size; ++x)
		{
			float x_dist = x - center;
			float dist = sqrtf(x_dist * x_dist + y_dist * y_dist);

			if(dist > center_ceil)
			{
				*(cur++) = 0;
				*(cur++) = 0;
				*(cur++) = 0;
				*(cur++) = 0;
				continue;
			}

			float hue = (atan2f(y_dist, x_dist) + M_PI) / (2.0f * M_PI);
			float sat = dist / center;

			color_hsv_t hsv = { hue, sat, 1.0f };
			color_argb_t argb = color_hsv_to_argb(hsv);

			*(cur++) = argb.r;
			*(cur++) = argb.g;
			*(cur++) = argb.b;
			*(cur++) = argb.a;
		}
	}

	int status = stbi_write_png("tex/var/cs_hs.png", size, size, 4, img, size * 4);
	assert_neq(status, 0);

	alloc_free(img, img_size);
}


int
main(
	void
	)
{
	var_bg_tile();
	var_rect();
	var_text_cursor();
	var_cs_b();
	var_cs_red();
	var_cs_green();
	var_cs_blue();
	var_rect8_t();
	var_rect128_t();
	var_cs_t();
	var_circle_t();
	var_t_mask();
	var_cs_hs();

	return 0;
}
