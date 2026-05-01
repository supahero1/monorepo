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
#include <shared/sync.h>
#include <shared/debug.h>
#include <shared/macro.h>
#include <shared/atomic.h>
#include <shared/threads.h>
#include <shared/alloc/base.h>
#include <shared/alloc/huge.h>
#include <shared/alloc/test.h>
#include <shared/alloc/btree.h>
#include <shared/alloc/types.h>
#include <shared/alloc/consts.h>
#include <shared/alloc/platform.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ALLOC_TEST_MAX_LIVE 65536
#define ALLOC_TEST_STRESS_SLOTS 512
#define ALLOC_TEST_STRESS_OPS 12000


typedef struct alloc_live
{
	uint8_t* ptr;
	alloc_t size;
	alloc_t alignment;
}
alloc_live_t;


alloc_live_t alloc_live[ALLOC_TEST_MAX_LIVE];
uint32_t alloc_live_count;


typedef struct alloc_xthread_ctx
{
	void* ptr;
	alloc_t size;
	alloc_t alignment;
	sync_mtx_t mtx;
	sync_cond_t cv;
	int phase;
}
alloc_xthread_ctx_t;


typedef struct alloc_worker_sync
{
	sync_mtx_t mtx;
	sync_cond_t cv;
	int phase;
	uint16_t numa;
	alloc_t unique_arenas;
}
alloc_worker_sync_t;


typedef struct alloc_pool_reuse_ctx
{
	alloc_arena_header_t* arena;
	uint32_t slot;
	uint16_t numa;
	alloc_t size;
}
alloc_pool_reuse_ctx_t;


typedef struct alloc_slot_alloc
{
	void* ptr;
	uint32_t slot;
}
alloc_slot_alloc_t;


typedef struct alloc_pool_reuse_worker_ctx
{
	alloc_arena_header_t* arena;
	uint32_t slot;
	uint16_t numa;
	alloc_t size;
}
alloc_pool_reuse_worker_ctx_t;

typedef struct alloc_retained_worker_ctx
{
	alloc_t size;
}
alloc_retained_worker_ctx_t;


alloc_t
alloc_test_effective_alignment(
	alloc_t size,
	alloc_t requested_alignment
	)
{
	assert_neq(size, 0);

	alloc_t alignment = requested_alignment;
	if(!alignment)
	{
		alignment = MACRO_NEXT_OR_EQUAL_POWER_OF_2(size);
	}

	alignment = MACRO_MIN(alignment, alloc_consts.page.size);
	assert_true(MACRO_IS_POWER_OF_2(alignment));
	return alignment;
}


bool
alloc_test_ranges_overlap(
	const void* a_ptr,
	alloc_t a_size,
	const void* b_ptr,
	alloc_t b_size
	)
{
	const uint8_t* a0 = a_ptr;
	const uint8_t* a1 = a0 + a_size;
	const uint8_t* b0 = b_ptr;
	const uint8_t* b1 = b0 + b_size;

	return a0 < b1 && b0 < a1;
}


int
alloc_test_find_live_idx(
	const void* ptr
	)
{
	for(uint32_t i = 0; i < alloc_live_count; ++i)
	{
		if(alloc_live[i].ptr == ptr)
		{
			return i;
		}
	}

	return -1;
}


void
alloc_test_assert_zero(
	const uint8_t* ptr,
	alloc_t size
	)
{
	for(alloc_t i = 0; i < size; ++i)
	{
		assert_eq(ptr[i], 0);
	}
}


void
alloc_test_assert_non_overlapping(
	const void* ptr,
	alloc_t size,
	int skip_idx
	)
{
	for(uint32_t i = 0; i < alloc_live_count; ++i)
	{
		if((int) i == skip_idx)
		{
			continue;
		}

		assert_false(alloc_test_ranges_overlap(ptr, size, alloc_live[i].ptr, alloc_live[i].size));
	}
}


void
alloc_test_add_live(
	void* ptr,
	alloc_t size,
	alloc_t alignment
	)
{
	assert_lt(alloc_live_count, ALLOC_TEST_MAX_LIVE);
	assert_eq(alloc_test_find_live_idx(ptr), -1);

	alloc_live[alloc_live_count++] =
	(alloc_live_t)
	{
		.ptr = ptr,
		.size = size,
		.alignment = alignment
	};
}


void
alloc_test_remove_live_idx(
	uint32_t idx
	)
{
	assert_lt(idx, alloc_live_count);
	alloc_live[idx] = alloc_live[alloc_live_count - 1];
	--alloc_live_count;
}


void*
alloc_test_alloc_checked(
	alloc_t size,
	int zero
	)
{
	void* ptr = alloc_alloc_e(size, zero);

	if(!size)
	{
		assert_null(ptr);
		return NULL;
	}

	assert_ptr(ptr, size);

	alloc_t alignment = alloc_test_effective_alignment(size, 0);
	assert_eq((alloc_t) ptr & (alignment - 1), 0);

	if(zero)
	{
		alloc_test_assert_zero(ptr, size);
	}

	alloc_test_assert_non_overlapping(ptr, size, -1);
	alloc_test_add_live(ptr, size, alignment);
	return ptr;
}


void*
alloc_test_alloc_aligned_checked(
	alloc_t size,
	alloc_t alignment,
	int zero
	)
{
	void* ptr = alloc_alloc_aligned_e(size, alignment, zero);

	if(!size)
	{
		assert_null(ptr);
		return NULL;
	}

	assert_ptr(ptr, size);

	alloc_t effective_alignment = alloc_test_effective_alignment(size, alignment);
	assert_eq((alloc_t) ptr & (effective_alignment - 1), 0);

	if(zero)
	{
		alloc_test_assert_zero(ptr, size);
	}

	alloc_test_assert_non_overlapping(ptr, size, -1);
	alloc_test_add_live(ptr, size, effective_alignment);
	return ptr;
}


void
alloc_test_free_checked(
	void* ptr,
	alloc_t size
	)
{
	if(!ptr)
	{
		assert_eq(size, 0);
		alloc_free_e(NULL, 0);
		return;
	}

	int idx = alloc_test_find_live_idx(ptr);
	assert_neq(idx, -1);
	assert_eq(alloc_live[idx].size, size);

	alloc_free_e(ptr, size);
	alloc_test_remove_live_idx((uint32_t) idx);
}


void
alloc_test_free_aligned_checked(
	void* ptr,
	alloc_t size,
	alloc_t alignment
	)
{
	if(!ptr)
	{
		assert_eq(size, 0);
		alloc_free_aligned_e(NULL, 0, alignment);
		return;
	}

	int idx = alloc_test_find_live_idx(ptr);
	assert_neq(idx, -1);
	assert_eq(alloc_live[idx].size, size);

	alloc_free_aligned_e(ptr, size, alignment);
	alloc_test_remove_live_idx((uint32_t) idx);
}


void*
alloc_test_realloc_checked(
	void* ptr,
	alloc_t old_size,
	alloc_t new_size,
	int zero
	)
{
	if(!ptr)
	{
		assert_eq(old_size, 0);
		return alloc_test_alloc_checked(new_size, zero);
	}

	int idx = alloc_test_find_live_idx(ptr);
	assert_neq(idx, -1);
	assert_eq(alloc_live[idx].size, old_size);

	void* new_ptr = alloc_realloc_e(ptr, old_size, new_size, zero);

	if(!new_size)
	{
		assert_null(new_ptr);
		alloc_test_remove_live_idx((uint32_t) idx);
		return NULL;
	}

	assert_ptr(new_ptr, new_size);
	alloc_t alignment = alloc_test_effective_alignment(new_size, 0);
	assert_eq((alloc_t) new_ptr & (alignment - 1), 0);

	if(zero && new_size > old_size)
	{
		alloc_test_assert_zero((uint8_t*) new_ptr + old_size, new_size - old_size);
	}

	alloc_live[idx].ptr = new_ptr;
	alloc_live[idx].size = new_size;
	alloc_live[idx].alignment = alignment;

	alloc_test_assert_non_overlapping(new_ptr, new_size, idx);
	return new_ptr;
}


void*
alloc_test_realloc_aligned_checked(
	void* ptr,
	alloc_t old_size,
	alloc_t old_alignment,
	alloc_t new_size,
	alloc_t new_alignment,
	int zero
	)
{
	if(!ptr)
	{
		assert_eq(old_size, 0);
		return alloc_test_alloc_aligned_checked(new_size, new_alignment, zero);
	}

	int idx = alloc_test_find_live_idx(ptr);
	assert_neq(idx, -1);
	assert_eq(alloc_live[idx].size, old_size);

	void* new_ptr = alloc_realloc_aligned_e(ptr, old_size, old_alignment, new_size, new_alignment, zero);

	if(!new_size)
	{
		assert_null(new_ptr);
		alloc_test_remove_live_idx((uint32_t) idx);
		return NULL;
	}

	assert_ptr(new_ptr, new_size);
	alloc_t effective_alignment = alloc_test_effective_alignment(new_size, new_alignment);
	assert_eq((alloc_t) new_ptr & (effective_alignment - 1), 0);

	if(zero && new_size > old_size)
	{
		alloc_test_assert_zero((uint8_t*) new_ptr + old_size, new_size - old_size);
	}

	alloc_live[idx].ptr = new_ptr;
	alloc_live[idx].size = new_size;
	alloc_live[idx].alignment = effective_alignment;

	alloc_test_assert_non_overlapping(new_ptr, new_size, idx);
	return new_ptr;
}


void
alloc_test_free_all_live(
	void
	)
{
	while(alloc_live_count)
	{
		alloc_live_t live = alloc_live[alloc_live_count - 1];
		alloc_test_free_checked(live.ptr, live.size);
	}
}


alloc_t
alloc_test_size_pattern(
	alloc_t i,
	int allow_virtual
	)
{
	alloc_t max_small = alloc_consts.slab.max_nonvirtual_size;
	alloc_t page = alloc_consts.page.size;

	switch(i % 8)
	{
	case 0: return 1;
	case 1: return 24;
	case 2: return 511;
	case 3: return MACRO_MIN((alloc_t) 512, max_small);
	case 4: return MACRO_MIN((alloc_t) 4095, max_small);
	case 5:
		if(allow_virtual)
		{
			return max_small + page;
		}
		return max_small;
	case 6: return MACRO_MIN((alloc_t) 97, max_small);
	default: return MACRO_MIN((alloc_t) 2048, max_small);
	}
}


alloc_t
alloc_test_alignment_pattern(
	alloc_t i
	)
{
	alloc_t shift = i % (alloc_consts.page.shift + 1);
	return (alloc_t) 1 << shift;
}


alloc_t
alloc_test_huge_hash(
	alloc_t size
	)
{
	return ((uint64_t) 0x9e3779b97f4a7c15 * size) >> (64 - alloc_consts.huge.slots_shift);
}


alloc_t
alloc_test_huge_time_count(
	const alloc_numa_local_data_t* local_data
	)
{
	alloc_t count = 0;
	for(alloc_huge_node_t* n = local_data->huge.time_head; n; n = n->time_next)
	{
		++count;
	}
	return count;
}


alloc_t
alloc_test_huge_bucket_count(
	const alloc_numa_local_data_t* local_data,
	alloc_t size
	)
{
	alloc_t count = 0;
	for(alloc_huge_node_t* n = local_data->huge.ht[alloc_test_huge_hash(size)]; n; n = n->next)
	{
		if(n->size == size)
		{
			++count;
		}
	}
	return count;
}

alloc_arena_header_t*
alloc_test_ptr_arena(
	const void* ptr
	)
{
	return alloc_test_get_arena_from_ptr(ptr);
}


alloc_slab_header_t*
alloc_test_ptr_slab_base(
	void* arena
	)
{
	return arena + alloc_consts.arena.slab_indexable_offset;
}


alloc_t
alloc_test_ptr_base_idx(
	const void* ptr
	)
{
	alloc_arena_header_t* arena = alloc_test_ptr_arena(ptr);
	return ((const uint8_t*) ptr - (const uint8_t*) arena->data) >> alloc_consts.slab.small_shift;
}


alloc_t
alloc_test_ptr_slab_start_idx(
	const void* ptr
	)
{
	alloc_arena_header_t* arena = alloc_test_ptr_arena(ptr);
	alloc_slab_header_t* slab_base = alloc_test_ptr_slab_base(arena);
	alloc_t slab_idx = alloc_test_ptr_base_idx(ptr) + alloc_consts.arena.prefix_slabs;

	for(alloc_t start = slab_idx + 1; start > alloc_consts.arena.prefix_slabs; --start)
	{
		alloc_t idx = start - 1;
		alloc_slab_header_t* slab = slab_base + idx;

		if(slab->idx != idx + 1 || !slab->count || !slab->handle)
		{
			continue;
		}

		alloc_t span = alloc_consts.slab.class_span[slab->handle->slab_class];
		if(slab_idx < idx + span)
		{
			return idx;
		}
	}

	assert_unreachable();
	return 0;
}


alloc_slab_header_t*
alloc_test_ptr_slab_header(
	const void* ptr
	)
{
	alloc_arena_header_t* arena = alloc_test_ptr_arena(ptr);
	alloc_slab_header_t* slab_base = alloc_test_ptr_slab_base(arena);
	return slab_base + alloc_test_ptr_slab_start_idx(ptr);
}

alloc_t
alloc_test_size_for_class(
	alloc_t class_idx
	)
{
	return alloc_test_find_size_for_class(class_idx);
}


alloc_t
alloc_test_find_huge_collision_size(
	alloc_t size
	)
{
	alloc_t bucket = alloc_test_huge_hash(size);

	for(alloc_t delta = alloc_consts.page.size; delta < alloc_consts.page.size * 8192; delta += alloc_consts.page.size)
	{
		alloc_t candidate = size + delta;
		if(candidate <= alloc_consts.slab.max_nonvirtual_size)
		{
			continue;
		}

		if(alloc_test_huge_hash(candidate) == bucket)
		{
			return candidate;
		}
	}

	assert_unreachable();
	return 0;
}


alloc_numa_local_data_t*
alloc_test_local_data_current_numa(
	uint16_t* out_numa
	)
{
	uint16_t numa;
	alloc_get_cpu_numa_node(NULL, &numa);
	if(out_numa)
	{
		*out_numa = numa;
	}
	return alloc_get_numa_local_data(numa);
}


alloc_t
alloc_test_global_pool_count(
	uint16_t numa
	)
{
	alloc_numa_local_data_t* local_data = alloc_get_numa_local_data(numa);
	sync_mtx_lock(&local_data->arena.mtx);
		alloc_t count = local_data->arena.high_watermark - local_data->arena.used_slots;
	sync_mtx_unlock(&local_data->arena.mtx);
	return count;
}


alloc_t
alloc_test_count_handle_empty_slabs(
	const alloc_handle_t* handle
	)
{
	alloc_t empty = 0;
	for(const alloc_slab_header_t* slab = handle->slab; slab; slab = slab->next)
	{
		if(!slab->count)
		{
			++empty;
		}
	}
	return empty;
}


alloc_t
alloc_test_size_for_handle_idx(
	alloc_handle_idx_t idx
	)
{
	assert_lt(idx, alloc_consts.handles.count);

	for(alloc_t size = 1; size <= alloc_consts.slab.max_nonvirtual_size; ++size)
	{
		if(alloc_test_default_idx_fn(size) == idx)
		{
			return size;
		}
	}

	assert_unreachable();
	return 0;
}


alloc_t
alloc_test_size_one_per_arena(
	void
	)
{
	alloc_t best_size = 0;
	alloc_t best_per_arena = (alloc_t) -1;

	for(alloc_handle_idx_t idx = 0; idx < alloc_consts.handles.count; ++idx)
	{
		const alloc_handle_t* handle = alloc_consts.handles.templates + idx;
		alloc_t span = alloc_consts.slab.class_span[handle->slab_class];
		alloc_t slabs = alloc_consts.arena.slab_count / span;
		alloc_t per_arena = slabs * handle->alloc_limit;

		if(per_arena == 1)
		{
			return alloc_test_size_for_handle_idx(idx);
		}

		if(per_arena < best_per_arena)
		{
			best_per_arena = per_arena;
			best_size = alloc_test_size_for_handle_idx(idx);
		}
	}

	assert_neq(best_size, 0);
	return best_size;
}


void
alloc_test_worker_two_arena_pause(
	void* arg
	)
{
	alloc_worker_sync_t* sync = arg;

	alloc_t size = alloc_test_size_for_class(0);
	const alloc_handle_t* h = alloc_test_get_handle(size);
	alloc_t class_idx = h->slab_class;
	alloc_t slabs_per_arena = alloc_consts.arena.slab_count / alloc_consts.slab.class_span[class_idx];
	alloc_t per_arena = slabs_per_arena * h->alloc_limit;
	alloc_t n = per_arena + h->alloc_limit;

	void** ptrs = malloc(sizeof(*ptrs) * n);
	assert_not_null(ptrs);
	alloc_arena_header_t* first = NULL;
	alloc_arena_header_t* second = NULL;
	alloc_t unique = 0;

	for(alloc_t i = 0; i < n; ++i)
	{
		ptrs[i] = alloc_malloc_e(size);
		assert_not_null(ptrs[i]);

		alloc_arena_header_t* arena = alloc_test_get_arena_from_ptr(ptrs[i]);
		if(!first)
		{
			first = arena;
			++unique;
		}
		else if(arena != first && !second)
		{
			second = arena;
			++unique;
		}
	}

	for(alloc_t i = 0; i < n; ++i)
	{
		alloc_free_e(ptrs[i], size);
	}

	free(ptrs);

	uint16_t numa;
	alloc_get_cpu_numa_node(NULL, &numa);

	sync_mtx_lock(&sync->mtx);
		sync->unique_arenas = unique;
		sync->numa = numa;
		sync->phase = 1;
		sync_cond_wake(&sync->cv);
		while(sync->phase != 2)
		{
			sync_cond_wait(&sync->cv, &sync->mtx);
		}
	sync_mtx_unlock(&sync->mtx);
}


void
alloc_test_worker_xthread_free(
	void* arg
	)
{
	alloc_xthread_ctx_t* ctx = arg;

	sync_mtx_lock(&ctx->mtx);
		while(ctx->phase != 1)
		{
			sync_cond_wait(&ctx->cv, &ctx->mtx);
		}
	sync_mtx_unlock(&ctx->mtx);

	alloc_free_aligned_e(ctx->ptr, ctx->size, ctx->alignment);

	sync_mtx_lock(&ctx->mtx);
		ctx->phase = 2;
		sync_cond_wake(&ctx->cv);
	sync_mtx_unlock(&ctx->mtx);
}


uint32_t
alloc_test_ptr_slot_idx(
	const void* ptr,
	const alloc_numa_local_data_t* local_data
	)
{
	alloc_arena_header_t* arena = alloc_test_get_arena_from_ptr(ptr);
	assert_not_null(arena);
	assert_eq(arena->numa, local_data->numa);
	assert_lt(arena->pool_idx, local_data->arena.slot_count);
	return arena->pool_idx;
}


void
alloc_test_worker_pool_reuse_fill(
	void* arg
	)
{
	alloc_pool_reuse_worker_ctx_t* ctx = arg;
	assert_not_null(ctx);

	uint16_t numa;
	alloc_get_cpu_numa_node(NULL, &numa);
	alloc_numa_local_data_t* local_data = alloc_get_numa_local_data(numa);
	assert_not_null(local_data);

	const alloc_handle_t* handle = alloc_test_get_handle(ctx->size);
	alloc_t class_idx = handle->slab_class;
	alloc_t slabs_per_arena = alloc_consts.arena.slab_count / alloc_consts.slab.class_span[class_idx];
	alloc_t per_arena = slabs_per_arena * handle->alloc_limit;
	assert_gt(per_arena, 0);

	void** ptrs = malloc(sizeof(*ptrs) * per_arena);
	assert_not_null(ptrs);

	for(alloc_t i = 0; i < per_arena; ++i)
	{
		ptrs[i] = alloc_malloc_e(ctx->size);
		assert_not_null(ptrs[i]);
		memset(ptrs[i], 0xA5, ctx->size);
	}

	ctx->arena = alloc_test_get_arena_from_ptr(ptrs[0]);
	ctx->slot = alloc_test_ptr_slot_idx(ptrs[0], local_data);
	ctx->numa = numa;

	for(alloc_t i = 0; i < per_arena; ++i)
	{
		alloc_free_e(ptrs[i], ctx->size);
	}

	free(ptrs);
}


void
alloc_test_worker_retained_then_exit(
	void* arg
	)
{
	alloc_retained_worker_ctx_t* ctx = arg;
	alloc_t size = ctx->size;

	const alloc_handle_t* handle = alloc_test_get_handle(size);
	alloc_t class_idx = handle->slab_class;
	alloc_t slabs_per_arena = alloc_consts.arena.slab_count / alloc_consts.slab.class_span[class_idx];
	alloc_t per_arena = slabs_per_arena * handle->alloc_limit;
	alloc_t n = per_arena + handle->alloc_limit;

	void** ptrs = malloc(sizeof(*ptrs) * n);
	assert_not_null(ptrs);

	for(alloc_t i = 0; i < n; ++i)
	{
		ptrs[i] = alloc_malloc_e(size);
		assert_not_null(ptrs[i]);
	}

	for(alloc_t i = 0; i < n; ++i)
	{
		alloc_free_e(ptrs[i], size);
	}

	free(ptrs);
}


void attr_test_fn
test_priority_pass__alloc_small_matrix(
	void
	)
{
	void* ptrs[8];

	for(alloc_t size = 1; size <= alloc_consts.slab.max_nonvirtual_size; ++size)
	{
		for(alloc_t i = 0; i < MACRO_ARRAY_LEN(ptrs); ++i)
		{
			ptrs[i] = alloc_test_alloc_checked(size, 0);
			memset(ptrs[i], size + i, size);
		}

		for(alloc_t i = 0; i < MACRO_ARRAY_LEN(ptrs); ++i)
		{
			alloc_test_free_checked(ptrs[i], size);
		}
	}
}


void attr_test_fn
test_priority_pass__alloc_default_overlap(
	void
	)
{
	for(alloc_t i = 0; i < 2048; ++i)
	{
		alloc_t size = alloc_test_size_pattern(i, 1);
		alloc_test_alloc_checked(size, 0);
	}

	alloc_test_free_all_live();
}


void attr_test_fn
test_priority_pass__alloc_aligned_overlap(
	void
	)
{
	for(alloc_t i = 0; i < 1536; ++i)
	{
		alloc_t size = alloc_test_size_pattern(i + 17, 1);
		alloc_t alignment = alloc_test_alignment_pattern(i);
		alloc_test_alloc_aligned_checked(size, alignment, 0);
	}

	for(uint32_t i = 0; i < alloc_live_count; ++i)
	{
		alloc_free_aligned_e(alloc_live[i].ptr, alloc_live[i].size, alloc_live[i].alignment);
	}
	alloc_live_count = 0;
}


void attr_test_fn
test_priority_pass__alloc_aligned_churn(
	void
	)
{
	alloc_live_t slots[ALLOC_TEST_STRESS_SLOTS] = {0};
	uint32_t seed = 0xC0FFEEu;

	for(alloc_t op = 0; op < ALLOC_TEST_STRESS_OPS; ++op)
	{
		alloc_t slot = rand_r(&seed) % ALLOC_TEST_STRESS_SLOTS;
		alloc_live_t* it = &slots[slot];

		if(!it->ptr)
		{
			alloc_t size = alloc_test_size_pattern(rand_r(&seed), 1);
			alloc_t alignment = alloc_test_alignment_pattern(rand_r(&seed));
			it->ptr = alloc_test_alloc_aligned_checked(size, alignment, 0);
			it->size = size;
			it->alignment = alloc_test_effective_alignment(size, alignment);
		}
		else
		{
			uint32_t action = rand_r(&seed) % 100;
			if(action < 30)
			{
				alloc_test_free_aligned_checked(it->ptr, it->size, it->alignment);
				*it = (alloc_live_t){0};
			}
			else
			{
				alloc_t new_size = alloc_test_size_pattern(rand_r(&seed), 1);
				alloc_t new_alignment = alloc_test_alignment_pattern(rand_r(&seed));

				void* new_ptr = alloc_test_realloc_aligned_checked(
					it->ptr, it->size, it->alignment, new_size, new_alignment, 0);
				it->ptr = new_ptr;
				it->size = new_size;
				it->alignment = alloc_test_effective_alignment(new_size, new_alignment);
			}
		}
	}

	for(alloc_t i = 0; i < ALLOC_TEST_STRESS_SLOTS; ++i)
	{
		if(!slots[i].ptr)
		{
			continue;
		}

		alloc_test_free_aligned_checked(slots[i].ptr, slots[i].size, slots[i].alignment);
	}
}


void attr_test_fn
test_priority_pass__alloc_aligned_api_re(
	void
	)
{
	void* ptr = alloc_test_alloc_aligned_checked(24, 8, 0);
	memset(ptr, 0xA5, 24);

	void* ptr2 = alloc_test_realloc_aligned_checked(ptr, 24, 8, 48, 32, 0);
	for(alloc_t i = 0; i < 24; ++i)
	{
		assert_eq(((uint8_t*) ptr2)[i], 0xA5);
	}

	alloc_test_free_aligned_checked(ptr2, 48, 32);
}


void attr_test_fn
test_priority_pass__alloc_xthread_aligned(
	void
	)
{
	void* ptr = alloc_test_alloc_aligned_checked(100, 16, 0);
	memset(ptr, 0xA5, 100);

	alloc_xthread_ctx_t ctx =
	(alloc_xthread_ctx_t)
	{
		.ptr = ptr,
		.size = 100,
		.alignment = 16,
		.phase = 0
	};

	sync_mtx_init(&ctx.mtx);
	sync_cond_init(&ctx.cv);

	thread_t th;
	thread_init(&th, (thread_data_t){ alloc_test_worker_xthread_free, &ctx });

	sync_mtx_lock(&ctx.mtx);
		ctx.phase = 1;
		sync_cond_wake(&ctx.cv);
	sync_mtx_unlock(&ctx.mtx);

	sync_mtx_lock(&ctx.mtx);
		while(ctx.phase != 2)
		{
			sync_cond_wait(&ctx.cv, &ctx.mtx);
		}
	sync_mtx_unlock(&ctx.mtx);

	thread_join(th);
	sync_cond_free(&ctx.cv);
	sync_mtx_free(&ctx.mtx);

	int idx = alloc_test_find_live_idx(ptr);
	assert_neq(idx, -1);
	alloc_test_remove_live_idx((uint32_t) idx);
}


void attr_test_fn
test_priority_pass__alloc_huge_exact_reuse(
	void
	)
{
	alloc_numa_local_data_t* local = alloc_get_numa_local_data(0);
	assert_not_null(local);

	alloc_t size = alloc_consts.slab.virtual_cutoff + alloc_consts.page.size;
	void* ptr = alloc_test_alloc_checked(size, 0);
	alloc_test_free_checked(ptr, size);

	assert_eq(alloc_test_huge_time_count(local), 1);
	assert_eq(alloc_test_huge_bucket_count(local, size), 1);

	void* reused = alloc_test_alloc_checked(size, 0);
	assert_eq(reused, ptr);
	alloc_test_free_checked(reused, size);
}


void attr_test_fn
test_priority_pass__alloc_huge_size_miss(
	void
	)
{
	alloc_t size_a = alloc_consts.slab.virtual_cutoff + alloc_consts.page.size;
	alloc_t size_b = size_a + alloc_consts.page.size;

	void* a = alloc_test_alloc_checked(size_a, 0);
	alloc_test_free_checked(a, size_a);

	void* b = alloc_test_alloc_checked(size_b, 0);
	assert_neq(b, a);

	void* a2 = alloc_test_alloc_checked(size_a, 0);
	assert_eq(a2, a);

	alloc_test_free_checked(b, size_b);
	alloc_test_free_checked(a2, size_a);
}


void attr_test_fn
test_priority_pass__alloc_huge_zero_reuse(
	void
	)
{
	alloc_t size = alloc_consts.slab.virtual_cutoff + alloc_consts.page.size;
	uint8_t* ptr = alloc_test_alloc_checked(size, 0);
	memset(ptr, 0xA5, size);
	alloc_test_free_checked(ptr, size);

	uint8_t* reused = alloc_test_alloc_checked(size, 1);
	assert_eq(reused, ptr);
	alloc_test_assert_zero(reused, size);
	alloc_test_free_checked(reused, size);
}


void attr_test_fn
test_priority_pass__alloc_small_pow2_map(
	void
	)
{
	alloc_t ptr_size = sizeof(void*);

	for(alloc_t size = 1; size <= alloc_consts.slab.max_nonvirtual_size; ++size)
	{
		const alloc_handle_t* handle = alloc_test_get_handle(size);
		assert_not_null(handle);

		alloc_t expected_alignment = size <= ptr_size ?
			ptr_size : MACRO_MIN((alloc_t) 1 << MACRO_CEIL_LOG2(size), alloc_consts.page.size);

		assert_eq(handle->alignment, expected_alignment);
	}
}


void attr_test_fn
test_priority_pass__alloc_default_idx_map(
	void
	)
{
	for(alloc_t size = 1; size <= alloc_consts.slab.max_nonvirtual_size; ++size)
	{
		alloc_handle_idx_t expected = size <= sizeof(void*) ?
			0 : MACRO_CEIL_LOG2(size) - alloc_consts.slab.smallest_allocation_shift;
		alloc_handle_idx_t got = alloc_test_default_idx_fn(size);
		assert_eq(got, expected);
		assert_lt(got, alloc_consts.handles.count);
	}
}


void attr_test_fn
test_priority_pass__alloc_aligned_idx_match(
	void
	)
{
	for(alloc_t size = 1; size <= alloc_consts.slab.max_nonvirtual_size; ++size)
	{
		for(alloc_t s = 0; s <= alloc_consts.page.shift; ++s)
		{
			alloc_t alignment = (alloc_t) 1 << s;
			alloc_t effective_size = MACRO_MAX(size, alignment);
			alloc_handle_idx_t expected = alloc_test_default_idx_fn(effective_size);

			void* ptr = alloc_test_alloc_aligned_checked(size, alignment, 0);
			alloc_slab_header_t* slab = alloc_test_ptr_slab_header(ptr);
			alloc_handle_idx_t got = slab->handle - slab->arena->tcb->handles;

			assert_eq(got, expected);
			assert_ge(slab->handle->alignment, alignment);
			alloc_test_free_aligned_checked(ptr, size, alignment);
		}
	}
}


void attr_test_fn
test_priority_pass__alloc_huge_chain_reuse(
	void
	)
{
	alloc_numa_local_data_t* local = alloc_test_local_data_current_numa(NULL);
	alloc_t size_a = alloc_consts.slab.virtual_cutoff + alloc_consts.page.size;
	alloc_t size_b = alloc_test_find_huge_collision_size(size_a);

	void* a = alloc_test_alloc_checked(size_a, 0);
	void* b = alloc_test_alloc_checked(size_b, 0);
	alloc_test_free_checked(a, size_a);
	alloc_test_free_checked(b, size_b);

	assert_eq(alloc_test_huge_hash(size_a), alloc_test_huge_hash(size_b));
	assert_eq(alloc_test_huge_time_count(local), 2);
	assert_eq(alloc_test_huge_bucket_count(local, size_a), 1);
	assert_eq(alloc_test_huge_bucket_count(local, size_b), 1);

	void* a2 = alloc_test_alloc_checked(size_a, 0);
	void* b2 = alloc_test_alloc_checked(size_b, 0);
	assert_eq(a2, a);
	assert_eq(b2, b);

	alloc_test_free_checked(a2, size_a);
	alloc_test_free_checked(b2, size_b);
}


void attr_test_fn
test_priority_pass__alloc_huge_timeout(
	void
	)
{
	alloc_numa_local_data_t* local = alloc_test_local_data_current_numa(NULL);
	uint16_t numa;
	alloc_get_cpu_numa_node(NULL, &numa);

	alloc_t size = alloc_consts.slab.virtual_cutoff + alloc_consts.page.size;
	void* ptr = alloc_test_alloc_checked(size, 0);
	alloc_test_free_checked(ptr, size);

	assert_eq(alloc_test_huge_time_count(local), 1);
	uint64_t deadline = atomic_load_acq(&local->huge.timer);
	assert_neq(deadline, UINT64_MAX);

	alloc_huge_reap_due(numa, deadline - 1);
	assert_eq(alloc_test_huge_time_count(local), 1);

	alloc_huge_reap_due(numa, deadline);
	alloc_huge_reap_due(numa, UINT64_MAX);
	assert_eq(alloc_test_huge_time_count(local), 0);
	assert_eq(alloc_test_huge_bucket_count(local, size), 0);
}


void attr_test_fn
test_priority_pass__alloc_huge_start_idle(
	void
	)
{
	uint16_t numa;
	alloc_numa_local_data_t* local = alloc_test_local_data_current_numa(&numa);
	uint64_t startup_deadline = atomic_load_acq(&local->huge.timer);
	if(startup_deadline == UINT64_MAX)
	{
		alloc_t size = alloc_consts.slab.virtual_cutoff + alloc_consts.page.size;
		void* ptr = alloc_test_alloc_checked(size, 0);
		alloc_test_free_checked(ptr, size);
		startup_deadline = atomic_load_acq(&local->huge.timer);
	}
	assert_neq(startup_deadline, UINT64_MAX);

	alloc_huge_reap_due(numa, startup_deadline - 1);
	assert_eq(atomic_load_acq(&local->huge.timer), startup_deadline);

	alloc_huge_reap_due(numa, startup_deadline);
	assert_eq(atomic_load_acq(&local->huge.timer), UINT64_MAX);
}


void attr_test_fn
test_priority_pass__alloc_thr_cache_pool(
	void
	)
{
	alloc_worker_sync_t sync = {0};
	sync_mtx_init(&sync.mtx);
	sync_cond_init(&sync.cv);

	thread_t th;
	thread_init(&th, (thread_data_t){ alloc_test_worker_two_arena_pause, &sync });

	sync_mtx_lock(&sync.mtx);
		while(sync.phase != 1)
		{
			sync_cond_wait(&sync.cv, &sync.mtx);
		}

		assert_ge(sync.unique_arenas, 2);
		alloc_t before = alloc_test_global_pool_count(sync.numa);

		sync.phase = 2;
		sync_cond_wake(&sync.cv);
	sync_mtx_unlock(&sync.mtx);

	thread_join(th);

	alloc_t after = alloc_test_global_pool_count(sync.numa);
	assert_ge(after, before + 1);

	sync_cond_free(&sync.cv);
	sync_mtx_free(&sync.mtx);
}


void attr_test_fn
test_priority_pass__alloc_handle_empty_lim(
	void
	)
{
	alloc_t size = alloc_test_size_for_class(0);
	const alloc_handle_t* handle = alloc_test_get_handle(size);
	alloc_t per_slab = handle->alloc_limit;
	alloc_t total = per_slab * 3;

	void** ptrs = malloc(sizeof(*ptrs) * total);
	assert_not_null(ptrs);
	for(alloc_t i = 0; i < total; ++i)
	{
		ptrs[i] = alloc_malloc_e(size);
		assert_not_null(ptrs[i]);
	}

	for(alloc_t i = 0; i < total; ++i)
	{
		alloc_free_e(ptrs[i], size);
	}

	alloc_t empty = 0;
	for(const alloc_slab_header_t* slab = handle->slab; slab; slab = slab->next)
	{
		if(!slab->count)
		{
			++empty;
		}
	}
	assert_le(empty, alloc_consts.slab.max_free_per_handle);

	free(ptrs);
}


void attr_test_fn
test_priority_pass__alloc_tail_shrink(
	void
	)
{
	alloc_config_t cfg = {0};
	cfg.arena_max_free_per_thread = 2;
	cfg.arena_max_free_per_thread_set = 1;
	cfg.slab_max_free_per_handle = 0;
	cfg.slab_max_free_per_handle_set = 1;
	cfg.tcache_max_per_class = 0;
	cfg.tcache_max_per_class_set = 1;
	cfg.arena_tail_decommit_enable = 1;
	cfg.arena_tail_decommit_enable_set = 1;
	cfg.arena_tail_decommit_trigger_div = 2;
	cfg.arena_tail_decommit_trigger_div_set = 1;
	cfg.arena_tail_decommit_amount_div = 2;
	cfg.arena_tail_decommit_amount_div_set = 1;
	alloc_configure(&cfg);

	uint16_t numa;
	alloc_numa_local_data_t* local = alloc_test_local_data_current_numa(&numa);
	assert_not_null(local);

	alloc_t size = alloc_test_size_for_class(0);
	const alloc_handle_t* h = alloc_test_get_handle(size);
	alloc_t class_idx = h->slab_class;
	alloc_t per_arena = (alloc_consts.arena.slab_count / alloc_consts.slab.class_span[class_idx]) * h->alloc_limit;
	assert_gt(per_arena, 0);

	alloc_t target_slots = 48;
	alloc_t total = per_arena * target_slots;
	alloc_slot_alloc_t* allocs = calloc(total, sizeof(*allocs));
	assert_not_null(allocs);

	for(alloc_t i = 0; i < total; ++i)
	{
		allocs[i].ptr = alloc_malloc_e(size);
		assert_not_null(allocs[i].ptr);
		allocs[i].slot = alloc_test_ptr_slot_idx(allocs[i].ptr, local);
	}

	uint32_t old_hwm = local->arena.high_watermark;
	assert_ge(old_hwm, 8);

	uint32_t keep_slots = old_hwm / 4;
	if(!keep_slots)
	{
		keep_slots = 1;
	}

	for(alloc_t i = 0; i < total; ++i)
	{
		if(allocs[i].ptr && allocs[i].slot >= keep_slots)
		{
			alloc_free_e(allocs[i].ptr, size);
			allocs[i].ptr = NULL;
		}
	}

	assert_lt(local->arena.high_watermark, old_hwm);

	for(alloc_t i = 0; i < total; ++i)
	{
		if(allocs[i].ptr)
		{
			alloc_free_e(allocs[i].ptr, size);
		}
	}

	free(allocs);
}


void attr_test_fn
test_priority_pass__alloc_tail_no_shrink(
	void
	)
{
	alloc_config_t cfg = {0};
	cfg.arena_max_free_per_thread = 2;
	cfg.arena_max_free_per_thread_set = 1;
	cfg.slab_max_free_per_handle = 0;
	cfg.slab_max_free_per_handle_set = 1;
	cfg.tcache_max_per_class = 0;
	cfg.tcache_max_per_class_set = 1;
	cfg.arena_tail_decommit_enable = 1;
	cfg.arena_tail_decommit_enable_set = 1;
	cfg.arena_tail_decommit_trigger_div = 2;
	cfg.arena_tail_decommit_trigger_div_set = 1;
	cfg.arena_tail_decommit_amount_div = 2;
	cfg.arena_tail_decommit_amount_div_set = 1;
	alloc_configure(&cfg);

	uint16_t numa;
	alloc_numa_local_data_t* local = alloc_test_local_data_current_numa(&numa);
	assert_not_null(local);

	alloc_t size = alloc_test_size_for_class(0);
	const alloc_handle_t* h = alloc_test_get_handle(size);
	alloc_t class_idx = h->slab_class;
	alloc_t per_arena = (alloc_consts.arena.slab_count / alloc_consts.slab.class_span[class_idx]) * h->alloc_limit;
	assert_gt(per_arena, 0);

	alloc_t target_slots = 48;
	alloc_t total = per_arena * target_slots;
	alloc_slot_alloc_t* allocs = calloc(total, sizeof(*allocs));
	assert_not_null(allocs);

	for(alloc_t i = 0; i < total; ++i)
	{
		allocs[i].ptr = alloc_malloc_e(size);
		assert_not_null(allocs[i].ptr);
		allocs[i].slot = alloc_test_ptr_slot_idx(allocs[i].ptr, local);
	}

	uint32_t old_hwm = local->arena.high_watermark;
	assert_ge(old_hwm, 8);

	uint32_t free_below = old_hwm / 2;
	if(!free_below)
	{
		free_below = 1;
	}

	for(alloc_t i = 0; i < total; ++i)
	{
		if(allocs[i].ptr && allocs[i].slot < free_below)
		{
			alloc_free_e(allocs[i].ptr, size);
			allocs[i].ptr = NULL;
		}
	}

	assert_eq(local->arena.high_watermark, old_hwm);

	for(alloc_t i = 0; i < total; ++i)
	{
		if(allocs[i].ptr)
		{
			alloc_free_e(allocs[i].ptr, size);
		}
	}

	free(allocs);
}


void attr_test_fn
test_priority_pass__alloc_pool_zero_user(
	void
	)
{
	alloc_config_t cfg = {0};
	cfg.arena_max_free_per_thread = 1;
	cfg.arena_max_free_per_thread_set = 1;
	cfg.slab_max_free_per_handle = 0;
	cfg.slab_max_free_per_handle_set = 1;
	cfg.tcache_max_per_class = 0;
	cfg.tcache_max_per_class_set = 1;
	cfg.arena_tail_decommit_enable = 0;
	cfg.arena_tail_decommit_enable_set = 1;
	alloc_configure(&cfg);

	alloc_pool_reuse_worker_ctx_t ctx =
	(alloc_pool_reuse_worker_ctx_t)
	{
		.arena = NULL,
		.slot = 0,
		.numa = 0,
		.size = 256
	};

	uint16_t numa;
	alloc_get_cpu_numa_node(NULL, &numa);
	alloc_t before = alloc_test_global_pool_count(numa);

	thread_t th;
	thread_init(&th, (thread_data_t){ alloc_test_worker_pool_reuse_fill, &ctx });
	thread_join(th);

	assert_not_null(ctx.arena);
	assert_ge(alloc_test_global_pool_count(ctx.numa), before + 1);

	void* zero_ptr = alloc_test_alloc_checked(ctx.size, 1);
	alloc_test_assert_zero(zero_ptr, ctx.size);
	alloc_test_free_checked(zero_ptr, ctx.size);
}


void attr_test_fn
test_priority_pass__alloc_large_cutoff(
	void
	)
{
	alloc_config_t cfg = {0};
	cfg.slab_small = (alloc_t) 1 << 13;
	cfg.slab_small_set = 1;
	cfg.slab_medium = (alloc_t) 1 << 15;
	cfg.slab_medium_set = 1;
	cfg.slab_large = (alloc_t) 1 << 17;
	cfg.slab_large_set = 1;
	cfg.slab_small_min_blocks = 16;
	cfg.slab_small_min_blocks_set = 1;
	cfg.slab_medium_min_blocks = 8;
	cfg.slab_medium_min_blocks_set = 1;
	cfg.slab_large_min_blocks = 4;
	cfg.slab_large_min_blocks_set = 1;

	alloc_configure(&cfg);

	assert_eq(alloc_consts.slab.class_count, 3);
	assert_eq(alloc_consts.slab.virtual_cutoff, (alloc_t) 1 << 15);
	assert_le(alloc_consts.slab.max_nonvirtual_size, alloc_consts.slab.virtual_cutoff);
	assert_not_null(alloc_test_get_handle(alloc_consts.slab.max_nonvirtual_size));
	assert_null(alloc_test_get_handle(alloc_consts.slab.max_nonvirtual_size + 1));
}


void attr_test_fn
test_priority_pass__alloc_large_virtual(
	void
	)
{
	alloc_t size = alloc_consts.slab.max_nonvirtual_size + alloc_consts.page.size;
	assert_true(size > alloc_consts.slab.max_nonvirtual_size);
	assert_null(alloc_test_get_handle(size));

	void* ptr = alloc_test_alloc_checked(size, 0);
	alloc_test_free_checked(ptr, size);
}


void attr_test_fn
test_priority_pass__alloc_span_fit(
	void
	)
{
	alloc_t arena_blocks = alloc_consts.arena.slab_count;
	for(alloc_t size = 1; size <= alloc_consts.slab.max_nonvirtual_size; ++size)
	{
		const alloc_handle_t* handle = alloc_test_get_handle(size);
		assert_not_null(handle);
		assert_true(handle->alloc_size);

		alloc_t span = alloc_consts.slab.class_span[handle->slab_class];
		assert_le(span, arena_blocks);
	}
}


void attr_test_fn
test_priority_pass__alloc_largest_align(
	void
	)
{
	alloc_t size = alloc_consts.slab.max_nonvirtual_size;
	void* ptr = alloc_test_alloc_checked(size, 0);

	const alloc_handle_t* handle = alloc_test_get_handle(size);
	if(handle)
	{
		assert_ge((alloc_t) 1 << handle->slab_shift, size);
		assert_eq((alloc_t) ptr & (handle->alignment - 1), 0);
	}

	alloc_test_free_checked(ptr, size);
}


void attr_test_fn
test_priority_pass__alloc_huge_mid_list(
	void
	)
{
	uint16_t numa;
	alloc_numa_local_data_t* local = alloc_test_local_data_current_numa(&numa);
	assert_not_null(local);

	alloc_t size_a = alloc_consts.slab.virtual_cutoff + alloc_consts.page.size;
	alloc_t size_b = size_a + alloc_consts.page.size;
	alloc_t size_c = size_b + alloc_consts.page.size;

	void* a = alloc_test_alloc_checked(size_a, 0);
	void* b = alloc_test_alloc_checked(size_b, 0);
	void* c = alloc_test_alloc_checked(size_c, 0);

	alloc_test_free_checked(a, size_a);
	alloc_test_free_checked(b, size_b);
	alloc_test_free_checked(c, size_c);

	assert_eq(alloc_test_huge_time_count(local), 3);

	void* b2 = alloc_test_alloc_checked(size_b, 0);
	assert_eq(b2, b);
	assert_eq(alloc_test_huge_time_count(local), 2);

	alloc_huge_reap_due(numa, UINT64_MAX);
	assert_eq(alloc_test_huge_time_count(local), 0);
	assert_eq(alloc_test_huge_bucket_count(local, size_a), 0);
	assert_eq(alloc_test_huge_bucket_count(local, size_b), 0);
	assert_eq(alloc_test_huge_bucket_count(local, size_c), 0);

	alloc_test_free_checked(b2, size_b);
}


void attr_test_fn
test_priority_pass__alloc_arena_coalesce(
	void
	)
{
	alloc_t size = alloc_test_size_for_class(0);
	void* a = alloc_test_alloc_checked(size, 0);
	void* b = alloc_test_alloc_checked(size, 0);
	void* c = alloc_test_alloc_checked(size, 0);

	alloc_arena_header_t* ah = alloc_test_get_arena_from_ptr(a);
	alloc_arena_header_t* bh = alloc_test_get_arena_from_ptr(b);
	alloc_arena_header_t* ch = alloc_test_get_arena_from_ptr(c);

	if(ah == bh || bh == ch || ah == ch)
	{
		alloc_test_free_checked(a, size);
		alloc_test_free_checked(b, size);
		alloc_test_free_checked(c, size);
		return;
	}

	alloc_test_free_checked(b, size);
	alloc_test_free_checked(a, size);

	uint16_t numa;
	alloc_numa_local_data_t* local = alloc_test_local_data_current_numa(&numa);
	(void) numa;

	sync_mtx_lock(&local->arena.mtx);
		uint32_t a_slot = ah->pool_idx;
		uint32_t b_slot = bh->pool_idx;

		if(a_slot + 1 == b_slot)
		{
			assert_neq(ah->link_next, bh);
		}
		else if(b_slot + 1 == a_slot)
		{
			assert_neq(bh->link_next, ah);
		}
	sync_mtx_unlock(&local->arena.mtx);

	alloc_test_free_checked(c, size);
}


void attr_test_fn
test_priority_pass__alloc_btree_cycles(
	void
	)
{
	if(alloc_consts.slab.class_count < 2)
	{
		return;
	}

	uint64_t* avail = calloc(1, alloc_btree_size());
	assert_not_null(avail);
	alloc_btree_init(avail);

	uint32_t avail_mask = 0;
	for(alloc_t c = 0; c < alloc_consts.slab.class_count; ++c)
	{
		if(avail[c])
		{
			avail_mask |= 1U << c;
		}
	}

	alloc_t children = alloc_consts.slab.class_span[1] / alloc_consts.slab.class_span[0];
	assert_le(children, 64);
	alloc_t idx[64] = {0};

	for(alloc_t cycle = 0; cycle < 32; ++cycle)
	{
		uint8_t seen[64] = {0};

		for(alloc_t i = 0; i < children; ++i)
		{
			idx[i] = alloc_btree_get(avail, 0, &avail_mask, NULL);
			assert_lt(idx[i], 64);
			assert_eq(seen[idx[i]], 0);
			seen[idx[i]] = 1;
		}

		for(alloc_t i = 0; i < children; ++i)
		{
			alloc_btree_ret(avail, 0, idx[i], &avail_mask, NULL);
		}

		alloc_t big = alloc_btree_get(avail, 1, &avail_mask, NULL);
		assert_lt(big, alloc_consts.arena.slab_count);
		assert_eq(big % children, 0);
		alloc_btree_ret(avail, 1, big, &avail_mask, NULL);
	}

	free(avail);
}


void attr_test_fn
test_priority_pass__alloc_big_to_small(
	void
	)
{
	alloc_config_t cfg = {0};
	cfg.tcache_max_per_class = 0;
	cfg.tcache_max_per_class_set = 1;
	cfg.slab_max_free_per_handle = 0;
	cfg.slab_max_free_per_handle_set = 1;
	alloc_configure(&cfg);

	if(alloc_consts.slab.class_count < 2)
	{
		return;
	}

	alloc_t big_size = alloc_test_size_for_class(1);
	alloc_t small_size = alloc_test_size_for_class(0);

	const alloc_handle_t* small_h = alloc_test_get_handle(small_size);
	alloc_t small_per_slab = small_h->alloc_limit;
	alloc_t children = alloc_consts.slab.class_span[1] / alloc_consts.slab.class_span[0];

	void* big = alloc_test_alloc_checked(big_size, 0);
	assert_eq(alloc_test_ptr_slab_header(big)->handle->slab_class, 1);
	alloc_arena_header_t* arena = alloc_test_get_arena_from_ptr(big);
	alloc_t big_start = alloc_test_ptr_slab_start_idx(big);
	alloc_test_free_checked(big, big_size);

	alloc_t n = children * small_per_slab * 2;
	void** ptrs = malloc(sizeof(*ptrs) * n);
	assert_not_null(ptrs);
	alloc_t hits = 0;

	for(alloc_t i = 0; i < n; ++i)
	{
		ptrs[i] = alloc_test_alloc_checked(small_size, 0);
		assert_eq(alloc_test_get_arena_from_ptr(ptrs[i]), arena);

		alloc_t start = alloc_test_ptr_slab_start_idx(ptrs[i]);
		if(start >= big_start && start < big_start + children)
		{
			++hits;
		}
	}

	assert_ge(hits, small_per_slab);

	for(alloc_t i = 0; i < n; ++i)
	{
		alloc_test_free_checked(ptrs[i], small_size);
	}
	free(ptrs);
}


void attr_test_fn
test_priority_pass__alloc_dtor_reclaims(
	void
	)
{
	uint16_t numa;
	alloc_get_cpu_numa_node(NULL, &numa);
	alloc_t before = alloc_test_global_pool_count(numa);

	alloc_retained_worker_ctx_t ctx =
	(alloc_retained_worker_ctx_t)
	{
		.size = alloc_test_size_for_class(0)
	};

	thread_t th;
	thread_init(&th, (thread_data_t){ alloc_test_worker_retained_then_exit, &ctx });
	thread_join(th);

	alloc_t after = alloc_test_global_pool_count(numa);
	assert_ge(after, before + 1);
}


void attr_test_fn
test_priority_pass__alloc_empty_lru_evict(
	void
	)
{
	alloc_t size = 2048;
	const alloc_handle_t* handle = alloc_test_get_handle(size);
	if(!handle)
	{
		return;
	}

	alloc_t span = alloc_consts.slab.class_span[handle->slab_class];
	alloc_t per_arena = (alloc_consts.arena.slab_count / span) * handle->alloc_limit;
	if(per_arena != 1)
	{
		return;
	}

	void* p0 = alloc_test_alloc_checked(size, 0);
	void* p1 = alloc_test_alloc_checked(size, 0);
	void* p2 = alloc_test_alloc_checked(size, 0);

	alloc_arena_header_t* a0 = alloc_test_get_arena_from_ptr(p0);
	alloc_arena_header_t* a1 = alloc_test_get_arena_from_ptr(p1);
	alloc_arena_header_t* a2 = alloc_test_get_arena_from_ptr(p2);

	if(a0 == a1 || a1 == a2 || a0 == a2)
	{
		alloc_test_free_checked(p0, size);
		alloc_test_free_checked(p1, size);
		alloc_test_free_checked(p2, size);
		return;
	}

	alloc_test_free_checked(p0, size);
	alloc_test_free_checked(p1, size);
	alloc_test_free_checked(p2, size);

	assert_eq(a0->in_empty + a1->in_empty + a2->in_empty, 2);
}


void attr_test_fn
test_priority_pass__alloc_huge_calibrate(
	void
	)
{
	alloc_config_t cfg = {0};
	cfg.huge_threshold_ms = 60000;
	cfg.huge_threshold_ms_set = 1;
	alloc_configure(&cfg);

	uint16_t numa;
	alloc_get_cpu_numa_node(NULL, &numa);
	alloc_numa_local_data_t* local = alloc_get_numa_local_data(numa);
	assert_not_null(local);

	alloc_t size = alloc_consts.slab.virtual_cutoff + alloc_consts.page.size;
	void* ptr = alloc_test_alloc_checked(size, 0);
	alloc_test_free_checked(ptr, size);

	assert_eq(alloc_test_huge_time_count(local), 1);

	uint64_t startup_deadline = atomic_load_acq(&local->huge.timer);
	assert_neq(startup_deadline, UINT64_MAX);

	alloc_huge_reap_due(numa, startup_deadline);

	uint64_t calibrated_deadline = atomic_load_acq(&local->huge.timer);
	assert_neq(calibrated_deadline, startup_deadline);

	if(alloc_test_huge_time_count(local))
	{
		assert_neq(calibrated_deadline, UINT64_MAX);
		void* reused = alloc_test_alloc_checked(size, 0);
		assert_eq(reused, ptr);
		alloc_test_free_checked(reused, size);
	}
	else
	{
		assert_eq(calibrated_deadline, UINT64_MAX);
	}
}


void attr_test_fn
test_priority_pass__alloc_aligned_inplace(
	void
	)
{
	alloc_t alignments[] = { 1, 2, 4, 8, 16 };
	alloc_t cap = alloc_consts.slab.max_nonvirtual_size;
	alloc_t step = MACRO_MAX(cap / 64, (alloc_t) 1);

	for(alloc_t ai = 0; ai < MACRO_ARRAY_LEN(alignments); ++ai)
	{
		alloc_t alignment = alignments[ai];
		if(alignment > alloc_consts.page.size)
		{
			continue;
		}

		for(alloc_t old_size = 1; old_size <= cap; old_size += step)
		{
			for(alloc_t new_size = 1; new_size < old_size; new_size += old_size / 4 + 1)
			{
				void* ptr = alloc_test_alloc_aligned_checked(old_size, alignment, 0);
				memset(ptr, 0xCC, old_size);

				void* rptr = alloc_test_realloc_aligned_checked(
					ptr, old_size, alignment, new_size, alignment, 0);
				assert_not_null(rptr);

				if(rptr == ptr)
				{
					const uint8_t* bytes = rptr;
					for(alloc_t i = 0; i < new_size; ++i)
					{
						assert_eq(bytes[i], 0xCC);
					}
				}

				alloc_test_free_aligned_checked(rptr, new_size, alignment);
			}
		}
	}
}


void attr_test_fn
test_priority_pass__alloc_aligned_zeroed(
	void
	)
{
	alloc_t alignments[] = { 1, 2, 4, 8, 16 };
	enum { ROUNDS = 4 };
	alloc_t cap = alloc_consts.slab.max_nonvirtual_size;
	alloc_t step = MACRO_MAX(cap / 64, (alloc_t) 1);

	for(alloc_t ai = 0; ai < MACRO_ARRAY_LEN(alignments); ++ai)
	{
		alloc_t alignment = alignments[ai];
		if(alignment > alloc_consts.page.size)
		{
			continue;
		}

		for(alloc_t size = 1; size <= cap; size += step)
		{
			void* ptrs[ROUNDS];

			for(alloc_t r = 0; r < ROUNDS; ++r)
			{
				ptrs[r] = alloc_test_alloc_aligned_checked(size, alignment, 0);
				memset(ptrs[r], 0xFF, size);
			}

			for(alloc_t r = 0; r < ROUNDS; ++r)
			{
				alloc_test_free_aligned_checked(ptrs[r], size, alignment);
			}

			for(alloc_t r = 0; r < ROUNDS; ++r)
			{
				void* zptr = alloc_test_alloc_aligned_checked(size, alignment, 1);
				alloc_test_assert_zero(zptr, size);
				alloc_test_free_aligned_checked(zptr, size, alignment);
			}
		}
	}
}


void attr_test_fn
test_priority_pass__alloc_tail_o1(
	void
	)
{
	uint16_t numa;
	alloc_get_cpu_numa_node(NULL, &numa);
	alloc_numa_local_data_t* local = alloc_get_numa_local_data(numa);
	assert_not_null(local);

	alloc_t size = alloc_test_size_for_class(0);
	void* ptrs[8];

	for(alloc_t i = 0; i < MACRO_ARRAY_LEN(ptrs); ++i)
	{
		ptrs[i] = alloc_test_alloc_checked(size, 0);
	}

	sync_mtx_lock(&local->arena.mtx);
		uint32_t hwm_before = local->arena.high_watermark;
		alloc_arena_header_t* tail_before = local->arena.link_tail;
		uint32_t tail_slot_before = tail_before ? tail_before->pool_idx : 0;
	sync_mtx_unlock(&local->arena.mtx);

	if(tail_before)
	{
		assert_lt(tail_slot_before, hwm_before);
	}

	for(alloc_t i = MACRO_ARRAY_LEN(ptrs); i > 0; --i)
	{
		alloc_test_free_checked(ptrs[i - 1], size);
	}

	sync_mtx_lock(&local->arena.mtx);
		alloc_arena_header_t* tail_after = local->arena.link_tail;
		uint32_t hwm_after = local->arena.high_watermark;
		if(tail_after)
		{
			uint32_t tail_slot_after = tail_after->pool_idx;
			bool bit_set = local->arena.bitmap_l0[tail_slot_after >> 6] &
				((uint64_t) 1 << (tail_slot_after & 63));

			if(!bit_set)
			{
				uint32_t tail_free = hwm_after - tail_slot_after;
				assert_gt(tail_free, 0);
			}
		}
	sync_mtx_unlock(&local->arena.mtx);
}


void attr_test_fn
test_priority_pass__alloc_tcache_frag(
	void
	)
{
	enum { PTR_COUNT = 4096 };
	alloc_t cases[] = { 32, 128, 512, 2048 };

	void** ptrs = malloc(sizeof(*ptrs) * PTR_COUNT);
	alloc_t* sizes = malloc(sizeof(*sizes) * PTR_COUNT);
	assert_not_null(ptrs);
	assert_not_null(sizes);

	for(alloc_t ci = 0; ci < MACRO_ARRAY_LEN(cases); ++ci)
	{
		alloc_t base = cases[ci];
		if(base > alloc_consts.slab.max_nonvirtual_size)
		{
			continue;
		}

		for(alloc_t i = 0; i < PTR_COUNT; ++i)
		{
			ptrs[i] = alloc_test_alloc_checked(base, 0);
			sizes[i] = base;
		}

		for(alloc_t i = 1; i < PTR_COUNT; i += 2)
		{
			alloc_test_free_checked(ptrs[i], sizes[i]);
			ptrs[i] = NULL;
		}

		for(alloc_t i = 1; i < PTR_COUNT; i += 2)
		{
			alloc_t expanded = base << 1;
			if(expanded > alloc_consts.slab.max_nonvirtual_size)
			{
				expanded = base;
			}
			ptrs[i] = alloc_test_alloc_checked(expanded, 0);
			sizes[i] = expanded;
		}

		for(alloc_t i = 0; i < PTR_COUNT; ++i)
		{
			alloc_test_free_checked(ptrs[i], sizes[i]);
		}
	}

	free(sizes);
	free(ptrs);
}


void attr_test_fn
test_priority_pass__alloc_slab_pack_one(
	void
	)
{
	alloc_t size = alloc_test_size_for_class(0);
	const alloc_handle_t* handle = alloc_test_get_handle(size);
	alloc_t alloc_limit = handle->alloc_limit;
	alloc_t slabs_to_fill = 8;
	alloc_t total = alloc_limit * slabs_to_fill;

	void** ptrs = malloc(sizeof(*ptrs) * total);
	assert_not_null(ptrs);

	for(alloc_t i = 0; i < total; ++i)
	{
		ptrs[i] = alloc_test_alloc_checked(size, 0);
	}

	alloc_arena_header_t* arena0 = alloc_test_ptr_arena(ptrs[0]);
	uint8_t seen[64] = {0};

	for(alloc_t slab_n = 0; slab_n < slabs_to_fill; ++slab_n)
	{
		alloc_t i = slab_n * alloc_limit;
		assert_eq(alloc_test_ptr_arena(ptrs[i]), arena0);
		alloc_t slab_start = alloc_test_ptr_slab_start_idx(ptrs[i]);
		assert_lt(slab_start, 64);
		assert_eq(seen[slab_start], 0);
		seen[slab_start] = 1;
	}

	for(alloc_t i = 0; i < total; ++i)
	{
		alloc_test_free_checked(ptrs[i], size);
	}
	free(ptrs);
}


void attr_test_fn
test_priority_pass__alloc_small_no_spread(
	void
	)
{
	alloc_t size = 256;
	enum { N = 4096 };
	void* ptrs[N] = {0};
	alloc_arena_header_t* seen[128] = {0};
	alloc_t seen_count = 0;

	for(alloc_t i = 0; i < N; ++i)
	{
		ptrs[i] = alloc_test_alloc_checked(size, 0);
		alloc_arena_header_t* arena = alloc_test_ptr_arena(ptrs[i]);
		alloc_t j = 0;
		for(; j < seen_count; ++j)
		{
			if(seen[j] == arena)
			{
				break;
			}
		}
		if(j == seen_count)
		{
			assert_lt(seen_count, MACRO_ARRAY_LEN(seen));
			seen[seen_count++] = arena;
		}
	}

	for(alloc_t i = 0; i < N; ++i)
	{
		alloc_test_free_checked(ptrs[i], size);
	}

	const alloc_handle_t* h = alloc_test_get_handle(size);
	alloc_t slabs_per_arena = alloc_consts.arena.slab_count / alloc_consts.slab.class_span[h->slab_class];
	alloc_t per_arena = slabs_per_arena * h->alloc_limit;
	alloc_t min_arenas = (N + per_arena - 1) / per_arena;
	assert_ge(seen_count, min_arenas);
	assert_le(seen_count, min_arenas + 4);
}


void attr_test_fn
test_priority_pass__alloc_vacancy_cross(
	void
	)
{
	if(alloc_consts.slab.class_count < 2)
	{
		return;
	}

	alloc_t big_size = alloc_test_size_for_class(1);
	alloc_t small_size = alloc_test_size_for_class(0);

	void* big = alloc_test_alloc_checked(big_size, 0);
	alloc_arena_header_t* arena_big = alloc_test_ptr_arena(big);
	alloc_test_free_checked(big, big_size);

	void* small = alloc_test_alloc_checked(small_size, 0);
	assert_eq(alloc_test_ptr_arena(small), arena_big);
	assert_eq(alloc_test_ptr_slab_header(small)->handle->slab_class, 0);
	alloc_test_free_checked(small, small_size);
}


void attr_test_fn
test_priority_pass__alloc_btree_coalesce(
	void
	)
{
	if(alloc_consts.slab.class_count < 2)
	{
		return;
	}

	uint64_t* avail = calloc(1, alloc_btree_size());
	assert_not_null(avail);
	alloc_btree_init(avail);

	uint32_t avail_mask = 0;
	for(alloc_t c = 0; c < alloc_consts.slab.class_count; ++c)
	{
		if(avail[c])
		{
			avail_mask |= 1U << c;
		}
	}

	alloc_t children = alloc_consts.slab.class_span[1] / alloc_consts.slab.class_span[0];
	alloc_t idx[64] = {0};
	for(alloc_t i = 0; i < children; ++i)
	{
		idx[i] = alloc_btree_get(avail, 0, &avail_mask, NULL);
	}
	for(alloc_t i = 0; i < children; ++i)
	{
		alloc_btree_ret(avail, 0, idx[i], &avail_mask, NULL);
	}

	alloc_t big = alloc_btree_get(avail, 1, &avail_mask, NULL);
	assert_eq(big % children, 0);
	free(avail);
}


void attr_test_fn
test_priority_pass__alloc_local_cache_reuse(
	void
	)
{
	uint16_t numa;
	alloc_get_cpu_numa_node(NULL, &numa);
	assert_eq(alloc_test_global_pool_count(numa), 0);

	alloc_t size = alloc_test_size_for_class(0);
	const alloc_handle_t* h = alloc_test_get_handle(size);
	alloc_t n = h->alloc_limit;
	void** ptrs = malloc(sizeof(*ptrs) * n);
	assert_not_null(ptrs);

	for(alloc_t i = 0; i < n; ++i)
	{
		ptrs[i] = alloc_test_alloc_checked(size, 0);
	}
	alloc_arena_header_t* arena0 = alloc_test_ptr_arena(ptrs[0]);

	for(alloc_t i = 0; i < n; ++i)
	{
		alloc_test_free_checked(ptrs[i], size);
	}

	assert_eq(alloc_test_global_pool_count(numa), 0);

	void* p = alloc_test_alloc_checked(size, 0);
	assert_eq(alloc_test_ptr_arena(p), arena0);
	assert_eq(alloc_test_global_pool_count(numa), 0);
	alloc_test_free_checked(p, size);
	free(ptrs);
}


void attr_test_fn
test_priority_pass__alloc_local_empty_zero(
	void
	)
{
	alloc_config_t cfg = {0};
	cfg.arena_max_free_per_thread = ALLOC_MAX_ARENA_FREE_PER_THREAD;
	cfg.arena_max_free_per_thread_set = 1;
	cfg.slab_max_free_per_handle = 0;
	cfg.slab_max_free_per_handle_set = 1;
	cfg.tcache_max_per_class = 0;
	cfg.tcache_max_per_class_set = 1;
	cfg.arena_tail_decommit_enable = 0;
	cfg.arena_tail_decommit_enable_set = 1;
	alloc_configure(&cfg);

	alloc_t size = 256;
	const alloc_handle_t* h = alloc_test_get_handle(size);
	alloc_t per_arena = (alloc_consts.arena.slab_count / alloc_consts.slab.class_span[h->slab_class]) * h->alloc_limit;
	assert_gt(per_arena, 0);

	void** ptrs = malloc(sizeof(*ptrs) * per_arena);
	assert_not_null(ptrs);

	for(alloc_t i = 0; i < per_arena; ++i)
	{
		ptrs[i] = alloc_test_alloc_checked(size, 0);
		memset(ptrs[i], 0xA5, size);
	}

	alloc_arena_header_t* arena0 = alloc_test_ptr_arena(ptrs[0]);
	for(alloc_t i = 0; i < per_arena; ++i)
	{
		alloc_test_free_checked(ptrs[i], size);
	}

	assert_eq(arena0->in_empty, 1);
	void* z = alloc_test_alloc_checked(size, 1);
	assert_eq(alloc_test_ptr_arena(z), arena0);
	assert_eq(arena0->in_empty, 0);
	assert_eq(alloc_test_ptr_arena(z)->clean, 0);
	assert_eq(alloc_test_ptr_slab_header(z)->clean, 0);
	alloc_test_assert_zero(z, size);
	alloc_test_free_checked(z, size);
	free(ptrs);
}


void attr_test_fn
test_priority_pass__alloc_local_empty_hdrs(
	void
	)
{
	alloc_config_t cfg = {0};
	cfg.arena_max_free_per_thread = ALLOC_MAX_ARENA_FREE_PER_THREAD;
	cfg.arena_max_free_per_thread_set = 1;
	cfg.slab_max_free_per_handle = 0;
	cfg.slab_max_free_per_handle_set = 1;
	cfg.tcache_max_per_class = 0;
	cfg.tcache_max_per_class_set = 1;
	cfg.arena_tail_decommit_enable = 0;
	cfg.arena_tail_decommit_enable_set = 1;
	alloc_configure(&cfg);

	alloc_t small = alloc_test_size_for_class(0);
	const alloc_handle_t* sh = alloc_test_get_handle(small);
	alloc_t empty_before = alloc_test_count_handle_empty_slabs(sh);
	alloc_t small_per_arena =
		(alloc_consts.arena.slab_count / alloc_consts.slab.class_span[sh->slab_class]) * sh->alloc_limit;
	void** small_ptrs = malloc(sizeof(*small_ptrs) * small_per_arena);
	assert_not_null(small_ptrs);

	for(alloc_t i = 0; i < small_per_arena; ++i)
	{
		small_ptrs[i] = alloc_test_alloc_checked(small, 0);
	}
	alloc_arena_header_t* arena0 = alloc_test_ptr_arena(small_ptrs[0]);
	for(alloc_t i = 0; i < small_per_arena; ++i)
	{
		alloc_test_free_checked(small_ptrs[i], small);
	}

	assert_eq(arena0->in_empty, 1);
	assert_eq(alloc_test_count_handle_empty_slabs(sh), empty_before);

	alloc_t large = alloc_test_size_for_class(alloc_consts.slab.class_count - 1);
	void* lp = alloc_test_alloc_checked(large, 0);
	assert_eq(alloc_test_ptr_arena(lp), arena0);

	alloc_slab_header_t* slab = alloc_test_ptr_slab_header(lp);
	assert_gt(slab->span, 1);
	for(alloc_t i = 1; i < slab->span; ++i)
	{
		alloc_slab_header_t* c = slab + i;
		assert_eq(c->idx, 0);
		assert_eq(c->count, 0);
		assert_null(c->handle);
		assert_null(atomic_load_acq(&c->foreign_free));
	}

	alloc_test_free_checked(lp, large);
	free(small_ptrs);
}


void attr_test_fn
test_priority_pass__alloc_tail_reuse_clean(
	void
	)
{
	alloc_config_t cfg = {0};
	cfg.arena_max_free_per_thread = 1;
	cfg.arena_max_free_per_thread_set = 1;
	cfg.slab_max_free_per_handle = 0;
	cfg.slab_max_free_per_handle_set = 1;
	cfg.tcache_max_per_class = 0;
	cfg.tcache_max_per_class_set = 1;
	cfg.arena_tail_decommit_enable = 1;
	cfg.arena_tail_decommit_enable_set = 1;
	cfg.arena_tail_decommit_trigger_div = 2;
	cfg.arena_tail_decommit_trigger_div_set = 1;
	cfg.arena_tail_decommit_amount_div = 2;
	cfg.arena_tail_decommit_amount_div_set = 1;
	alloc_configure(&cfg);

	uint16_t numa;
	alloc_get_cpu_numa_node(NULL, &numa);
	alloc_numa_local_data_t* local = alloc_get_numa_local_data(numa);
	assert_not_null(local);

	alloc_t size = 256;
	const alloc_handle_t* h = alloc_test_get_handle(size);
	alloc_t per_arena = (alloc_consts.arena.slab_count / alloc_consts.slab.class_span[h->slab_class]) * h->alloc_limit;
	alloc_t target_slots = 8;
	alloc_t total = per_arena * target_slots;

	alloc_slot_alloc_t* allocs = calloc(total, sizeof(*allocs));
	assert_not_null(allocs);

	for(alloc_t i = 0; i < total; ++i)
	{
		allocs[i].ptr = alloc_test_alloc_checked(size, 0);
		memset(allocs[i].ptr, 0xA5, size);
		allocs[i].slot = alloc_test_ptr_slot_idx(allocs[i].ptr, local);
	}

	uint32_t old_hwm = local->arena.high_watermark;
	uint32_t keep_slots = MACRO_MAX(1u, old_hwm / 4);
	for(alloc_t i = 0; i < total; ++i)
	{
		if(allocs[i].slot >= keep_slots && allocs[i].ptr)
		{
			alloc_test_free_checked(allocs[i].ptr, size);
			allocs[i].ptr = NULL;
		}
	}

	assert_le(local->arena.high_watermark, old_hwm);
	assert_ge(local->arena.committed_slots, local->arena.high_watermark);

	void** probes = malloc(sizeof(*probes) * total);
	assert_not_null(probes);
	alloc_t probe_n = 0;
	void* clean_ptr = NULL;

	for(alloc_t a = 0; a < target_slots && !clean_ptr; ++a)
	{
		void* first = NULL;
		for(alloc_t i = 0; i < per_arena; ++i)
		{
			void* p = alloc_test_alloc_checked(size, 1);
			probes[probe_n++] = p;
			if(!first)
			{
				first = p;
			}
		}

		alloc_arena_header_t* arena = alloc_test_ptr_arena(first);
		if(arena->clean)
		{
			assert_eq(alloc_test_ptr_slab_header(first)->clean, 1);
			clean_ptr = first;
		}
	}

	if(clean_ptr)
	{
		alloc_test_assert_zero(clean_ptr, size);
	}

	for(alloc_t i = 0; i < probe_n; ++i)
	{
		alloc_test_free_checked(probes[i], size);
	}
	for(alloc_t i = 0; i < total; ++i)
	{
		if(allocs[i].ptr)
		{
			alloc_test_free_checked(allocs[i].ptr, size);
		}
	}

	free(probes);
	free(allocs);
}


void attr_test_fn
test_priority_pass__alloc_tail_full_release(
	void
	)
{
	alloc_config_t cfg = {0};
	cfg.arena_max_free_per_thread = 2;
	cfg.arena_max_free_per_thread_set = 1;
	cfg.slab_max_free_per_handle = 0;
	cfg.slab_max_free_per_handle_set = 1;
	cfg.tcache_max_per_class = 0;
	cfg.tcache_max_per_class_set = 1;
	cfg.arena_tail_decommit_enable = 1;
	cfg.arena_tail_decommit_enable_set = 1;
	cfg.arena_tail_decommit_trigger_div = 2;
	cfg.arena_tail_decommit_trigger_div_set = 1;
	cfg.arena_tail_decommit_amount_div = 2;
	cfg.arena_tail_decommit_amount_div_set = 1;
	alloc_configure(&cfg);

	uint16_t numa;
	alloc_get_cpu_numa_node(NULL, &numa);
	alloc_numa_local_data_t* local = alloc_get_numa_local_data(numa);
	assert_not_null(local);

	alloc_t size = alloc_test_size_for_class(0);
	const alloc_handle_t* h = alloc_test_get_handle(size);
	alloc_t per_arena = (alloc_consts.arena.slab_count / alloc_consts.slab.class_span[h->slab_class]) * h->alloc_limit;
	alloc_t target_slots = 4;
	alloc_t total = per_arena * target_slots;

	alloc_slot_alloc_t* allocs = calloc(total, sizeof(*allocs));
	assert_not_null(allocs);

	for(alloc_t i = 0; i < total; ++i)
	{
		allocs[i].ptr = alloc_malloc_e(size);
		assert_ptr(allocs[i].ptr, size);
		allocs[i].slot = alloc_test_ptr_slot_idx(allocs[i].ptr, local);
	}

	uint32_t old_hwm = local->arena.high_watermark;
	uint32_t old_committed = local->arena.committed_slots;
	assert_ge(old_committed, old_hwm);

	alloc_t freed = 0;
	for(alloc_t i = total; i > 0; --i)
	{
		if(!allocs[i - 1].ptr)
		{
			continue;
		}
		alloc_free_e(allocs[i - 1].ptr, size);
		allocs[i - 1].ptr = NULL;
		++freed;
	}
	assert_eq(freed, total);

	uint32_t new_hwm = local->arena.high_watermark;
	uint32_t new_committed = local->arena.committed_slots;
	assert_le(local->arena.used_slots, alloc_consts.arena.max_free_per_thread + 1);
	assert_le(new_hwm, old_hwm);
	assert_le(new_committed, old_committed);

	free(allocs);
}


void attr_test_fn
test_priority_pass__alloc_btree_recycle_clean(
	void
	)
{
	if(alloc_consts.slab.class_count < 2)
	{
		return;
	}

	alloc_t size_a = alloc_test_size_for_class(alloc_consts.slab.class_count - 1);
	alloc_t size_b = alloc_test_size_for_class(0);
	const alloc_handle_t* h = alloc_test_get_handle(size_a);
	alloc_t n = h->alloc_limit;

	void* anchor = alloc_test_alloc_checked(size_b, 0);
	alloc_arena_header_t* arena = alloc_test_ptr_arena(anchor);
	arena->clean = 1;

	void** ptrs = malloc(sizeof(*ptrs) * n);
	assert_not_null(ptrs);
	for(alloc_t i = 0; i < n; ++i)
	{
		ptrs[i] = alloc_test_alloc_checked(size_a, 0);
		assert_eq(alloc_test_ptr_arena(ptrs[i]), arena);
		memset(ptrs[i], 0xFF, size_a);
	}
	for(alloc_t i = 0; i < n; ++i)
	{
		alloc_test_free_checked(ptrs[i], size_a);
	}
	free(ptrs);

	assert_neq(arena->count, 0);
	void* z = alloc_test_alloc_checked(size_a, 1);
	assert_eq(alloc_test_ptr_arena(z), arena);
	alloc_test_assert_zero(z, size_a);
	alloc_test_free_checked(z, size_a);
	alloc_test_free_checked(anchor, size_b);
}
