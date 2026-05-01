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
#include <shared/alloc/base.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/ftstroke.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#define FONT_AVG_SIZE 400
#define FONT_STROKE_THICKNESS 31
#define TEXT_FILE_SIZE ((uint32_t) 1 << 24)


typedef struct font_glyph_data
{
	int16_t top;
	int16_t left;
	uint32_t codepoint;
}
font_glyph_data_t;


int
main(
	void
	)
{
	bool write_img = getenv("WRITE_IMG") != NULL;
	bool fill_blank = getenv("FILL_BLANK") != NULL;

	FT_Library free_type;
	int status = FT_Init_FreeType(&free_type);
	assert_eq(status, 0);

	FT_Face face;
	status = FT_New_Face(free_type, "tex/Ubuntu.ttf", 0, &face);
	assert_eq(status, 0);

	FT_Set_Pixel_Sizes(face, 0, FONT_AVG_SIZE);

	uint32_t max_width = 0;
	uint32_t max_height = 0;
	uint32_t max_idx = 0;
	uint32_t max_codepoint = 0;

	uint32_t table_size = (uint32_t) 1 << 24;

	font_glyph_data_t* glyph_data = alloc_malloc(glyph_data, table_size);
	assert_ptr(glyph_data, table_size);

	font_glyph_data_t* cur_glyph_data = glyph_data + 1;

	uint32_t* glyph_map = alloc_calloc(glyph_map, table_size);
	assert_ptr(glyph_map, table_size);

	uint32_t* codepoint_map = alloc_calloc(codepoint_map, table_size);
	assert_ptr(codepoint_map, table_size);

	codepoint_map['\t'] = ' ';
	codepoint_map['\n'] = '\n';

	uint32_t glyph_idx;
	uint32_t codepoint = FT_Get_First_Char(face, &glyph_idx);

	while(glyph_idx)
	{
		printf("codepoint %u glyph_idx %u\n", codepoint, glyph_idx);

		if(codepoint < ' ')
		{
			goto goto_skip;
		}

		codepoint_map[codepoint] = codepoint;
		if(codepoint > max_codepoint)
		{
			max_codepoint = codepoint;
		}

		status = FT_Load_Char(face, codepoint, FT_LOAD_TARGET_MONO);
		assert_eq(status, 0);

		FT_Stroker stroker;
		status = FT_Stroker_New(free_type, &stroker);
		assert_eq(status, 0);

		FT_Stroker_Set(stroker, FONT_STROKE_THICKNESS * 64,
			FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_MITER_VARIABLE, 0);

		FT_Glyph glyph;
		status = FT_Get_Glyph(face->glyph, &glyph);
		assert_eq(status, 0);

		status = FT_Glyph_StrokeBorder(&glyph, stroker, 0, 1);
		assert_eq(status, 0);

		status = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_MONO, 0, 1);
		assert_eq(status, 0);

		FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph) glyph;

		uint32_t bg_width = bitmap_glyph->bitmap.width;
		if(bg_width > max_width)
		{
			printf("new max_width %u (bg)\n", bg_width);
			max_width = bg_width;
		}

		uint32_t bg_height = bitmap_glyph->bitmap.rows;
		if(bg_height > max_height)
		{
			printf("new max_height %u (bg)\n", bg_height);
			max_height = bg_height;
		}

		uint32_t bg_top = bitmap_glyph->top;
		uint32_t bg_left = bitmap_glyph->left;
		uint32_t bg_pitch = bitmap_glyph->bitmap.pitch << 3;

		uint32_t bg_size = MACRO_NEXT_OR_EQUAL_POWER_OF_2(MACRO_MAX(bg_width, bg_height));
		if(bg_size <= 4)
		{
			bg_size = 4;
		}

		glyph_map[glyph_idx] = cur_glyph_data - glyph_data;
		if(glyph_idx > max_idx)
		{
			max_idx = glyph_idx;
		}

		uint32_t img_size = bg_size * bg_size * 4;
		uint8_t* bg_img = alloc_calloc(bg_img, img_size);
		assert_ptr(bg_img, img_size);

		for(uint32_t y = 0; y < bg_height; ++y)
		{
			for(uint32_t x = 0; x < bg_width; ++x)
			{
				uint32_t old_idx = y * bg_pitch + x;
				uint32_t new_idx = y * bg_size + x;
				uint32_t byte = old_idx >> 3;
				uint32_t bit = 7 - (old_idx & 7);

				uint32_t pixel = (bitmap_glyph->bitmap.buffer[byte] >> bit) & 1;
				if(pixel)
				{
					bg_img[4 * new_idx + 3] = 255;
				}
			}
		}

		if(fill_blank)
		{
			uint32_t max_y = bg_height;
			uint32_t max_x = bg_width;
			uint32_t pixels = max_x * max_y;

			for(uint32_t y = 0; y < max_y; ++y)
			{
				for(uint32_t x = 0; x < max_x; ++x)
				{
					uint8_t* pixel = bg_img + 4 * (y * bg_size + x) + 3;

					if(*pixel)
					{
						continue;
					}

					typedef struct img_pos
					{
						uint16_t y;
						uint16_t x;
					}
					img_pos;

					img_pos* stack = alloc_malloc(stack, pixels);
					assert_ptr(stack, pixels);

					img_pos* head = stack;

					uint8_t* visited = alloc_calloc(visited, pixels);
					assert_ptr(visited, pixels);

					*(head++) = (img_pos){ y, x };
					visited[y * max_x + x] = 1;

					bool enclosed = true;

					do
					{
						--head;

						uint16_t cur_y = head->y;
						uint16_t cur_x = head->x;

						if(cur_x)
						{
							uint8_t* visit = visited + cur_y * max_x + (cur_x - 1);

							if(!*visit && !bg_img[4 * (cur_y * bg_size + (cur_x - 1)) + 3])
							{
								*visit = 1;
								*(head++) = (img_pos){ cur_y, cur_x - 1 };
							}
						}
						else
						{
							enclosed = false;
							break;
						}

						if(cur_x != max_x - 1)
						{
							uint8_t* visit = visited + cur_y * max_x + (cur_x + 1);

							if(!*visit && !bg_img[4 * (cur_y * bg_size + (cur_x + 1)) + 3])
							{
								*visit = 1;
								*(head++) = (img_pos){ cur_y, cur_x + 1 };
							}
						}
						else
						{
							enclosed = false;
							break;
						}

						if(cur_y)
						{
							uint8_t* visit = visited + (cur_y - 1) * max_x + cur_x;

							if(!*visit && !bg_img[4 * ((cur_y - 1) * bg_size + cur_x) + 3])
							{
								*visit = 1;
								*(head++) = (img_pos){ cur_y - 1, cur_x };
							}
						}
						else
						{
							enclosed = false;
							break;
						}

						if(cur_y != max_y - 1)
						{
							uint8_t* visit = visited + (cur_y + 1) * max_x + cur_x;

							if(!*visit && !bg_img[4 * ((cur_y + 1) * bg_size + cur_x) + 3])
							{
								*visit = 1;
								*(head++) = (img_pos){ cur_y + 1, cur_x };
							}
						}
						else
						{
							enclosed = false;
							break;
						}
					}
					while(head != stack);

					alloc_free(visited, pixels);
					alloc_free(stack, pixels);

					if(enclosed)
					{
						*pixel = 255;
					}
				}
			}
		}

		FT_Stroker_Done(stroker);
		FT_Done_Glyph(glyph);

		status = FT_Get_Glyph(face->glyph, &glyph);
		assert_eq(status, 0);

		status = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_MONO, 0, 1);
		assert_eq(status, 0);

		bitmap_glyph = (FT_BitmapGlyph) glyph;

		uint32_t width = bitmap_glyph->bitmap.width;
		if(width > max_width)
		{
			printf("new max_width %u\n", width);
			max_width = width;
		}

		uint32_t height = bitmap_glyph->bitmap.rows;
		if(height > max_height)
		{
			printf("new max_height %u\n", height);
			max_height = height;
		}

		uint32_t left = bitmap_glyph->left;
		uint32_t top = bitmap_glyph->top;
		uint32_t off_x = left - bg_left;
		uint32_t off_y = bg_top - top;
		uint32_t pitch = bitmap_glyph->bitmap.pitch << 3;

		for(uint32_t y = 0; y < height; ++y)
		{
			for(uint32_t x = 0; x < width; ++x)
			{
				uint32_t old_idx = y * pitch + x;
				uint32_t new_bg_idx = (y + off_y) * bg_size + (x + off_x);
				uint32_t byte = old_idx >> 3;
				uint32_t bit = 7 - (old_idx & 7);

				uint32_t pixel = (bitmap_glyph->bitmap.buffer[byte] >> bit) & 1;
				if(pixel)
				{
					bg_img[4 * new_bg_idx + 0] = 255;
					bg_img[4 * new_bg_idx + 1] = 255;
					bg_img[4 * new_bg_idx + 2] = 255;
					bg_img[4 * new_bg_idx + 3] = 255;
				}
			}
		}

		cur_glyph_data->top = face->glyph->metrics.horiBearingY / 64.0f;
		cur_glyph_data->left = face->glyph->metrics.horiBearingX / 64.0f;
		cur_glyph_data->codepoint = codepoint;
		++cur_glyph_data;

		if(write_img)
		{
			char path[32];
			sprintf(path, "tex/font/%u.png", codepoint);

			status = stbi_write_png(path, bg_size, bg_size, 4, bg_img, bg_size * 4);
			assert_neq(status, 0);
		}

		alloc_free(bg_img, img_size);


		FT_Done_Glyph(glyph);
		goto_skip:
		codepoint = FT_Get_Next_Char(face, codepoint, &glyph_idx);
	}

	printf("max_width %d max_height %d max_idx %u\n", max_width, max_height, max_idx);
	++max_idx;

	FT_Done_Face(face);
	FT_Done_FreeType(free_type);


	char* data = alloc_malloc(data, TEXT_FILE_SIZE);
	assert_not_null(data);

	uint32_t fonts = cur_glyph_data - glyph_data;
	int font_size = (int)(max_height * 1.1f);

	char* str = data;
	str += sprintf(str, "/* This file was generated by font_gen.c */\n\n");
	str += sprintf(str, "#pragma once\n\n");
	str += sprintf(str, "#include <client/tex/base.h>\n\n");
	str += sprintf(str, "#define FONT_AVG_SIZE %d\n", FONT_AVG_SIZE);
	str += sprintf(str, "#define FONT_SIZE %d\n", font_size);
	str += sprintf(str, "#define FONT_ASCENDER %.04ff\n", font_size * 0.3f - FONT_STROKE_THICKNESS * 1.6666f);
	str += sprintf(str, "#define FONT_STROKE_THICKNESS %d\n\n\n", FONT_STROKE_THICKNESS);
	str += sprintf(str, "typedef struct font_glyph_data\n{\n");
	str += sprintf(str, "\tfloat top;\n");
	str += sprintf(str, "\tfloat left;\n");
	str += sprintf(str, "\ttex_t tex;\n");
	str += sprintf(str, "}\nfont_glyph_data_t;\n\n\n");
	str += sprintf(str, "extern const font_glyph_data_t font_glyph_data[%d];\n\n", fonts);
	str += sprintf(str, "extern const uint32_t font_glyph_map[%d];\n", max_idx);

	file_t base_h =
	{
		.data = (void*) data,
		.len = str - data
	};
	file_write("include/client/font/base.h", base_h);


	str = data;
	str += sprintf(str, "/* This file was generated by font_gen.c */\n\n");
	str += sprintf(str, "#include <client/font/base.h>\n\n\n");
	str += sprintf(str, "#define __(x) TEX_##x\n");
	str += sprintf(str, "#define _(x, y, z) { x, y, __(z) },\n\n");
	str += sprintf(str, "const font_glyph_data_t font_glyph_data[%d] =\n{\n", fonts);
	str += sprintf(str, "/*   0*/ {0},\n");

	uint32_t i = 1;
	uint32_t full_loops = (fonts - i) / 7;
	for(; i < full_loops * 7; i += 7)
	{
		str += sprintf(str,
			"/*%4u*/_(%3.0f,%3.0f,%5u)_(%3.0f,%3.0f,%5u)_(%3.0f,%3.0f,%5u)"
			"_(%3.0f,%3.0f,%5u)_(%3.0f,%3.0f,%5u)_(%3.0f,%3.0f,%5u)_(%3.0f,%3.0f,%5u)\n",
			i, (float) glyph_data[i].top, (float) glyph_data[i].left, glyph_data[i].codepoint,
			(float) glyph_data[i + 1].top, (float) glyph_data[i + 1].left, glyph_data[i + 1].codepoint,
			(float) glyph_data[i + 2].top, (float) glyph_data[i + 2].left, glyph_data[i + 2].codepoint,
			(float) glyph_data[i + 3].top, (float) glyph_data[i + 3].left, glyph_data[i + 3].codepoint,
			(float) glyph_data[i + 4].top, (float) glyph_data[i + 4].left, glyph_data[i + 4].codepoint,
			(float) glyph_data[i + 5].top, (float) glyph_data[i + 5].left, glyph_data[i + 5].codepoint,
			(float) glyph_data[i + 6].top, (float) glyph_data[i + 6].left, glyph_data[i + 6].codepoint);
	}

	if(i != fonts)
	{
		str += sprintf(str, "/*%4u*/", i);
		for(; i < fonts; ++i)
		{
			str += sprintf(str, "_(%3.0f,%3.0f,%5u)",
				(float) glyph_data[i].top, (float) glyph_data[i].left, glyph_data[i].codepoint);
		}
		str += sprintf(str, "\n");
	}

	alloc_free(glyph_data, table_size);

	str += sprintf(str, "};\n\n");
	str += sprintf(str, "#undef _\n");
	str += sprintf(str, "#undef __\n\n\n");
	str += sprintf(str, "const uint32_t font_glyph_map[%d] =\n{\n", max_idx);

	i = 0;
	full_loops = (max_idx - i) / 22;
	for(; i < full_loops * 22; i += 22)
	{
		str += sprintf(str,
			"/*%4u*/ %4u,%4u,%4u,%4u,%4u,%4u,%4u,%4u,%4u,%4u,%4u,%4u,%4u,%4u,%4u,%4u,%4u,%4u,%4u,%4u,%4u,%4u,\n",
			i, glyph_map[i], glyph_map[i + 1], glyph_map[i + 2], glyph_map[i + 3], glyph_map[i + 4],
			glyph_map[i + 5], glyph_map[i + 6], glyph_map[i + 7], glyph_map[i + 8], glyph_map[i + 9],
			glyph_map[i + 10], glyph_map[i + 11], glyph_map[i + 12], glyph_map[i + 13], glyph_map[i + 14],
			glyph_map[i + 15], glyph_map[i + 16], glyph_map[i + 17], glyph_map[i + 18], glyph_map[i + 19],
			glyph_map[i + 20], glyph_map[i + 21]);
	}

	if(i != max_idx)
	{
		str += sprintf(str, "/*%4u*/ ", i);
		for(; i < max_idx; ++i)
		{
			str += sprintf(str, "%4u,", glyph_map[i]);
		}
		*(str - 1) = '\n';
	}

	alloc_free(glyph_map, table_size);

	str += sprintf(str, "};\n");

	file_t base_c =
	{
		.data = (void*) data,
		.len = str - data
	};
	file_write("src/client/font/base.c", base_c);


	str = data;
	str += sprintf(str, "/* This file was generated by font_gen.c */\n\n");
	str += sprintf(str, "#pragma once\n\n");
	str += sprintf(str, "#include <stdint.h>\n\n\n");
	str += sprintf(str, "extern uint32_t\n");
	str += sprintf(str, "filter_codepoint(\n");
	str += sprintf(str, "\tuint32_t codepoint\n");
	str += sprintf(str, "\t);\n");

	file_t filter_h =
	{
		.data = (void*) data,
		.len = str - data
	};
	file_write("include/client/font/filter.h", filter_h);


	str = data;
	str += sprintf(str, "/* This file was generated by font_gen.c */\n\n");
	str += sprintf(str, "#include <client/font/filter.h>\n\n\n");
	str += sprintf(str, "#define _(x) case x:\n\n");
	str += sprintf(str, "uint32_t\nfilter_codepoint(\n\tuint32_t codepoint\n\t)\n{\n");
	str += sprintf(str, "\tswitch(codepoint)\n\t{\n\n\tcase '\\t': return ' '; case '\\n':\n");

	uint32_t printed = 0;
	uint32_t dedupe = 1;
	for(i = 32; i <= max_codepoint; ++i)
	{
		if(!(printed % 15) && !dedupe)
		{
			str += sprintf(str, "\n");
			dedupe = 1;
		}

		if(codepoint_map[i])
		{
			str += sprintf(str, "_(%5u)", i);
			++printed;
			dedupe = 0;
		}
	}

	str += sprintf(str, "\n\treturn codepoint;\n\n\tdefault: return 0;\n\n\t}\n}\n");
	str += sprintf(str, "\n#undef _\n");

	alloc_free(codepoint_map, table_size);

	file_t filter_c =
	{
		.data = (void*) data,
		.len = str - data
	};
	file_write("src/client/font/filter.c", filter_c);

	alloc_free(data, TEXT_FILE_SIZE);

	return 0;
}
