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
#include <shared/debug.h>
#include <shared/macro.h>
#include <shared/alloc/base.h>

#include <string.h>
#include <strings.h>


void*
cstr_alloc(
	uint64_t len
	)
{
	uint8_t* cstr = alloc_malloc(cstr, len + 1);
	assert_ptr(cstr, len + 1);

	cstr[len] = '\0';

	return cstr;
}


void*
cstr_resize(
	void* cstr,
	uint64_t new_len
	)
{
	assert_not_null(cstr);

	return cstr_resize_len(cstr, strlen(cstr), new_len);
}


void*
cstr_resize_len(
	void* cstr,
	uint64_t old_len,
	uint64_t new_len
	)
{
	assert_ptr(cstr, old_len);

	if(!cstr && !old_len)
	{
		return cstr_alloc(new_len);
	}

	cstr = alloc_remalloc(cstr, old_len + 1, new_len + 1);
	assert_not_null(cstr);

	((uint8_t*) cstr)[new_len] = '\0';

	return cstr;
}


void*
cstr_init(
	const void* cstr
	)
{
	assert_not_null(cstr);

	return cstr_init_len(cstr, strlen(cstr));
}


void*
cstr_init_len(
	const void* cstr,
	uint64_t len
	)
{
	assert_ptr(cstr, len);

	void* copy = cstr_alloc(len);
	memcpy(copy, cstr, len);
	return copy;
}


void
cstr_free(
	const void* cstr
	)
{
	if(!cstr)
	{
		return;
	}

	cstr_free_len(cstr, strlen(cstr));
}


void
cstr_free_len(
	const void* cstr,
	uint64_t len
	)
{
	assert_ptr(cstr, len);

	if(cstr)
	{
		alloc_free(cstr, len + 1);
	}
}


bool
cstr_cmp(
	const void* cstr1,
	const void* cstr2
	)
{
	assert_not_null(cstr1);
	assert_not_null(cstr2);

	return !strcmp(cstr1, cstr2);
}


bool
cstr_case_cmp(
	const void* cstr1,
	const void* cstr2
	)
{
	assert_not_null(cstr1);
	assert_not_null(cstr2);

	return !strcasecmp(cstr1, cstr2);
}





str_t
str_init(
	void
	)
{
	str_t str = alloc_malloc(str, 1);
	assert_ptr(str, 1);

	str->str = NULL;
	str->len = 0;

	return str;
}


str_t
str_init_copy_cstr(
	const void* cstr
	)
{
	assert_not_null(cstr);

	return str_init_copy(&(
		(struct str)
		{
			.str = (void*) cstr,
			.len = strlen(cstr)
		}
		));
}


str_t
str_init_move_cstr(
	void* cstr
	)
{
	assert_not_null(cstr);

	return str_init_move(&(
		(struct str)
		{
			.str = cstr,
			.len = strlen(cstr)
		}
		));
}


str_t
str_init_copy_len(
	const void* cstr,
	uint64_t len
	)
{
	assert_ptr(cstr, len);

	return str_init_copy(&(
		(struct str)
		{
			.str = (void*) cstr,
			.len = len
		}
		));
}


str_t
str_init_move_len(
	void* cstr,
	uint64_t len
	)
{
	assert_ptr(cstr, len);

	return str_init_move(&(
		(struct str)
		{
			.str = cstr,
			.len = len
		}
		));
}


str_t
str_init_copy(
	const str_t other
	)
{
	assert_not_null(other);
	assert_ptr(other->str, other->len);

	str_t str = str_init();
	str_set_copy(str, other);
	return str;
}


str_t
str_init_move(
	str_t other
	)
{
	assert_not_null(other);
	assert_ptr(other->str, other->len);

	str_t str = str_init();
	str_set_move(str, other);
	return str;
}


void
str_free_str(
	str_t str
	)
{
	assert_not_null(str);

	cstr_free_len(str->str, str->len);
}


void
str_free(
	str_t str
	)
{
	if(!str)
	{
		return;
	}

	str_free_str(str);

	alloc_free(str, 1);
}


void
str_reset(
	str_t str
	)
{
	assert_not_null(str);

	str->str = NULL;
	str->len = 0;
}


void
str_clear(
	str_t str
	)
{
	assert_not_null(str);

	str_free_str(str);

	str->str = NULL;
	str->len = 0;
}


bool
str_is_empty(
	const str_t str
	)
{
	assert_not_null(str);

	if(str->len == 0)
	{
		assert_true(str->str == NULL || ((uint8_t*) str->str)[0] == '\0');
	}

	return str->len == 0;
}


void
str_set_copy_cstr(
	str_t str,
	const void* cstr
	)
{
	assert_not_null(str);
	assert_not_null(cstr);

	str_clear(str);
	str_set_copy(str, &(
		(struct str)
		{
			.str = (void*) cstr,
			.len = strlen(cstr)
		}
		));
}


void
str_set_move_cstr(
	str_t str,
	void* cstr
	)
{
	assert_not_null(str);
	assert_not_null(cstr);

	str_clear(str);
	str_set_move(str, &(
		(struct str)
		{
			.str = cstr,
			.len = strlen(cstr)
		}
		));
}


void
str_set_copy_len(
	str_t str,
	const void* cstr,
	uint64_t len
	)
{
	assert_not_null(str);
	assert_ptr(cstr, len);

	str_clear(str);
	str_set_copy(str, &(
		(struct str)
		{
			.str = (void*) cstr,
			.len = len
		}
		));
}


void
str_set_move_len(
	str_t str,
	void* cstr,
	uint64_t len
	)
{
	assert_not_null(str);
	assert_ptr(cstr, len);

	str_clear(str);
	str_set_move(str, &(
		(struct str)
		{
			.str = cstr,
			.len = len
		}
		));
}


void
str_set_copy(
	str_t str,
	const str_t other
	)
{
	assert_not_null(str);
	assert_not_null(other);

	str_clear(str);

	if(other->str)
	{
		str->len = other->len;
		str->str = cstr_init_len(other->str, other->len);
	}
}


void
str_set_move(
	str_t str,
	str_t other
	)
{
	assert_not_null(str);
	assert_not_null(other);

	str_clear(str);

	str->len = other->len;
	str->str = other->str;

	other->str = NULL;
	other->len = 0;
}


void
str_resize(
	str_t str,
	uint64_t len
	)
{
	assert_not_null(str);

	str->str = cstr_resize_len(str->str, str->len, len);
	str->len = len;
}


bool
str_cmp(
	const str_t str1,
	const str_t str2
	)
{
	assert_not_null(str1);
	assert_not_null(str2);
	assert_ptr(str1->str, str1->len);
	assert_ptr(str2->str, str2->len);

	if(str1->len != str2->len)
	{
		return false;
	}

	return !strncmp(str1->str, str2->str, str1->len);
}


bool
str_case_cmp(
	const str_t str1,
	const str_t str2
	)
{
	assert_not_null(str1);
	assert_not_null(str2);
	assert_ptr(str1->str, str1->len);
	assert_ptr(str2->str, str2->len);

	if(str1->len != str2->len)
	{
		return false;
	}

	return !strncasecmp(str1->str, str2->str, str1->len);
}


bool
str_cmp_cstr(
	const str_t str,
	const void* cstr
	)
{
	assert_not_null(str);
	assert_not_null(cstr);
	assert_ptr(str->str, str->len);

	uint64_t cstr_len = strlen(cstr);
	return str_cmp_len(str, cstr, cstr_len);
}


bool
str_case_cmp_cstr(
	const str_t str,
	const void* cstr
	)
{
	assert_not_null(str);
	assert_not_null(cstr);
	assert_ptr(str->str, str->len);

	uint64_t cstr_len = strlen(cstr);
	return str_case_cmp_len(str, cstr, cstr_len);
}


bool
str_cmp_len(
	const str_t str,
	const void* cstr,
	uint64_t len
	)
{
	assert_not_null(str);
	assert_ptr(cstr, len);
	assert_ptr(str->str, str->len);

	if(str->len != len)
	{
		return false;
	}

	return !strncmp(str->str, cstr, str->len);
}


bool
str_case_cmp_len(
	const str_t str,
	const void* cstr,
	uint64_t len
	)
{
	assert_not_null(str);
	assert_ptr(cstr, len);
	assert_ptr(str->str, str->len);

	if(str->len != len)
	{
		return false;
	}

	return !strncasecmp(str->str, cstr, str->len);
}
