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

#include <tests/base.h>
#include <shared/attr.h>
#include <shared/file.h>
#include <shared/debug.h>
#include <shared/macro.h>

#include <stdint.h>
#include <string.h>

#define TEST_WRITE_FILENAME	\
(test_is_on_valgrind ? "bin/tests/shared/file_write.txt.val" : "bin/tests/shared/file_write.txt")

#define TEST_EXISTS_FILENAME	\
(test_is_on_valgrind ? "bin/tests/shared/file_exists.txt.val" : "bin/tests/shared/file_exists.txt")

#define TEST_EXISTS_DIRNAME	\
(test_is_on_valgrind ? "bin/tests/shared/dir_exists" : "bin/tests/shared/dir_exists")


uint8_t data[] = "test\n";
uint64_t len = sizeof(data) - 1;


void attr_test_fn
test_normal_pass__file_exists_remove(
	void
	)
{
	file_remove(TEST_EXISTS_FILENAME);

	bool status = file_exists(TEST_EXISTS_FILENAME);
	assert_false(status);

	file_t file =
	{
		.data = data,
		.len = len
	};
	status = file_write(TEST_EXISTS_FILENAME, file);
	assert_true(status);

	status = file_exists(TEST_EXISTS_FILENAME);
	assert_true(status);

	status = file_remove(TEST_EXISTS_FILENAME);
	assert_true(status);

	status = file_exists(TEST_EXISTS_FILENAME);
	assert_false(status);

	status = file_remove(TEST_EXISTS_FILENAME);
	assert_true(status);
}


void attr_test_fn
test_normal_pass__file_read_not_empty(
	void
	)
{
	file_t file;
	bool status = file_read("tests/shared/file_read_1.txt", &file);
	assert_true(status);

	assert_eq(file.len, len);
	assert_false(memcmp(file.data, data, len));

	file_free(file);
}


void attr_test_fn
test_normal_pass__file_read_empty(
	void
	)
{
	file_t file;
	bool status = file_read("tests/shared/file_read_2.txt", &file);
	assert_true(status);

	assert_eq(file.len, 0);

	file_free(file);
}


void attr_test_fn
test_normal_pass__file_read_non_existent(
	void
	)
{
	file_t file;
	bool status = file_read("tests/shared/file_read_non_existent.txt", &file);
	assert_false(status);
}


void attr_test_fn
test_normal_pass__file_read_cap(
	void
	)
{
	file_t file;
	bool status = file_read_cap("tests/shared/file_read_1.txt", &file, len);
	assert_true(status);

	assert_eq(file.len, len);
	assert_false(memcmp(file.data, data, len));

	file_free(file);
}


void attr_test_fn
test_normal_pass__file_read_cap_too_small(
	void
	)
{
	file_t file;
	bool status = file_read_cap("tests/shared/file_read_1.txt", &file, len - 1);
	assert_false(status);
}


void attr_test_fn
test_normal_pass__file_read_cap_non_existent(
	void
	)
{
	file_t file;
	bool status = file_read_cap("tests/shared/file_read_non_existent.txt", &file, len - 1);
	assert_false(status);
}


void attr_test_fn
test_normal_pass__file_write(
	void
	)
{
	file_t file =
	{
		.data = data,
		.len = len
	};
	bool status = file_write(TEST_WRITE_FILENAME, file);
	assert_true(status);

	status = file_read(TEST_WRITE_FILENAME, &file);
	assert_true(status);

	assert_eq(file.len, len);
	assert_false(memcmp(file.data, data, len));

	file_free(file);
}


void attr_test_fn
test_normal_pass__file_read_multiple_not_move_cursor(
	void
	)
{
	file_t file;
	bool status = file_read("tests/shared/file_read_1.txt", &file);
	assert_true(status);

	assert_eq(file.len, len);
	assert_false(memcmp(file.data, data, len));

	file_free(file);

	status = file_read("tests/shared/file_read_1.txt", &file);
	assert_true(status);

	assert_eq(file.len, len);
	assert_false(memcmp(file.data, data, len));

	file_free(file);
}


void attr_test_fn
test_normal_pass__file_write_multiple_not_append(
	void
	)
{
	file_t file =
	{
		.data = data,
		.len = len
	};
	bool status = file_write(TEST_WRITE_FILENAME, file);
	assert_true(status);

	status = file_write(TEST_WRITE_FILENAME, file);
	assert_true(status);

	status = file_read(TEST_WRITE_FILENAME, &file);
	assert_true(status);

	assert_eq(file.len, len);
	assert_false(memcmp(file.data, data, len));

	file_free(file);
}


void attr_test_fn
test_normal_pass__dir_exists_create(
	void
	)
{
	dir_remove(TEST_EXISTS_DIRNAME);

	bool status = dir_exists("tests/shared");
	assert_true(status);

	status = dir_exists(TEST_EXISTS_DIRNAME);
	assert_false(status);

	status = dir_create(TEST_EXISTS_DIRNAME);
	assert_true(status);

	status = dir_exists(TEST_EXISTS_DIRNAME);
	assert_true(status);

	status = dir_create(TEST_EXISTS_DIRNAME);
	assert_true(status);

	status = dir_remove(TEST_EXISTS_DIRNAME);
	assert_true(status);

	status = dir_exists(TEST_EXISTS_DIRNAME);
	assert_false(status);

	status = dir_remove(TEST_EXISTS_DIRNAME);
	assert_true(status);
}
