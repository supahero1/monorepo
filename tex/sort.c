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

#include <shared/debug.h>
#include <shared/macro.h>

#include <png.h>
#include <stdio.h>
#include <dirent.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>


uint32_t
sort_sizeof_png(
	const char* path
	)
{
	FILE* file = fopen(path, "rb");
	assert_not_null(file);

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	assert_not_null(png);

	png_infop info = png_create_info_struct(png);
	assert_not_null(info);

	png_init_io(png, file);
	png_read_info(png, info);

	uint32_t size = png_get_image_width(png, info);

	png_destroy_read_struct(&png, &info, NULL);
	fclose(file);

	return size;
}


int
main(
	void
	)
{
	struct dirent* entry;
	DIR* dir = opendir("tex/img");
	assert_not_null(dir);

	char old_path[512];
	char dir_path[512];
	char new_path[512];
	int sizes[16] = {0};

	while((entry = readdir(dir)) != NULL)
	{
		if(!strstr(entry->d_name, ".png"))
		{
			continue;
		}

		sprintf(old_path, "tex/img/%s", entry->d_name);

			uint32_t size = sort_sizeof_png(old_path);
			assert_neq(size, 0);
			assert_true(MACRO_IS_POWER_OF_2(size));

			uint32_t size_log = MACRO_FLOOR_LOG2(size);
			if(sizes[size_log] == 0)
			{
			sizes[size_log] = 1;

			sprintf(dir_path, "tex/img/%u", size);
#ifdef _WIN32
			int status = mkdir(dir_path);
#else
			int status = mkdir(dir_path, 0755);
#endif
			assert_eq(status, 0);
		}

		sprintf(new_path, "tex/img/%u/%s", size, entry->d_name);
		rename(old_path, new_path);
	}

	closedir(dir);

	return 0;
}
