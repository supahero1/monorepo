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
#include <shared/alloc/base.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>


bool
file_exists(
	const char* path
	)
{
	struct stat info;
	if(stat(path, &info) != 0)
	{
		return false;
	}

	return S_ISREG(info.st_mode);
}


bool
file_write(
	const char* path,
	file_t file
	)
{
	int fd = open(path, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	if(fd < 0)
	{
		return false;
	}

	if(ftruncate(fd, file.len) != 0)
	{
		return false;
	}

	if(lseek(fd, 0, SEEK_SET) != 0)
	{
		return false;
	}

	ssize_t bytes = write(fd, file.data, file.len);
	close(fd);

	return bytes == file.len;
}


bool
file_read_cap(
	const char* path,
	file_t* file,
	uint64_t cap
	)
{
	int fd = open(path, O_RDONLY);
	if(fd < 0)
	{
		return false;
	}

	bool status = false;

	if(lseek(fd, 0, SEEK_SET) != 0)
	{
		goto goto_end;
	}

	struct stat info;
	if(fstat(fd, &info) != 0)
	{
		goto goto_end;
	}

	file->len = info.st_size;
	if(file->len > cap)
	{
		goto goto_end;
	}

	file->data = alloc_malloc(file->data, file->len);
	hard_assert_ptr(file->data, file->len);

	if(file->len && read(fd, file->data, file->len) != file->len)
	{
		file_free(*file);
	}
	else
	{
		status = true;
	}


	goto_end:

	close(fd);
	return status;
}


bool
file_read(
	const char* path,
	file_t* file
	)
{
	return file_read_cap(path, file, -1);
}


bool
file_remove(
	const char* path
	)
{
	return unlink(path) == 0 || errno == ENOENT;
}


void
file_free(
	file_t file
	)
{
	alloc_free(file.data, file.len);
}


bool
dir_exists(
	const char* path
	)
{
	struct stat info;
	if(stat(path, &info) != 0)
	{
		return false;
	}

	return S_ISDIR(info.st_mode);
}


bool
dir_create(
	const char* path
	)
{
	return mkdir(path, S_IRWXU) == 0 || errno == EEXIST;
}


bool
dir_remove(
	const char* path
	)
{
	return rmdir(path) == 0 || errno == ENOENT;
}
