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
#include <shared/attr.h>
#include <shared/debug.h>
#include <shared/macro.h>

#include <string.h>


void attr_test_fn
test_normal_pass__cstr_alloc_free(
	void
	)
{
	void* cstr = cstr_alloc(0);
	assert_true(cstr_cmp(cstr, ""));
	cstr_free(cstr);

	cstr = cstr_alloc(4);
	memcpy(cstr, "test", 4);
	cstr_free(cstr);
}


void attr_test_fn
test_normal_pass__cstr_resize(
	void
	)
{
	void* cstr = cstr_alloc(0);
	assert_true(cstr_cmp(cstr, ""));

	cstr = cstr_resize(cstr, 4);
	memcpy(cstr, "test", 4);
	assert_true(cstr_cmp(cstr, "test"));

	cstr = cstr_resize(cstr, 0);
	assert_true(cstr_cmp(cstr, ""));

	cstr_free(cstr);
}


void attr_test_fn
test_normal_pass__cstr_resize_len(
	void
	)
{
	void* cstr = cstr_alloc(0);
	assert_true(cstr_cmp(cstr, ""));

	cstr = cstr_resize_len(cstr, 0, 4);
	memcpy(cstr, "test", 4);
	assert_true(cstr_cmp(cstr, "test"));

	cstr = cstr_resize_len(cstr, 4, 0);
	assert_true(cstr_cmp(cstr, ""));

	cstr_free(cstr);
}


void attr_test_fn
test_normal_pass__cstr_init_free(
	void
	)
{
	void* cstr = cstr_init("test");
	cstr_free(cstr);
}


void attr_test_fn
test_normal_pass__cstr_init_len_free(
	void
	)
{
	void* cstr = cstr_init_len("testo", 4);
	assert_true(cstr_cmp(cstr, "test"));
	cstr_free(cstr);
}


void attr_test_fn
test_normal_pass__cstr_init_len_free_null(
	void
	)
{
	void* cstr = cstr_init_len(NULL, 0);
	assert_true(cstr_cmp(cstr, ""));
	cstr_free(cstr);
}


void attr_test_fn
test_normal_pass__cstr_cmp(
	void
	)
{
	void* cstr1 = cstr_init_len("testa", 4);
	void* cstr2 = cstr_init_len("test", 4);
	assert_true(cstr_cmp(cstr1, cstr2));
	cstr_free(cstr2);
	cstr_free(cstr1);

	cstr1 = cstr_init_len("foo", 3);
	cstr2 = cstr_init_len("bar", 3);
	assert_false(cstr_cmp(cstr1, cstr2));
	cstr_free(cstr2);
	cstr_free(cstr1);

	cstr1 = cstr_init_len("foo", 3);
	cstr2 = cstr_init_len("fooo", 4);
	assert_false(cstr_cmp(cstr1, cstr2));
	cstr_free(cstr2);
	cstr_free(cstr1);

	cstr1 = cstr_init_len("foo", 3);
	cstr2 = cstr_init_len("fOo", 3);
	assert_false(cstr_cmp(cstr1, cstr2));
	cstr_free(cstr2);
	cstr_free(cstr1);
}


void attr_test_fn
test_normal_pass__cstr_case_cmp(
	void
	)
{
	void* cstr1 = cstr_init_len("test", 4);
	void* cstr2 = cstr_init_len("testa", 4);
	assert_true(cstr_case_cmp(cstr1, cstr2));
	cstr_free(cstr2);
	cstr_free(cstr1);

	cstr1 = cstr_init_len("foo", 3);
	cstr2 = cstr_init_len("bar", 3);
	assert_false(cstr_case_cmp(cstr1, cstr2));
	cstr_free(cstr2);
	cstr_free(cstr1);

	cstr1 = cstr_init_len("foo", 3);
	cstr2 = cstr_init_len("fooo", 4);
	assert_false(cstr_case_cmp(cstr1, cstr2));
	cstr_free(cstr2);
	cstr_free(cstr1);

	cstr1 = cstr_init_len("foo", 3);
	cstr2 = cstr_init_len("fOo", 3);
	assert_true(cstr_case_cmp(cstr1, cstr2));
	cstr_free(cstr2);
	cstr_free(cstr1);
}


void attr_test_fn
test_normal_fail___cstr_resize_null(
	void
	)
{
	cstr_resize(NULL, 0);
}


void attr_test_fn
test_normal_pass__cstr_resize_len_null(
	void
	)
{
	void* cstr = cstr_resize_len(NULL, 0, 4);
	memcpy(cstr, "test", 4);
	assert_true(cstr_cmp(cstr, "test"));

	cstr_free(cstr);
}


void attr_test_fn
test_normal_fail__cstr_resize_len_null_non_zero_len(
	void
	)
{
	cstr_resize_len(NULL, 4, 0);
}


void attr_test_fn
test_normal_fail__cstr_init_null(
	void
	)
{
	cstr_init(NULL);
}


void attr_test_fn
test_normal_fail__cstr_init_len_null_non_zero_len(
	void
	)
{
	cstr_init_len(NULL, 4);
}


void attr_test_fn
test_normal_pass__cstr_free_null(
	void
	)
{
	cstr_free(NULL);
}


void attr_test_fn
test_normal_pass__cstr_free_null_len(
	void
	)
{
	cstr_free_len(NULL, 0);
}


void attr_test_fn
test_normal_fail__cstr_free_null_len_non_zero_len(
	void
	)
{
	cstr_free_len(NULL, 4);
}


void attr_test_fn
test_normal_fail__cstr_cmp_null_cstr1(
	void
	)
{
	cstr_cmp(NULL, "test");
}


void attr_test_fn
test_normal_fail__cstr_cmp_null_cstr2(
	void
	)
{
	cstr_cmp("test", NULL);
}


void attr_test_fn
test_normal_fail__cstr_cmp_null(
	void
	)
{
	cstr_cmp(NULL, NULL);
}


void attr_test_fn
test_normal_fail__cstr_case_cmp_null_cstr1(
	void
	)
{
	cstr_case_cmp(NULL, "test");
}


void attr_test_fn
test_normal_fail__cstr_case_cmp_null_cstr2(
	void
	)
{
	cstr_case_cmp("test", NULL);
}


void attr_test_fn
test_normal_fail__cstr_case_cmp_null(
	void
	)
{
	cstr_case_cmp(NULL, NULL);
}


void attr_test_fn
test_normal_pass__str_init_free(
	void
	)
{
	str_t str = str_init();
	str_free(str);
}


void attr_test_fn
test_normal_pass__str_init_copy_cstr(
	void
	)
{
	str_t str = str_init_copy_cstr("test");
	assert_true(str_cmp_cstr(str, "test"));
	str_free(str);
}


void attr_test_fn
test_normal_pass__str_init_move(
	void
	)
{
	str_t str = str_init_copy_cstr("test");
	assert_true(str_cmp_cstr(str, "test"));

	str_t str2 = str_init_move(str);

	assert_true(str_is_empty(str));
	assert_true(str_cmp_cstr(str2, "test"));

	str_free(str);
	str_free(str2);
}


void attr_test_fn
test_normal_pass__str_init_move_empty(
	void
	)
{
	str_t str = str_init();
	str_t str2 = str_init_move(str);

	assert_true(str_is_empty(str));
	assert_true(str_is_empty(str2));

	str_free(str);
	str_free(str2);
}


void attr_test_fn
test_normal_pass__str_set_copy_cstr(
	void
	)
{
	str_t str = str_init();

	str_set_copy_cstr(str, "test");
	assert_true(str_cmp_cstr(str, "test"));

	str_set_copy_cstr(str, "test2");
	assert_true(str_cmp_cstr(str, "test2"));

	str_free(str);
}


void attr_test_fn
test_normal_pass__str_set_move_cstr(
	void
	)
{
	str_t str = str_init_copy_cstr("test");
	void* ptr = str->str;
	str_reset(str);
	str_free(str);

	str = str_init_move_cstr(ptr);
	assert_true(str_cmp_cstr(str, "test"));
	str_free(str);
}


void attr_test_fn
test_normal_pass__str_set_copy(
	void
	)
{
	str_t str = str_init_copy_cstr("test");

	str_t str2 = str_init();

	str_set_copy(str2, str);

	assert_true(str_cmp_cstr(str, "test"));
	assert_true(str_cmp_cstr(str2, "test"));

	str_free(str);
	str_free(str2);
}


void attr_test_fn
test_normal_pass__str_set_move(
	void
	)
{
	str_t str = str_init_copy_cstr("test");

	str_t str2 = str_init();
	str_set_move(str2, str);

	assert_true(str_is_empty(str));
	assert_true(str_cmp_cstr(str2, "test"));

	str_free(str);
	str_free(str2);
}


void attr_test_fn
test_normal_pass__str_cmp(
	void
	)
{
	str_t str1 = str_init_copy_len("testa", 4);
	str_t str2 = str_init_copy_len("test", 4);
	assert_true(str_cmp(str1, str2));

	str_set_copy_len(str1, "foo", 3);
	str_set_copy_len(str2, "bar", 3);
	assert_false(str_cmp(str1, str2));

	str_set_copy_len(str1, "foo", 3);
	str_set_copy_len(str2, "foo", 4);
	assert_false(str_cmp(str1, str2));

	str_set_copy_len(str1, "foo", 3);
	str_set_copy_len(str2, "fOo", 3);
	assert_false(str_cmp(str1, str2));

	str_free(str1);
	str_free(str2);
}


void attr_test_fn
test_normal_pass__str_case_cmp(
	void
	)
{
	str_t str1 = str_init_copy_len("test", 4);
	str_t str2 = str_init_copy_len("testa", 4);
	assert_true(str_case_cmp(str1, str2));

	str_set_copy_len(str1, "foo", 3);
	str_set_copy_len(str2, "bar", 3);
	assert_false(str_case_cmp(str1, str2));

	str_set_copy_len(str1, "foo", 3);
	str_set_copy_len(str2, "foo", 4);
	assert_false(str_case_cmp(str1, str2));

	str_set_copy_len(str1, "foo", 3);
	str_set_copy_len(str2, "fOo", 3);
	assert_true(str_case_cmp(str1, str2));

	str_free(str1);
	str_free(str2);
}


void attr_test_fn
test_normal_pass__str_cmp_cstr(
	void
	)
{
	str_t str = str_init_copy_len("test", 4);
	assert_true(str_cmp_cstr(str, "test"));

	str_set_copy_len(str, "foo", 3);
	assert_false(str_cmp_cstr(str, "bar"));

	str_set_copy_len(str, "foo", 3);
	assert_false(str_cmp_cstr(str, "foo2"));

	str_set_copy_len(str, "foo", 3);
	assert_false(str_cmp_cstr(str, "fOo"));

	str_free(str);
}


void attr_test_fn
test_normal_pass__str_case_cmp_cstr(
	void
	)
{
	str_t str = str_init_copy_len("test", 4);
	assert_true(str_case_cmp_cstr(str, "test"));

	str_set_copy_len(str, "foo", 3);
	assert_false(str_case_cmp_cstr(str, "bar"));

	str_set_copy_len(str, "foo", 3);
	assert_false(str_case_cmp_cstr(str, "foo2"));

	str_set_copy_len(str, "foo", 3);
	assert_true(str_case_cmp_cstr(str, "fOo"));

	str_free(str);
}


void attr_test_fn
test_normal_pass__str_free_null(
	void
	)
{
	str_free(NULL);
}


void attr_test_fn
test_normal_fail__str_reset_null(
	void
	)
{
	str_reset(NULL);
}


void attr_test_fn
test_normal_fail__str_init_copy_cstr_null(
	void
	)
{
	str_init_copy_cstr(NULL);
}


void attr_test_fn
test_normal_fail__str_init_move_cstr_null(
	void
	)
{
	str_init_move_cstr(NULL);
}


void attr_test_fn
test_normal_pass__str_init_copy_len_null_cstr(
	void
	)
{
	str_t str = str_init_copy_len(NULL, 0);
	str_free(str);
}


void attr_test_fn
test_normal_fail__str_init_copy_len_null_cstr_non_zero_len(
	void
	)
{
	str_init_copy_len(NULL, 4);
}


void attr_test_fn
test_normal_pass__str_init_move_len_null_cstr(
	void
	)
{
	str_t str = str_init_move_len(NULL, 0);
	str_free(str);
}


void attr_test_fn
test_normal_fail__str_init_move_len_null_cstr_non_zero_len(
	void
	)
{
	str_init_move_len(NULL, 4);
}


void attr_test_fn
test_normal_fail__str_init_copy_null(
	void
	)
{
	str_init_copy(NULL);
}


void attr_test_fn
test_normal_fail__str_init_move_null_other(
	void
	)
{
	str_init_move(NULL);
}


void attr_test_fn
test_normal_fail__str_clear_null(
	void
	)
{
	str_clear(NULL);
}


void attr_test_fn
test_normal_fail__str_is_empty_null(
	void
	)
{
	str_is_empty(NULL);
}


void attr_test_fn
test_normal_fail__str_set_copy_cstr_null_str(
	void
	)
{
	str_set_copy_cstr(NULL, "test");
}


void attr_test_fn
test_normal_fail__str_set_copy_cstr_null_cstr(
	void
	)
{
	str_t str = str_init();
	str_set_copy_cstr(str, NULL);
}


void attr_test_fn
test_normal_fail__str_set_copy_cstr_null(
	void
	)
{
	str_set_copy_cstr(NULL, NULL);
}


void attr_test_fn
test_normal_fail__str_set_move_cstr_null_str(
	void
	)
{
	str_set_move_cstr(NULL, "test");
}


void attr_test_fn
test_normal_fail__str_set_move_cstr_null_cstr(
	void
	)
{
	str_t str = str_init();
	str_set_move_cstr(str, NULL);
}


void attr_test_fn
test_normal_fail__str_set_move_cstr_null(
	void
	)
{
	str_set_move_cstr(NULL, NULL);
}


void attr_test_fn
test_normal_fail__str_set_copy_null_str(
	void
	)
{
	str_t str = str_init();
	str_set_copy(NULL, str);
}


void attr_test_fn
test_normal_fail__str_set_copy_null_other(
	void
	)
{
	str_t str = str_init();
	str_set_copy(str, NULL);
}


void attr_test_fn
test_normal_fail__str_set_copy_null(
	void
	)
{
	str_set_copy(NULL, NULL);
}


void attr_test_fn
test_normal_fail__str_set_move_null_str(
	void
	)
{
	str_t str = str_init();
	str_set_move(NULL, str);
}


void attr_test_fn
test_normal_fail__str_set_move_null_other(
	void
	)
{
	str_t str = str_init();
	str_set_move(str, NULL);
}


void attr_test_fn
test_normal_fail__str_set_move_null(
	void
	)
{
	str_set_move(NULL, NULL);
}


void attr_test_fn
test_normal_fail__str_resize_null(
	void
	)
{
	str_resize(NULL, 4);
}


void attr_test_fn
test_normal_fail__str_cmp_null_str1(
	void
	)
{
	str_t str = str_init_copy_len("test", 4);
	str_cmp(NULL, str);
}


void attr_test_fn
test_normal_fail__str_cmp_null_str2(
	void
	)
{
	str_t str = str_init_copy_len("test", 4);
	str_cmp(str, NULL);
}


void attr_test_fn
test_normal_fail__str_cmp_null(
	void
	)
{
	str_cmp(NULL, NULL);
}


void attr_test_fn
test_normal_fail__str_case_cmp_null_str1(
	void
	)
{
	str_t str = str_init_copy_len("test", 4);
	str_case_cmp(NULL, str);
}


void attr_test_fn
test_normal_fail__str_case_cmp_null_str2(
	void
	)
{
	str_t str = str_init_copy_len("test", 4);
	str_case_cmp(str, NULL);
}


void attr_test_fn
test_normal_fail__str_case_cmp_null(
	void
	)
{
	str_case_cmp(NULL, NULL);
}


void attr_test_fn
test_normal_fail__str_cmp_cstr_null_str(
	void
	)
{
	str_cmp_cstr(NULL, "test");
}


void attr_test_fn
test_normal_fail__str_cmp_cstr_null_cstr(
	void
	)
{
	str_t str = str_init_copy_len("test", 4);
	str_cmp_cstr(str, NULL);
}


void attr_test_fn
test_normal_fail__str_cmp_cstr_null(
	void
	)
{
	str_cmp_cstr(NULL, NULL);
}


void attr_test_fn
test_normal_fail__str_case_cmp_cstr_null_str(
	void
	)
{
	str_case_cmp_cstr(NULL, "test");
}


void attr_test_fn
test_normal_fail__str_case_cmp_cstr_null_cstr(
	void
	)
{
	str_t str = str_init_copy_len("test", 4);
	str_case_cmp_cstr(str, NULL);
}


void attr_test_fn
test_normal_fail__str_case_cmp_cstr_null(
	void
	)
{
	str_case_cmp_cstr(NULL, NULL);
}


void attr_test_fn
test_normal_fail__str_cmp_len_null_str(
	void
	)
{
	str_cmp_len(NULL, "test", 4);
}


void attr_test_fn
test_normal_pass__str_cmp_len_null_cstr(
	void
	)
{
	str_t str = str_init_copy_len("test", 4);
	assert_false(str_cmp_len(str, NULL, 0));
	str_free(str);
}


void attr_test_fn
test_normal_fail__str_cmp_len_null_cstr_non_zero_len(
	void
	)
{
	str_t str = str_init_copy_len("test", 4);
	str_cmp_len(str, NULL, 4);
}


void attr_test_fn
test_normal_fail__str_cmp_len_null(
	void
	)
{
	str_cmp_len(NULL, NULL, 4);
}


void attr_test_fn
test_normal_fail__str_case_cmp_len_null_str(
	void
	)
{
	str_case_cmp_len(NULL, "test", 4);
}


void attr_test_fn
test_normal_pass__str_case_cmp_len_null_cstr(
	void
	)
{
	str_t str = str_init_copy_len("test", 4);
	assert_false(str_case_cmp_len(str, NULL, 0));
	str_free(str);
}


void attr_test_fn
test_normal_fail__str_case_cmp_len_null_cstr_non_zero_len(
	void
	)
{
	str_t str = str_init_copy_len("test", 4);
	str_case_cmp_len(str, NULL, 4);
}


void attr_test_fn
test_normal_fail__str_case_cmp_len_null(
	void
	)
{
	str_case_cmp_len(NULL, NULL, 4);
}
