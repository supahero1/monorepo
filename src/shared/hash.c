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

#include <shared/str.h>
#include <shared/hash.h>
#include <shared/debug.h>
#include <shared/macro.h>
#include <shared/alloc/base.h>

#include <string.h>


typedef struct hash_table_entry
{
	const char* key;
	void* value;

	uint32_t len;
	uint32_t next;
}
hash_table_entry_t;

struct hash_table
{
	uint32_t* buckets;
	hash_table_entry_t* entries;

	uint32_t bucket_count;
	uint32_t entries_used;
	uint32_t entries_size;
	uint32_t free_entry;

	hash_table_key_free_fn_t key_free_fn;
	hash_table_value_free_fn_t value_free_fn;
};


uint32_t
hash_table_hash(
	const char* key,
	uint32_t* len
	)
{
	const char* key_start = key;
	uint32_t hash = 0x811c9dc5;

	while(*key)
	{
		hash ^= *(key++);
		hash *= 0x01000193;
	}

	*len = key - key_start;
	return hash;
}


void
hash_table_default_key_free_fn(
	str_t key
	)
{
	(void) key;
}


void
hash_table_default_value_free_fn(
	void* value
	)
{
	(void) value;
}


hash_table_t
hash_table_init(
	uint32_t bucket_count,
	hash_table_key_free_fn_t key_free_fn,
	hash_table_value_free_fn_t value_free_fn
	)
{
	assert_gt(bucket_count, 0);

	if(!key_free_fn)
	{
		key_free_fn = hash_table_default_key_free_fn;
	}

	if(!value_free_fn)
	{
		value_free_fn = hash_table_default_value_free_fn;
	}

	hash_table_t table = alloc_malloc(table, 1);
	assert_ptr(table, 1);

	table->bucket_count = bucket_count;
	assert_neq(table->bucket_count, 0);

	table->entries_used = 1;
	table->entries_size = 0;
	table->free_entry = 0;

	table->buckets = alloc_calloc(table->buckets, table->bucket_count);
	assert_ptr(table->buckets, table->bucket_count);

	table->entries = NULL;

	table->key_free_fn = key_free_fn;
	table->value_free_fn = value_free_fn;

	return table;
}


void
hash_table_for_each_free_fn(
	str_t key,
	void* value,
	void* data
	)
{
	hash_table_t table = data;
	assert_not_null(table);

	table->key_free_fn(key);
	table->value_free_fn(value);
}


void
hash_table_for_each_free(
	hash_table_t table
	)
{
	assert_not_null(table);

	if(
		table->key_free_fn != hash_table_default_key_free_fn ||
		table->value_free_fn != hash_table_default_value_free_fn
		)
	{
		hash_table_for_each(table, hash_table_for_each_free_fn, table);
	}
}


void
hash_table_free(
	hash_table_t table
	)
{
	assert_not_null(table);

	hash_table_for_each_free(table);

	alloc_free(table->buckets, table->bucket_count);
	alloc_free(table->entries, table->entries_size);

	alloc_free(table, 1);
}


void
hash_table_clear(
	hash_table_t table
	)
{
	assert_not_null(table);

	hash_table_for_each_free(table);

	memset(table->buckets, 0, sizeof(*table->buckets) * table->bucket_count);

	alloc_free(table->entries, table->entries_size);

	table->entries = NULL;
	table->entries_used = 1;
	table->entries_size = 0;
	table->free_entry = 0;
}


void
hash_table_for_each(
	hash_table_t table,
	hash_table_for_each_fn_t fn,
	void* data
	)
{
	assert_not_null(table);
	assert_not_null(fn);

	uint32_t* bucket = table->buckets;
	uint32_t* bucket_end = bucket + table->bucket_count;

	for(; bucket < bucket_end; ++bucket)
	{
		uint32_t entry_idx = *bucket;
		for(hash_table_entry_t* entry = &table->entries[entry_idx]; entry_idx; entry = &table->entries[entry_idx])
		{
			struct str entry_key_data = { (void*) entry->key, entry->len };
			str_t entry_key = &entry_key_data;

			fn(entry_key, entry->value, data);

			entry_idx = entry->next;
		}
	}
}


void
hash_table_ensure_entry(
	hash_table_t table
	)
{
	if(table->entries_used >= table->entries_size)
	{
		uint32_t new_size = (table->entries_used << 1) | 3;
		assert_neq(new_size, table->entries_size);

		table->entries = alloc_remalloc(table->entries, table->entries_size, new_size);
		assert_not_null(table->entries);

		table->entries_size = new_size;
	}
}


uint32_t
hash_table_get_entry(
	hash_table_t table
	)
{
	if(table->free_entry)
	{
		uint32_t entry = table->free_entry;
		table->free_entry = table->entries[entry].next;
		return entry;
	}

	hash_table_ensure_entry(table);

	return table->entries_used++;
}


void
hash_table_ret_entry(
	hash_table_t table,
	uint32_t entry
	)
{
	table->entries[entry].next = table->free_entry;
	table->free_entry = entry;
}


bool
hash_table_has(
	hash_table_t table,
	const char* key
	)
{
	assert_not_null(table);
	assert_not_null(key);

	uint32_t len;
	uint32_t hash = hash_table_hash(key, &len) % table->bucket_count;

	struct str search_key_data = { (void*) key, len };
	str_t search_key = &search_key_data;

	uint32_t entry_idx = table->buckets[hash];
	for(hash_table_entry_t* entry = &table->entries[entry_idx]; entry_idx; entry = &table->entries[entry_idx])
	{
		struct str entry_key_data = { (void*) entry->key, entry->len };
		str_t entry_key = &entry_key_data;

		if(str_case_cmp(search_key, entry_key))
		{
			return true;
		}

		entry_idx = entry->next;
	}

	return false;
}


bool
hash_table_add(
	hash_table_t table,
	const char* key,
	void* value
	)
{
	assert_not_null(table);
	assert_not_null(key);

	hash_table_ensure_entry(table);

	uint32_t len;
	uint32_t hash = hash_table_hash(key, &len) % table->bucket_count;

	struct str search_key_data = { (void*) key, len };
	str_t search_key = &search_key_data;

	uint32_t* next = &table->buckets[hash];
	for(hash_table_entry_t* entry = &table->entries[*next]; *next; entry = &table->entries[*next])
	{
		struct str entry_key_data = { (void*) entry->key, entry->len };
		str_t entry_key = &entry_key_data;

		if(str_case_cmp(search_key, entry_key))
		{
			table->key_free_fn(search_key);
			table->value_free_fn(value);

			return false;
		}

		next = &entry->next;
	}

	uint32_t entry = hash_table_get_entry(table);
	table->entries[entry] =
	(hash_table_entry_t)
	{
		.key = key,
		.value = value,
		.len = len,
		.next = 0
	};

	*next = entry;
	return true;
}


bool
hash_table_set(
	hash_table_t table,
	const char* key,
	void* value
	)
{
	assert_not_null(table);
	assert_not_null(key);

	hash_table_ensure_entry(table);

	uint32_t len;
	uint32_t hash = hash_table_hash(key, &len) % table->bucket_count;

	struct str search_key_data = { (void*) key, len };
	str_t search_key = &search_key_data;

	uint32_t* next = &table->buckets[hash];
	for(hash_table_entry_t* entry = &table->entries[*next]; *next; entry = &table->entries[*next])
	{
		struct str entry_key_data = { (void*) entry->key, entry->len };
		str_t entry_key = &entry_key_data;

		if(str_case_cmp(search_key, entry_key))
		{
			table->key_free_fn(entry_key);
			table->value_free_fn(entry->value);

			entry->key = key;
			entry->value = value;

			return true;
		}

		next = &entry->next;
	}

	uint32_t entry = hash_table_get_entry(table);
	table->entries[entry] =
	(hash_table_entry_t)
	{
		.key = key,
		.value = value,
		.len = len,
		.next = 0
	};

	*next = entry;
	return false;
}


bool
hash_table_modify(
	hash_table_t table,
	const char* key,
	void* value
	)
{
	assert_not_null(table);
	assert_not_null(key);

	uint32_t len;
	uint32_t hash = hash_table_hash(key, &len) % table->bucket_count;

	struct str search_key_data = { (void*) key, len };
	str_t search_key = &search_key_data;

	uint32_t* next = &table->buckets[hash];
	for(hash_table_entry_t* entry = &table->entries[*next]; *next; entry = &table->entries[*next])
	{
		struct str entry_key_data = { (void*) entry->key, entry->len };
		str_t entry_key = &entry_key_data;

		if(str_case_cmp(search_key, entry_key))
		{
			table->key_free_fn(entry_key);
			table->value_free_fn(entry->value);

			entry->key = key;
			entry->value = value;

			return true;
		}

		next = &entry->next;
	}

	return false;
}


void*
hash_table_get(
	hash_table_t table,
	const char* key
	)
{
	assert_not_null(table);
	assert_not_null(key);

	uint32_t len;
	uint32_t hash = hash_table_hash(key, &len) % table->bucket_count;

	struct str search_key_data = { (void*) key, len };
	str_t search_key = &search_key_data;

	uint32_t entry_idx = table->buckets[hash];
	for(hash_table_entry_t* entry = &table->entries[entry_idx]; entry_idx; entry = &table->entries[entry_idx])
	{
		struct str entry_key_data = { (void*) entry->key, entry->len };
		str_t entry_key = &entry_key_data;

		if(str_case_cmp(search_key, entry_key))
		{
			return entry->value;
		}

		entry_idx = entry->next;
	}

	return NULL;
}


bool
hash_table_del(
	hash_table_t table,
	const char* key
	)
{
	assert_not_null(table);
	assert_not_null(key);

	uint32_t len;
	uint32_t hash = hash_table_hash(key, &len) % table->bucket_count;

	struct str search_key_data = { (void*) key, len };
	str_t search_key = &search_key_data;

	uint32_t* next = &table->buckets[hash];
	for(hash_table_entry_t* entry = &table->entries[*next]; *next; entry = &table->entries[*next])
	{
		struct str entry_key_data = { (void*) entry->key, entry->len };
		str_t entry_key = &entry_key_data;

		if(str_case_cmp(search_key, entry_key))
		{
			table->key_free_fn(entry_key);
			table->value_free_fn(entry->value);

			uint32_t next_idx = *next;
			*next = table->entries[next_idx].next;
			hash_table_ret_entry(table, next_idx);

			return true;
		}

		next = &table->entries[*next].next;
	}

	return false;
}
