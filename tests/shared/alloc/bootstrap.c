/*
 *   Copyright 2026 Franciszek Balcerak
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
#include <shared/debug.h>
#include <shared/macro.h>
#include <shared/alloc/types.h>
#include <shared/alloc/consts.h>
#include <shared/alloc/bootstrap.h>

#include <stdint.h>
#include <string.h>

#define BOOTSTRAP_MAX_LIVE 4096


typedef struct bootstrap_live_alloc
{
	uint8_t* ptr;
	alloc_t size;
}
bootstrap_live_alloc_t;


bootstrap_live_alloc_t bootstrap_live[BOOTSTRAP_MAX_LIVE];
uint32_t bootstrap_live_count = 0;


alloc_t
bootstrap_expected_alignment(
	alloc_t size
	)
{
	assert_neq(size, 0);

	alloc_t alignment = MACRO_NEXT_OR_EQUAL_POWER_OF_2(size);
	alignment = MACRO_MIN(alignment, alloc_consts.page.size);
	return alignment;
}


void
bootstrap_assert_zeroed(
	const uint8_t* ptr,
	alloc_t size
	)
{
	assert_not_null(ptr);

	for(alloc_t i = 0; i < size; ++i)
	{
		assert_eq(ptr[i], 0);
	}
}


void
bootstrap_assert_non_overlapping(
	const uint8_t* ptr,
	alloc_t size,
	int skip_idx
	)
{
	assert_not_null(ptr);
	assert_neq(size, 0);

	const uint8_t* a0 = ptr;
	const uint8_t* a1 = ptr + size;

	for(uint32_t i = 0; i < bootstrap_live_count; ++i)
	{
		if((int) i == skip_idx)
		{
			continue;
		}

		const uint8_t* b0 = bootstrap_live[i].ptr;
		const uint8_t* b1 = b0 + bootstrap_live[i].size;

		bool overlaps = a0 < b1 && b0 < a1;
		assert_false(overlaps);
	}
}


int
bootstrap_find_live_idx(
	const uint8_t* ptr
	)
{
	for(uint32_t i = 0; i < bootstrap_live_count; ++i)
	{
		if(bootstrap_live[i].ptr == ptr)
		{
			return i;
		}
	}

	return -1;
}


void
bootstrap_add_live(
	uint8_t* ptr,
	alloc_t size
	)
{
	assert_not_null(ptr);
	assert_neq(size, 0);
	assert_lt(bootstrap_live_count, BOOTSTRAP_MAX_LIVE);
	assert_eq(bootstrap_find_live_idx(ptr), -1);

	bootstrap_live[bootstrap_live_count++] =
	(bootstrap_live_alloc_t)
	{
		.ptr = ptr,
		.size = size
	};
}


void
bootstrap_remove_live_idx(
	uint32_t idx
	)
{
	assert_lt(idx, bootstrap_live_count);

	bootstrap_live[idx] = bootstrap_live[bootstrap_live_count - 1];
	--bootstrap_live_count;
}


uint8_t*
bootstrap_alloc_checked(
	alloc_t size,
	int zero
	)
{
	uint8_t* ptr = alloc_bootstrap_alloc(size, zero);

	if(!size)
	{
		assert_null(ptr);
		return NULL;
	}

	assert_ptr(ptr, size);

	alloc_t alignment = bootstrap_expected_alignment(size);
	assert_eq((alloc_t) ptr % alignment, 0);

	if(zero)
	{
		bootstrap_assert_zeroed(ptr, size);
	}

	bootstrap_assert_non_overlapping(ptr, size, -1);
	bootstrap_add_live(ptr, size);

	return ptr;
}


void
bootstrap_free_checked(
	uint8_t* ptr,
	alloc_t size
	)
{
	if(!ptr)
	{
		assert_eq(size, 0);
		alloc_bootstrap_free(NULL, 0);
		return;
	}

	int idx = bootstrap_find_live_idx(ptr);
	assert_neq(idx, -1);
	assert_eq(bootstrap_live[idx].size, size);

	alloc_bootstrap_free(ptr, size);
	bootstrap_remove_live_idx(idx);
}


uint8_t*
bootstrap_realloc_checked(
	uint8_t* ptr,
	alloc_t old_size,
	alloc_t new_size,
	int zero
	)
{
	if(!ptr)
	{
		assert_eq(old_size, 0);
		return bootstrap_alloc_checked(new_size, zero);
	}

	int idx = bootstrap_find_live_idx(ptr);
	assert_neq(idx, -1);
	assert_eq(bootstrap_live[idx].size, old_size);

	uint8_t* new_ptr = alloc_bootstrap_realloc(ptr, old_size, new_size, zero);

	if(!new_size)
	{
		assert_null(new_ptr);
		bootstrap_remove_live_idx(idx);
		return NULL;
	}

	assert_ptr(new_ptr, new_size);

	alloc_t alignment = bootstrap_expected_alignment(new_size);
	assert_eq((alloc_t) new_ptr % alignment, 0);

	if(zero && new_size > old_size)
	{
		bootstrap_assert_zeroed(new_ptr + old_size, new_size - old_size);
	}

	bootstrap_live[idx].ptr = new_ptr;
	bootstrap_live[idx].size = new_size;

	bootstrap_assert_non_overlapping(new_ptr, new_size, idx);

	return new_ptr;
}


void
bootstrap_free_all_live(
	void
	)
{
	while(bootstrap_live_count)
	{
		bootstrap_live_alloc_t live = bootstrap_live[bootstrap_live_count - 1];
		bootstrap_free_checked(live.ptr, live.size);
	}
}


void attr_test_fn
test_normal_pass__bootstrap_alloc_zero(
	void
	)
{
	uint8_t* ptr = bootstrap_alloc_checked(0, 0);
	assert_null(ptr);
}


void attr_test_fn
test_normal_pass__bootstrap_alloc_aligns(
	void
	)
{
	const alloc_t sizes[] = {1, 2, 3, 7, 8, 15, 16, 8191, 8192, 8193};

	for(uint32_t i = 0; i < MACRO_ARRAY_LEN(sizes); ++i)
	{
		uint8_t* ptr = bootstrap_alloc_checked(sizes[i], 0);
		bootstrap_free_checked(ptr, sizes[i]);
	}
}


void attr_test_fn
test_normal_pass__bootstrap_alloc_zeroed(
	void
	)
{
	uint8_t* ptr = bootstrap_alloc_checked(257, 1);
	bootstrap_assert_zeroed(ptr, 257);

	bootstrap_free_checked(ptr, 257);
}


void attr_test_fn
test_normal_pass__bootstrap_alloc_nonoverlap(
	void
	)
{
	const alloc_t sizes[] = {13, 97, 1025, 4097, 33, 2049, 65, 777};
	uint8_t* ptrs[MACRO_ARRAY_LEN(sizes)];

	for(uint32_t i = 0; i < MACRO_ARRAY_LEN(sizes); ++i)
	{
		ptrs[i] = bootstrap_alloc_checked(sizes[i], i & 1);
	}

	for(uint32_t i = 0; i < MACRO_ARRAY_LEN(sizes); ++i)
	{
		bootstrap_free_checked(ptrs[i], sizes[i]);
	}
}


void attr_test_fn
test_normal_pass__bootstrap_alloc_oom_null(
	void
	)
{
	const alloc_t chunk = 1024;
	uint8_t* ptrs[BOOTSTRAP_MAX_LIVE];
	uint32_t used = 0;

	while(used < BOOTSTRAP_MAX_LIVE)
	{
		uint8_t* ptr = alloc_bootstrap_alloc(chunk, 0);
		if(!ptr)
		{
			break;
		}

		alloc_t alignment = bootstrap_expected_alignment(chunk);
		assert_eq((alloc_t) ptr % alignment, 0);

		bootstrap_assert_non_overlapping(ptr, chunk, -1);
		bootstrap_add_live(ptr, chunk);
		ptrs[used++] = ptr;
	}

	assert_gt(used, 0);
	assert_null(alloc_bootstrap_alloc(chunk, 0));

	for(uint32_t i = 0; i < used; ++i)
	{
		bootstrap_free_checked(ptrs[i], chunk);
	}
}


void attr_test_fn
test_normal_pass__bootstrap_realloc_null(
	void
	)
{
	uint8_t* ptr = bootstrap_realloc_checked(NULL, 0, 73, 0);
	bootstrap_free_checked(ptr, 73);
}


void attr_test_fn
test_normal_pass__bootstrap_realloc_to_zero(
	void
	)
{
	uint8_t* ptr = bootstrap_alloc_checked(111, 0);
	ptr = bootstrap_realloc_checked(ptr, 111, 0, 0);
	assert_null(ptr);
}


void attr_test_fn
test_normal_pass__bootstrap_realloc_grow(
	void
	)
{
	uint8_t* ptr = bootstrap_alloc_checked(64, 0);

	for(uint32_t i = 0; i < 64; ++i)
	{
		ptr[i] = (uint8_t) (i ^ 0x5A);
	}

	uint8_t* grown = bootstrap_realloc_checked(ptr, 64, 257, 1);

	for(uint32_t i = 0; i < 64; ++i)
	{
		assert_eq(grown[i], (uint8_t) (i ^ 0x5A));
	}

	bootstrap_assert_zeroed(grown + 64, 193);
	bootstrap_free_checked(grown, 257);
}


void attr_test_fn
test_normal_pass__bootstrap_realloc_shrink(
	void
	)
{
	uint8_t* ptr = bootstrap_alloc_checked(257, 0);

	for(uint32_t i = 0; i < 257; ++i)
	{
		ptr[i] = (uint8_t) ((i * 17) ^ 0xA5);
	}

	uint8_t* shrunk = bootstrap_realloc_checked(ptr, 257, 96, 0);

	for(uint32_t i = 0; i < 96; ++i)
	{
		assert_eq(shrunk[i], (uint8_t) ((i * 17) ^ 0xA5));
	}

	bootstrap_free_checked(shrunk, 96);
}


void attr_test_fn
test_normal_pass__bootstrap_realloc_align(
	void
	)
{
	uint8_t* ptr = bootstrap_alloc_checked(3, 0);
	uint8_t* ptr2 = bootstrap_realloc_checked(ptr, 3, 8192, 0);
	bootstrap_free_checked(ptr2, 8192);
}


void attr_test_fn
test_normal_pass__bootstrap_free_null(
	void
	)
{
	bootstrap_free_checked(NULL, 0);
}


void attr_test_fn
test_normal_pass__bootstrap_free_reverse(
	void
	)
{
	const alloc_t sizes[] = {31, 67, 129, 2047, 9};
	uint8_t* ptrs[MACRO_ARRAY_LEN(sizes)];

	for(uint32_t i = 0; i < MACRO_ARRAY_LEN(sizes); ++i)
	{
		ptrs[i] = bootstrap_alloc_checked(sizes[i], 0);
	}

	for(int i = (int) MACRO_ARRAY_LEN(sizes) - 1; i >= 0; --i)
	{
		bootstrap_free_checked(ptrs[i], sizes[i]);
	}
}


void attr_test_fn
test_normal_pass__bootstrap_free_middle(
	void
	)
{
	uint8_t* a = bootstrap_alloc_checked(128, 0);
	uint8_t* b = bootstrap_alloc_checked(256, 0);
	uint8_t* c = bootstrap_alloc_checked(512, 0);

	bootstrap_free_checked(b, 256);

	uint8_t* d = bootstrap_alloc_checked(200, 1);
	bootstrap_assert_zeroed(d, 200);

	bootstrap_free_checked(a, 128);
	bootstrap_free_checked(c, 512);
	bootstrap_free_checked(d, 200);
}


void attr_test_fn
test_normal_pass__bootstrap_cleanup_live(
	void
	)
{
	uint8_t* a = bootstrap_alloc_checked(64, 0);
	uint8_t* b = bootstrap_alloc_checked(96, 1);
	assert_not_null(a);
	assert_not_null(b);

	bootstrap_free_all_live();
	assert_eq(bootstrap_live_count, 0);
}


void attr_test_fn
test_normal_pass__bootstrap_realloc_same(
	void
	)
{
	uint8_t* ptr = bootstrap_alloc_checked(96, 0);
	for(uint32_t i = 0; i < 96; ++i)
	{
		ptr[i] = (uint8_t) (i ^ 0x31);
	}

	uint8_t* ptr2 = bootstrap_realloc_checked(ptr, 96, 96, 0);
	assert_eq(ptr2, ptr);

	for(uint32_t i = 0; i < 96; ++i)
	{
		assert_eq(ptr2[i], (uint8_t) (i ^ 0x31));
	}

	bootstrap_free_checked(ptr2, 96);
}


void attr_test_fn
test_normal_pass__bootstrap_realloc_inp(
	void
	)
{
	/*
	 * Keep realloc within the same alignment class (512-byte alignment for
	 * both sizes) so in-place growth is expected when the next block is free.
	 */
	uint8_t* a = bootstrap_alloc_checked(300, 0);
	uint8_t* b = bootstrap_alloc_checked(512, 0);

	for(uint32_t i = 0; i < 300; ++i)
	{
		a[i] = (uint8_t) (0x80 + i);
	}

	bootstrap_free_checked(b, 512);

	uint8_t* a2 = bootstrap_realloc_checked(a, 300, 350, 1);
	assert_eq(a2, a);

	for(uint32_t i = 0; i < 300; ++i)
	{
		assert_eq(a2[i], (uint8_t) (0x80 + i));
	}

	bootstrap_assert_zeroed(a2 + 300, 50);
	bootstrap_free_checked(a2, 350);
}


void attr_test_fn
test_normal_pass__bootstrap_realloc_move(
	void
	)
{
	uint8_t* a = bootstrap_alloc_checked(64, 0);
	uint8_t* b = bootstrap_alloc_checked(64, 0);

	for(uint32_t i = 0; i < 64; ++i)
	{
		a[i] = (uint8_t) (i ^ 0xCC);
	}

	uint8_t* a2 = bootstrap_realloc_checked(a, 64, 2048, 0);
	assert_neq(a2, a);

	for(uint32_t i = 0; i < 64; ++i)
	{
		assert_eq(a2[i], (uint8_t) (i ^ 0xCC));
	}

	bootstrap_free_checked(b, 64);
	bootstrap_free_checked(a2, 2048);
}


void attr_test_fn
test_normal_pass__bootstrap_realloc_tail(
	void
	)
{
	uint8_t* a = bootstrap_alloc_checked(1024, 0);
	for(uint32_t i = 0; i < 1024; ++i)
	{
		a[i] = (uint8_t) (i * 3);
	}

	uint8_t* a2 = bootstrap_realloc_checked(a, 1024, 128, 0);
	assert_eq(a2, a);

	for(uint32_t i = 0; i < 128; ++i)
	{
		assert_eq(a2[i], (uint8_t) (i * 3));
	}

	uint8_t* b = bootstrap_alloc_checked(700, 1);
	bootstrap_assert_zeroed(b, 700);

	bootstrap_free_checked(a2, 128);
	bootstrap_free_checked(b, 700);
}


void attr_test_fn
test_normal_pass__bootstrap_reuse_mid(
	void
	)
{
	uint8_t* a = bootstrap_alloc_checked(200, 0);
	uint8_t* b = bootstrap_alloc_checked(300, 0);
	uint8_t* c = bootstrap_alloc_checked(400, 0);

	bootstrap_free_checked(b, 300);

	uint8_t* d = bootstrap_alloc_checked(300, 0);
	assert_eq(d, b);

	bootstrap_free_checked(a, 200);
	bootstrap_free_checked(c, 400);
	bootstrap_free_checked(d, 300);
}


void attr_test_fn
test_normal_fail__bootstrap_free_null_sz(
	void
	)
{
	alloc_bootstrap_free(NULL, 1);
}


void attr_test_fn
test_normal_fail__bootstrap_free_bad_ptr(
	void
	)
{
	alloc_bootstrap_free(TEST_PTR, 1);
}


void attr_test_fn
test_normal_fail__bootstrap_free_bad_size(
	void
	)
{
	uint8_t* ptr = bootstrap_alloc_checked(64, 0);
	alloc_bootstrap_free(ptr, 65);
}


void attr_test_fn
test_normal_fail__bootstrap_free_twice(
	void
	)
{
	uint8_t* ptr = bootstrap_alloc_checked(32, 0);
	alloc_bootstrap_free(ptr, 32);
	alloc_bootstrap_free(ptr, 32);
}


void attr_test_fn
test_normal_fail__bootstrap_realloc_badold(
	void
	)
{
	uint8_t* ptr = bootstrap_alloc_checked(48, 0);
	alloc_bootstrap_realloc(ptr, 49, 64, 0);
}


void attr_test_fn
test_normal_fail__bootstrap_realloc_nullold(
	void
	)
{
	alloc_bootstrap_realloc(NULL, 1, 64, 0);
}
