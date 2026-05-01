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
#include <shared/hash.h>
#include <shared/debug.h>
#include <shared/macro.h>

#include <string.h>


void attr_test_fn
test_normal_pass__hash_table_init_free(
	void
	)
{
	hash_table_t table = hash_table_init(256, NULL, NULL);
	hash_table_free(table);
}


void attr_test_fn
test_normal_fail__hash_table_init_zero_buckets(
	void
	)
{
	hash_table_init(0, NULL, NULL);
}


void attr_test_fn
test_normal_fail__hash_table_free_null(
	void
	)
{
	hash_table_free(NULL);
}


typedef struct for_each_data
{
	const char* key;
	uint32_t len;
	void* value;
}
for_each_data_t;


void
hash_table_for_each_fn(
	str_t key,
	void* value,
	void* data
	)
{
	for_each_data_t** ptr = data;
	for_each_data_t* cur = *ptr;

	cur->key = key->str;
	cur->len = key->len;
	cur->value = value;

	*ptr = cur + 1;
}


void attr_test_fn
test_normal_pass__hash_table_functions(
	void
	)
{
	hash_table_t table = hash_table_init(8, NULL, NULL);

	assert_false(hash_table_has(table, "foo"));
	assert_false(hash_table_has(table, "bar"));
	assert_false(hash_table_modify(table, "foo", (void*) 1));
	assert_false(hash_table_del(table, "foo"));

	hash_table_clear(table);

	assert_true(hash_table_add(table, "foo", (void*) 1));
	assert_true(hash_table_has(table, "foo"));
	assert_eq(hash_table_get(table, "foo"), (void*) 1);

	assert_false(hash_table_set(table, "bar", (void*) 2));
	assert_false(hash_table_add(table, "bar", (void*) 3));

	assert_true(hash_table_has(table, "foo"));
	assert_eq(hash_table_get(table, "foo"), (void*) 1);

	assert_true(hash_table_has(table, "bar"));
	assert_eq(hash_table_get(table, "bar"), (void*) 2);

	assert_true(hash_table_modify(table, "foo", (void*) 3));
	assert_eq(hash_table_get(table, "foo"), (void*) 3);

	assert_true(hash_table_set(table, "foo", (void*) 1));
	assert_eq(hash_table_get(table, "foo"), (void*) 1);

	for_each_data_t data[4];
	for_each_data_t* data_end = data;
	hash_table_for_each(table, hash_table_for_each_fn, &data_end);
	assert_eq(data_end, data + 2);

	for(for_each_data_t* cur = data; cur != data_end; ++cur)
	{
		if(strcmp(cur->key, "foo") == 0)
		{
			assert_eq(cur->len, 3);
			assert_eq(cur->value, (void*) 1);
		}
		else if(strcmp(cur->key, "bar") == 0)
		{
			assert_eq(cur->len, 3);
			assert_eq(cur->value, (void*) 2);
		}
		else
		{
			assert_unreachable();
		}
	}

	assert_true(hash_table_del(table, "foo"));
	assert_false(hash_table_has(table, "foo"));
	assert_false(hash_table_del(table, "foo"));
	assert_true(hash_table_has(table, "bar"));
	assert_eq(hash_table_get(table, "bar"), (void*) 2);

	data_end = data;
	hash_table_for_each(table, hash_table_for_each_fn, &data_end);
	assert_eq(data_end, data + 1);

	for(for_each_data_t* cur = data; cur != data_end; ++cur)
	{
		assert_false(strcmp(cur->key, "bar"));
		assert_eq(cur->len, 3);
		assert_eq(cur->value, (void*) 2);
	}

	hash_table_clear(table);

	assert_false(hash_table_has(table, "foo"));
	assert_false(hash_table_has(table, "bar"));

	data_end = data;
	hash_table_for_each(table, hash_table_for_each_fn, &data_end);
	assert_eq(data_end, data + 0);

	hash_table_free(table);
}


void
hash_table_key_dtor(
	str_t key
	)
{
	assert_not_null(key);

	assert_eq(key->len, 3);
	str_clear(key);
}


void
hash_table_value_dtor(
	void* value
	)
{
	assert_not_null(value);

	assert_true(str_cmp_cstr(value, "test"));
	str_free(value);
}


void attr_test_fn
test_normal_pass__hash_table_key_value_dtors(
	void
	)
{
	hash_table_t table = hash_table_init(8, hash_table_key_dtor, hash_table_value_dtor);

	str_t key = str_init_copy_cstr("foo");
	str_t value = str_init_copy_cstr("test");
	assert_true(hash_table_add(table, key->str, value));
	assert_true(hash_table_has(table, "foo"));
	str_reset(key);
	str_free(key);

	key = str_init_copy_cstr("bar");
	value = str_init_copy_cstr("test");
	assert_true(hash_table_add(table, key->str, value));
	assert_true(hash_table_has(table, "bar"));
	str_reset(key);
	str_free(key);

	key = str_init_copy_cstr("bar");
	value = str_init_copy_cstr("test");
	assert_true(hash_table_modify(table, key->str, value));
	assert_true(hash_table_has(table, "bar"));
	str_reset(key);
	str_free(key);

	assert_true(hash_table_del(table, "bar"));

	hash_table_free(table);
}


void attr_test_fn
test_normal_fail__hash_table_has_null_table(
	void
	)
{
	hash_table_has(NULL, "foo");
}


void attr_test_fn
test_normal_fail__hash_table_has_null_key(
	void
	)
{
	hash_table_t table = hash_table_init(1, NULL, NULL);
	hash_table_has(table, NULL);
}


void attr_test_fn
test_normal_fail__hash_table_has_null(
	void
	)
{
	hash_table_has(NULL, NULL);
}


void attr_test_fn
test_normal_fail__hash_table_add_null_table(
	void
	)
{
	hash_table_add(NULL, "foo", (void*) 1);
}


void attr_test_fn
test_normal_fail__hash_table_add_null_key(
	void
	)
{
	hash_table_t table = hash_table_init(1, NULL, NULL);
	hash_table_add(table, NULL, (void*) 1);
}


void attr_test_fn
test_normal_fail__hash_table_add_null(
	void
	)
{
	hash_table_add(NULL, NULL, (void*) 1);
}


void attr_test_fn
test_normal_fail__hash_table_get_null_table(
	void
	)
{
	hash_table_get(NULL, "foo");
}


void attr_test_fn
test_normal_fail__hash_table_get_null_key(
	void
	)
{
	hash_table_t table = hash_table_init(1, NULL, NULL);
	hash_table_get(table, NULL);
}


void attr_test_fn
test_normal_fail__hash_table_get_null(
	void
	)
{
	hash_table_get(NULL, NULL);
}


void attr_test_fn
test_normal_fail__hash_table_del_null_table(
	void
	)
{
	hash_table_del(NULL, "foo");
}


void attr_test_fn
test_normal_fail__hash_table_del_null_key(
	void
	)
{
	hash_table_t table = hash_table_init(1, NULL, NULL);
	hash_table_del(table, NULL);
}


void attr_test_fn
test_normal_fail__hash_table_del_null(
	void
	)
{
	hash_table_del(NULL, NULL);
}


void attr_test_fn
test_normal_fail__hash_table_for_each_null_table(
	void
	)
{
	hash_table_for_each(NULL, hash_table_for_each_fn, NULL);
}


void attr_test_fn
test_normal_fail__hash_table_for_each_null_fn(
	void
	)
{
	hash_table_t table = hash_table_init(1, NULL, NULL);
	hash_table_for_each(table, NULL, NULL);
}


void attr_test_fn
test_normal_fail__hash_table_for_each_null(
	void
	)
{
	hash_table_for_each(NULL, NULL, NULL);
}


void attr_test_fn
test_normal_fail__hash_table_clear_null_table(
	void
	)
{
	hash_table_clear(NULL);
}


void attr_test_fn
test_normal_fail__hash_table_set_null_table(
	void
	)
{
	hash_table_set(NULL, "foo", (void*) 1);
}


void attr_test_fn
test_normal_fail__hash_table_set_null_key(
	void
	)
{
	hash_table_t table = hash_table_init(1, NULL, NULL);
	hash_table_set(table, NULL, (void*) 1);
}


void attr_test_fn
test_normal_fail__hash_table_set_null(
	void
	)
{
	hash_table_set(NULL, NULL, (void*) 1);
}


void attr_test_fn
test_normal_fail__hash_table_modify_null_table(
	void
	)
{
	hash_table_modify(NULL, "foo", (void*) 1);
}


void attr_test_fn
test_normal_fail__hash_table_modify_null_key(
	void
	)
{
	hash_table_t table = hash_table_init(1, NULL, NULL);
	hash_table_modify(table, NULL, (void*) 1);
}


void attr_test_fn
test_normal_fail__hash_table_modify_null(
	void
	)
{
	hash_table_modify(NULL, NULL, (void*) 1);
}
