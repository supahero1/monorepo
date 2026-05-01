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
#include <shared/debug.h>
#include <shared/macro.h>
#include <shared/extent.h>
#include <shared/alloc/base.h>
#include <tests/quadtree_static.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define MAX_ENTITIES 16


void attr_test_fn
test_normal_pass__quadtree_init_free(
	void
	)
{
	quadtree_t qt = {0};
	quadtree_init(&qt);
	quadtree_free(&qt);
}


void attr_test_fn
test_normal_fail__quadtree_init_null(
	void
	)
{
	quadtree_init(NULL);
}


void attr_test_fn
test_normal_fail__quadtree_free_null(
	void
	)
{
	quadtree_free(NULL);
}


void attr_test_fn
test_normal_fail__quadtree_insert_null_qt(
	void
	)
{
	quadtree_insert(NULL, TEST_PTR);
}


void attr_test_fn
test_normal_fail__quadtree_insert_null_data(
	void
	)
{
	quadtree_t qt = {0};
	quadtree_init(&qt);
	quadtree_insert(&qt, NULL);
}


void attr_test_fn
test_normal_fail__quadtree_remove_null(
	void
	)
{
	quadtree_remove(NULL, 1);
}


void attr_test_fn
test_normal_fail__quadtree_remove_invalid_entity_idx_1(
	void
	)
{
	quadtree_t qt = {0};
	quadtree_init(&qt);
	quadtree_remove(NULL, 0);
}


void attr_test_fn
test_normal_fail__quadtree_remove_invalid_entity_idx_2(
	void
	)
{
	quadtree_t qt = {0};
	quadtree_init(&qt);
	quadtree_remove(NULL, 1);
}


void attr_test_fn
test_normal_fail__quadtree_normalize_null(
	void
	)
{
	quadtree_normalize(NULL);
}


void attr_test_fn
test_normal_fail__quadtree_update_null_qt(
	void
	)
{
	quadtree_update(NULL, TEST_PTR, NULL);
}


void attr_test_fn
test_normal_fail__quadtree_update_null_update_fn(
	void
	)
{
	quadtree_t qt = {0};
	quadtree_init(&qt);
	quadtree_update(&qt, NULL, NULL);
}


void attr_test_fn
test_normal_fail__quadtree_query_rect_null_qt(
	void
	)
{
	quadtree_query_rect(NULL, (rect_extent_t){0}, TEST_PTR, NULL);
}


void attr_test_fn
test_normal_fail__quadtree_query_rect_null_query_fn(
	void
	)
{
	quadtree_t qt = {0};
	quadtree_init(&qt);
	quadtree_query_rect(&qt, (rect_extent_t){0}, NULL, NULL);
}


void attr_test_fn
test_normal_fail__quadtree_query_circle_null_qt(
	void
	)
{
	quadtree_query_circle(NULL, 0.0f, 0.0f, 1.0f, TEST_PTR, NULL);
}


void attr_test_fn
test_normal_fail__quadtree_query_circle_null_query_fn(
	void
	)
{
	quadtree_t qt = {0};
	quadtree_init(&qt);
	quadtree_query_circle(&qt, 0.0f, 0.0f, 1.0f, NULL, NULL);
}


void attr_test_fn
test_normal_fail__quadtree_query_nodes_rect_null_qt(
	void
	)
{
	quadtree_query_nodes_rect(NULL, (rect_extent_t){0}, TEST_PTR, NULL);
}


void attr_test_fn
test_normal_fail__quadtree_query_nodes_rect_null_node_query_fn(
	void
	)
{
	quadtree_t qt = {0};
	quadtree_init(&qt);
	quadtree_query_nodes_rect(&qt, (rect_extent_t){0}, NULL, NULL);
}


void attr_test_fn
test_normal_fail__quadtree_query_nodes_circle_null_qt(
	void
	)
{
	quadtree_query_nodes_circle(NULL, 0.0f, 0.0f, 1.0f, TEST_PTR, NULL);
}


void attr_test_fn
test_normal_fail__quadtree_query_nodes_circle_null_node_query_fn(
	void
	)
{
	quadtree_t qt = {0};
	quadtree_init(&qt);
	quadtree_query_nodes_circle(&qt, 0.0f, 0.0f, 1.0f, NULL, NULL);
}


void attr_test_fn
test_normal_fail__quadtree_collide_null_qt(
	void
	)
{
	quadtree_collide(NULL, TEST_PTR, NULL);
}


void attr_test_fn
test_normal_fail__quadtree_collide_null_collide_fn(
	void
	)
{
	quadtree_t qt = {0};
	quadtree_init(&qt);
	quadtree_collide(&qt, NULL, NULL);
}


void attr_test_fn
test_normal_fail__quadtree_nearest_rect_null_qt(
	void
	)
{
	quadtree_nearest_rect(NULL, (rect_extent_t){0}, 1, TEST_PTR, NULL);
}


void attr_test_fn
test_normal_fail__quadtree_nearest_rect_null_query_fn(
	void
	)
{
	quadtree_t qt = {0};
	quadtree_init(&qt);
	quadtree_nearest_rect(&qt, (rect_extent_t){0}, 1, NULL, NULL);
}


void attr_test_fn
test_normal_fail__quadtree_nearest_circle_null_qt(
	void
	)
{
	quadtree_nearest_circle(NULL, 0.0f, 0.0f, 10.0f, 1, TEST_PTR, NULL);
}


void attr_test_fn
test_normal_fail__quadtree_nearest_circle_null_query_fn(
	void
	)
{
	quadtree_t qt = {0};
	quadtree_init(&qt);
	quadtree_nearest_circle(&qt, 0.0f, 0.0f, 10.0f, 1, NULL, NULL);
}


void attr_test_fn
test_normal_fail__quadtree_raycast_null_qt(
	void
	)
{
	quadtree_raycast(NULL, 0.0f, 0.0f, 1.0f, 0.0f, TEST_PTR, NULL);
}


void attr_test_fn
test_normal_fail__quadtree_raycast_null_query_fn(
	void
	)
{
	quadtree_t qt = {0};
	quadtree_init(&qt);
	quadtree_raycast(&qt, 0.0f, 0.0f, 1.0f, 0.0f, NULL, NULL);
}


typedef struct qt_test
{
	quadtree_t qt;
	uint32_t next_idx;
	uint32_t remove_idx;
	uint32_t updated[MAX_ENTITIES];
	uint32_t queried[MAX_ENTITIES];
	uint32_t queried_nodes[MAX_ENTITIES];
	uint32_t queried_nodes_count;
	uint32_t collided[MAX_ENTITIES][MAX_ENTITIES];
}
qt_test_t;

typedef struct qt_test_opts
{
	uint32_t split_threshold;
	uint32_t merge_threshold;
	uint32_t max_depth;
	uint32_t dfs_length;
	uint32_t merge_ht_size;
	float min_size;

	bool merge_threshold_set;
}
qt_test_opts_t;


qt_test_t
qt_test_init(
	float x,
	float y,
	float w,
	float h,
	qt_test_opts_t opts
	)
{
	qt_test_t test = {0};

	test.qt.split_threshold = opts.split_threshold;
	test.qt.merge_threshold = opts.merge_threshold;
	test.qt.max_depth = opts.max_depth;
	test.qt.dfs_length = opts.dfs_length;
	test.qt.merge_ht_size = opts.merge_ht_size;
	test.qt.min_size = opts.min_size;

	test.qt.merge_threshold_set = opts.merge_threshold_set;

	test.qt.half_extent = (half_extent_t){ .x = x, .y = y, .w = w, .h = h };
	test.qt.rect_extent = half_to_rect_extent(test.qt.half_extent);
	quadtree_init(&test.qt);

	return test;
}


void
qt_test_insert(
	qt_test_t* test,
	float x,
	float y,
	float w,
	float h
	)
{
	half_extent_t half_extent = { .x = x, .y = y, .w = w, .h = h };
	quadtree_insert(&test->qt, &(
		(qt_test_entity_data_t)
		{
			.rect_extent = half_to_rect_extent(half_extent),
			.idx = test->next_idx++
		}
		));
}


quadtree_status_t
qt_test_remove_update_fn(
	quadtree_t* qt,
	quadtree_entity_info_t info,
	void* user_data
	)
{
	(void) user_data;

	qt_test_t* test = MACRO_CONTAINER_OF(qt, qt_test_t, qt);
	if(info.data->idx == test->remove_idx)
	{
		quadtree_remove(qt, info.idx);
	}

	return QUADTREE_STATUS_NOT_CHANGED;
}


void
qt_test_remove(
	qt_test_t* test,
	uint32_t entity_idx
	)
{
	test->remove_idx = entity_idx;
	quadtree_update(&test->qt, qt_test_remove_update_fn, NULL);
}


quadtree_status_t
qt_test_update_fn(
	quadtree_t* qt,
	quadtree_entity_info_t info,
	void* user_data
	)
{
	(void) user_data;

	qt_test_t* test = MACRO_CONTAINER_OF(qt, qt_test_t, qt);
	++test->updated[info.data->idx];

	return QUADTREE_STATUS_NOT_CHANGED;
}


void
qt_test_update(
	qt_test_t* test,
	uint32_t* expected,
	uint32_t expected_count
	)
{
	memset(test->updated, 0, sizeof(test->updated));
	quadtree_update(&test->qt, qt_test_update_fn, NULL);

	for(uint32_t i = 0; i < MAX_ENTITIES; ++i)
	{
		bool should_be_updated = false;

		for(uint32_t j = 0; j < expected_count; ++j)
		{
			if(expected[j] == i)
			{
				should_be_updated = true;
				break;
			}
		}

		if(should_be_updated)
		{
			assert_eq(test->updated[i], 1, fprintf(stderr, "Expected to update entity %u\n", i););
		}
		else
		{
			assert_eq(test->updated[i], 0, fprintf(stderr, "Unexpectedly updated entity %u\n", i););
		}
	}
}


quadtree_status_t
qt_test_query_fn(
	quadtree_t* qt,
	quadtree_entity_info_t info,
	void* user_data
	)
{
	(void) user_data;

	qt_test_t* test = MACRO_CONTAINER_OF(qt, qt_test_t, qt);
	++test->queried[info.data->idx];

	return QUADTREE_STATUS_NOT_CHANGED;
}


void
qt_test_query(
	qt_test_t* test,
	float x,
	float y,
	float w,
	float h,
	uint32_t* expected,
	uint32_t expected_count
	)
{
	memset(test->queried, 0, sizeof(test->queried));
	quadtree_query_rect(&test->qt, half_to_rect_extent((half_extent_t){ .x = x, .y = y, .w = w, .h = h }), qt_test_query_fn, NULL);

	for(uint32_t i = 0; i < MAX_ENTITIES; ++i)
	{
		bool should_be_found = false;

		for(uint32_t j = 0; j < expected_count; ++j)
		{
			if(expected[j] == i)
			{
				should_be_found = true;
				break;
			}
		}

		if(should_be_found)
		{
			assert_eq(test->queried[i], 1, fprintf(stderr, "Expected to find entity %u\n", i););
		}
		else
		{
			assert_eq(test->queried[i], 0, fprintf(stderr, "Unexpectedly found entity %u\n", i););
		}
	}
}


void
qt_test_query_circle(
	qt_test_t* test,
	float x,
	float y,
	float radius,
	uint32_t* expected,
	uint32_t expected_count
	)
{
	memset(test->queried, 0, sizeof(test->queried));
	quadtree_query_circle(&test->qt, x, y, radius, qt_test_query_fn, NULL);

	for(uint32_t i = 0; i < MAX_ENTITIES; ++i)
	{
		bool should_be_found = false;

		for(uint32_t j = 0; j < expected_count; ++j)
		{
			if(expected[j] == i)
			{
				should_be_found = true;
				break;
			}
		}

		if(should_be_found)
		{
			assert_eq(test->queried[i], 1, fprintf(stderr, "Expected to find entity %u\n", i););
		}
		else
		{
			assert_eq(test->queried[i], 0, fprintf(stderr, "Unexpectedly found entity %u\n", i););
		}
	}
}


quadtree_status_t
qt_test_query_nodes_fn(
	quadtree_t* qt,
	const quadtree_node_info_t* info,
	void* user_data
	)
{
	(void) user_data;

	qt_test_t* test = MACRO_CONTAINER_OF(qt, qt_test_t, qt);
	test->queried_nodes[test->queried_nodes_count++] = info->node_idx;

	return QUADTREE_STATUS_NOT_CHANGED;
}


void
qt_test_query_nodes(
	qt_test_t* test,
	float x,
	float y,
	float w,
	float h,
	uint32_t* expected,
	uint32_t expected_count
	)
{
	test->queried_nodes_count = 0;
	memset(test->queried_nodes, 0, sizeof(test->queried_nodes));
	quadtree_query_nodes_rect(&test->qt, half_to_rect_extent((half_extent_t){ .x = x, .y = y, .w = w, .h = h }), qt_test_query_nodes_fn, NULL);

	assert_eq(test->queried_nodes_count, expected_count, fprintf(stderr, "Expected %u nodes, got %u\n", expected_count, test->queried_nodes_count););

	for(uint32_t i = 0; i < expected_count; ++i)
	{
		bool found = false;
		for(uint32_t j = 0; j < test->queried_nodes_count; ++j)
		{
			if(test->queried_nodes[j] == expected[i])
			{
				found = true;
				break;
			}
		}

		assert_eq(found, true, fprintf(stderr, "Expected to find node %u\n", expected[i]););
	}
}


void
qt_test_collide_fn(
	const quadtree_t* qt,
	quadtree_entity_info_t a,
	quadtree_entity_info_t b,
	void* user_data
	)
{
	(void) user_data;

	qt_test_t* test = MACRO_CONTAINER_OF(qt, qt_test_t, qt);

	++test->collided[a.data->idx][b.data->idx];
	++test->collided[b.data->idx][a.data->idx];
}


void
qt_test_collide(
	qt_test_t* test,
	ipair_t* expected,
	uint32_t expected_count
	)
{
	memset(test->collided, 0, sizeof(test->collided));
	quadtree_collide(&test->qt, qt_test_collide_fn, NULL);

	for(uint32_t i = 0; i < MAX_ENTITIES; ++i)
	{
		for(uint32_t j = 0; j < MAX_ENTITIES; ++j)
		{
			bool should_collide = false;

			for(uint32_t k = 0; k < expected_count; ++k)
			{
				if((expected[k].x == i && expected[k].y == j) ||
				   (expected[k].y == i && expected[k].x == j))
				{
					should_collide = true;
					break;
				}
			}

			if(should_collide)
			{
				assert_eq(test->collided[i][j], 1, fprintf(stderr, "Expected collision between %u and %u\n", i, j););
			}
			else
			{
				assert_eq(test->collided[i][j], 0, fprintf(stderr, "Unexpected collision between %u and %u\n", i, j););
			}
		}
	}
}


void
qt_test_normalize(
	qt_test_t* test
	)
{
	quadtree_normalize(&test->qt);
}


void
qt_test_free(
	qt_test_t* test
	)
{
	quadtree_free(&test->qt);
}


void attr_test_fn
test_normal_pass__quadtree_update_one(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 0.0f, 0.0f, 0.0f, 0.0f);
	qt_test_update(&test, (uint32_t[]){ 0 }, 1);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_collide_one(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 0.0f, 0.0f, 1.0f, 1.0f);
	qt_test_collide(&test, NULL, 0);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_query_one(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 0.0f, 0.0f, 1.0f, 1.0f);
	qt_test_query(&test, 0.0f, 0.0f, 2.0f, 2.0f, (uint32_t[]){ 0 }, 1);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_free_non_empty(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 0.0f, 0.0f, 1.0f, 1.0f);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_normalize_nothing(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_normalize(&test);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_border_collide(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){ .split_threshold = 2 }
		);

	qt_test_insert(&test, -1.0f, -1.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 1.0f, 1.0f, 1.0f, 1.0f);
	qt_test_collide(&test, (ipair_t[]){ {{ 0, 1 }} }, 1);
	assert_neq(test.qt.nodes[0].type, QUADTREE_NODE_TYPE_LEAF);

	qt_test_update(&test, (uint32_t[]){ 0, 1 }, 2);
	qt_test_query(&test, 0.0f, 0.0f, 2.0f, 2.0f, (uint32_t[]){ 0, 1 }, 2);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_collide_none(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, -10.0f, -10.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 10.0f, 10.0f, 1.0f, 1.0f);

	qt_test_collide(&test, NULL, 0);
	assert_eq(test.qt.nodes[0].type, QUADTREE_NODE_TYPE_LEAF);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_query_none(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);


	qt_test_query(&test, 0.0f, 0.0f, 1000.0f, 1000.0f, NULL, 0);
	assert_eq(test.qt.nodes[0].type, QUADTREE_NODE_TYPE_LEAF);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_remove_one(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 0.0f, 0.0f, 1.0f, 1.0f);
	qt_test_query(&test, test.qt.half_extent.x, test.qt.half_extent.y, test.qt.half_extent.w, test.qt.half_extent.h, (uint32_t[]){ 0 }, 1);

	qt_test_remove(&test, 0);
	qt_test_query(&test, test.qt.half_extent.x, test.qt.half_extent.y, test.qt.half_extent.w, test.qt.half_extent.h, NULL, 0);

	qt_test_collide(&test, NULL, 0);
	qt_test_update(&test, NULL, 0);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_split_then_merge(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){ .split_threshold = 2, .merge_threshold = 1 }
		);

	qt_test_insert(&test, -31.5f, -31.5f, 0.1f, 0.1f);
	qt_test_normalize(&test);
	assert_eq(test.qt.nodes[0].type, QUADTREE_NODE_TYPE_LEAF);
	assert_eq(test.qt.nodes[0].count, 1);

	qt_test_insert(&test, 31.5f, 31.5f, 0.1f, 0.1f);
	qt_test_normalize(&test);
	assert_neq(test.qt.nodes[0].type, QUADTREE_NODE_TYPE_LEAF);

	qt_test_remove(&test, 1);
	qt_test_normalize(&test);
	assert_eq(test.qt.nodes[0].type, QUADTREE_NODE_TYPE_LEAF);
	assert_eq(test.qt.nodes[0].count, 1);

	qt_test_remove(&test, 0);
	qt_test_normalize(&test);
	assert_eq(test.qt.nodes[0].type, QUADTREE_NODE_TYPE_LEAF);
	assert_eq(test.qt.nodes[0].count, 0);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_lazy_merge_one_at_a_time(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){ .split_threshold = 2, .merge_threshold = 0 }
		);

	qt_test_insert(&test, 0.0f, 0.0f, 64.0f, 64.0f);
	qt_test_insert(&test, 0.0f, 0.0f, 64.0f, 64.0f);
	qt_test_normalize(&test);
	assert_neq(test.qt.nodes[0].type, QUADTREE_NODE_TYPE_LEAF);

	qt_test_remove(&test, 1);
	qt_test_normalize(&test);
	assert_neq(test.qt.nodes[0].type, QUADTREE_NODE_TYPE_LEAF);

	qt_test_remove(&test, 0);
	qt_test_query(&test, test.qt.half_extent.x, test.qt.half_extent.y, test.qt.half_extent.w, test.qt.half_extent.h, NULL, 0);

	/* Two overlapping entities will cause tens of splits, which
	 * can't be merged with one normalization (in query above),
	 * so even though there's no entities, it's not a leaf yet.
	 */
	assert_neq(test.qt.nodes[0].type, QUADTREE_NODE_TYPE_LEAF);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_lazy_merge_successful(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){ .split_threshold = 2, .merge_threshold = 0, .merge_threshold_set = true }
		);

	qt_test_insert(&test, -30.0f, -30.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 30.0f, 30.0f, 1.0f, 1.0f);
	qt_test_normalize(&test);
	assert_neq(test.qt.nodes[0].type, QUADTREE_NODE_TYPE_LEAF);

	qt_test_remove(&test, 1);
	qt_test_normalize(&test);
	assert_neq(test.qt.nodes[0].type, QUADTREE_NODE_TYPE_LEAF);

	qt_test_remove(&test, 0);
	qt_test_query(&test, test.qt.half_extent.x, test.qt.half_extent.y, test.qt.half_extent.w, test.qt.half_extent.h, NULL, 0);
	assert_eq(test.qt.nodes[0].type, QUADTREE_NODE_TYPE_LEAF);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_compaction_on_remove(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, -10.0f, -10.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 0.0f, 0.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 10.0f, 10.0f, 1.0f, 1.0f);
	qt_test_query(&test, test.qt.half_extent.x, test.qt.half_extent.y, test.qt.half_extent.w, test.qt.half_extent.h, (uint32_t[]){ 0, 1, 2 }, 3);

	qt_test_remove(&test, 1);
	qt_test_query(&test, test.qt.half_extent.x, test.qt.half_extent.y, test.qt.half_extent.w, test.qt.half_extent.h, (uint32_t[]){ 0, 2 }, 2);

	qt_test_insert(&test, 20.0f, 20.0f, 1.0f, 1.0f);
	qt_test_query(&test, test.qt.half_extent.x, test.qt.half_extent.y, test.qt.half_extent.w, test.qt.half_extent.h, (uint32_t[]){ 0, 2, 3 }, 3);

	qt_test_remove(&test, 0);
	qt_test_query(&test, test.qt.half_extent.x, test.qt.half_extent.y, test.qt.half_extent.w, test.qt.half_extent.h, (uint32_t[]){ 2, 3 }, 2);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_remove_all_stable_ids(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, -10.0f, -10.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 10.0f, 10.0f, 1.0f, 1.0f);
	qt_test_query(&test, test.qt.half_extent.x, test.qt.half_extent.y, test.qt.half_extent.w, test.qt.half_extent.h, (uint32_t[]){ 0, 1 }, 2);

	qt_test_remove(&test, 1);
	qt_test_query(&test, test.qt.half_extent.x, test.qt.half_extent.y, test.qt.half_extent.w, test.qt.half_extent.h, (uint32_t[]){ 0 }, 1);

	qt_test_remove(&test, 0);
	qt_test_query(&test, test.qt.half_extent.x, test.qt.half_extent.y, test.qt.half_extent.w, test.qt.half_extent.h, (uint32_t[]){}, 0);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_query_no_overlap(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, -20.0f, -20.0f, 5.0f, 5.0f);
	qt_test_insert(&test, 20.0f, 20.0f, 5.0f, 5.0f);

	qt_test_query(&test, 0.0f, 0.0f, 5.0f, 5.0f, NULL, 0);
	qt_test_query(&test, -50.0f, -50.0f, 10.0f, 10.0f, NULL, 0);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_query_partial_overlap(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, -10.0f, -10.0f, 5.0f, 5.0f);
	qt_test_insert(&test, 0.0f, 0.0f, 5.0f, 5.0f);
	qt_test_insert(&test, 10.0f, 10.0f, 5.0f, 5.0f);

	qt_test_query(&test, -5.0f, -5.0f, 8.0f, 8.0f, (uint32_t[]){ 0, 1 }, 2);
	qt_test_query(&test, 5.0f, 5.0f, 8.0f, 8.0f, (uint32_t[]){ 1, 2 }, 2);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_query_center_point_split_entities(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){ .split_threshold = 2 }
		);

	qt_test_insert(&test, 1.0f, 1.0f, 1.0f, 1.0f);
	qt_test_insert(&test, -10.0f, -10.0f, 10.0f, 10.0f);
	qt_test_normalize(&test);
	assert_neq(test.qt.nodes[0].type, QUADTREE_NODE_TYPE_LEAF);

	qt_test_query(&test, 0.0f, 0.0f, 0.0f, 0.0f, (uint32_t[]){ 0, 1 }, 2);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_deep_recursion_collide_once(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){ .split_threshold = 2 }
		);

	qt_test_insert(&test, 0.5f, 0.5f, 0.5f, 0.5f);
	qt_test_insert(&test, -0.5f, -0.5f, 0.5f, 0.5f);
	qt_test_normalize(&test);

	qt_test_collide(&test, (ipair_t[]){ {{ 0, 1 }} }, 1);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_deep_recursion_update_once(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){ .split_threshold = 2 }
		);

	qt_test_insert(&test, 0.5f, 0.5f, 0.5f, 0.5f);
	qt_test_insert(&test, -0.5f, -0.5f, 0.5f, 0.5f);
	qt_test_normalize(&test);

	qt_test_update(&test, (uint32_t[]){ 0, 1 }, 2);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_position_flags_outside_entity(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){ .split_threshold = 2, .merge_threshold = 1 }
		);

	qt_test_insert(&test, 30.0f, 30.0f, 5.0f, 5.0f);
	qt_test_insert(&test, -200.0f, -200.0f, 30.0f, 30.0f);
	qt_test_normalize(&test);
	assert_neq(test.qt.nodes[0].type, QUADTREE_NODE_TYPE_LEAF);

	qt_test_remove(&test, 0);
	qt_test_normalize(&test);
	assert_eq(test.qt.nodes[0].type, QUADTREE_NODE_TYPE_LEAF);
	assert_eq(test.qt.nodes[0].count, 1);

	qt_test_query(&test, 0.0f, 0.0f, 500.0f, 500.0f, (uint32_t[]){ 1 }, 1);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_query_tick(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){ .split_threshold = 3 }
		);

	for(int y = 0; y < 4; ++y)
	{
		for(int x = 0; x < 4; ++x)
		{
			float px = -48.0f + x * 32.0f;
			float py = -48.0f + y * 32.0f;
			qt_test_insert(&test, px, py, 8.0f, 8.0f);
		}
	}
	qt_test_normalize(&test);

	qt_test_query(&test, -32.0f, -32.0f, 24.0f, 24.0f, (uint32_t[]){ 0, 1, 4, 5 }, 4);
	qt_test_query(&test, 32.0f, 32.0f, 24.0f, 24.0f, (uint32_t[]){ 10, 11, 14, 15 }, 4);
	qt_test_query(&test, -32.0f, -32.0f, 24.0f, 24.0f, (uint32_t[]){ 0, 1, 4, 5 }, 4);
	qt_test_query(&test, 0.0f, 0.0f, 24.0f, 24.0f, (uint32_t[]){ 5, 6, 9, 10 }, 4);
	qt_test_query(&test, 32.0f, 32.0f, 24.0f, 24.0f, (uint32_t[]){ 10, 11, 14, 15 }, 4);
	qt_test_query(&test, 0.0f, 0.0f, 24.0f, 24.0f, (uint32_t[]){ 5, 6, 9, 10 }, 4);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_query_nodes_not_split(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 0.0f, 0.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 5.0f, 5.0f, 1.0f, 1.0f);

	assert_eq(test.qt.nodes[0].type, QUADTREE_NODE_TYPE_LEAF);

	qt_test_query_nodes(&test, 0.0f, 0.0f, 64.0f, 64.0f, (uint32_t[]){ 0 }, 1);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_query_nodes_split_once(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){ .split_threshold = 2 }
		);

	qt_test_insert(&test, -30.0f, -30.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 30.0f, 30.0f, 1.0f, 1.0f);
	qt_test_normalize(&test);

	assert_neq(test.qt.nodes[0].type, QUADTREE_NODE_TYPE_LEAF);

	qt_test_query_nodes(&test, 0.0f, 0.0f, 64.0f, 64.0f, (uint32_t[]){ 1, 2, 3, 4 }, 4);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_cascading_merge_on_normalize(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t)
		{
			.split_threshold = 2,
			.merge_threshold = 1,
			.max_depth = 10,
			.min_size = 0.1f,
			.merge_threshold_set = true
		}
		);

	qt_test_insert(&test, 0.0f, 0.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 0.0f, 0.0f, 1.0f, 1.0f);

	uint32_t depth = quadtree_depth(&test.qt);
	assert_eq(depth, 10);

	qt_test_remove(&test, 0);
	qt_test_remove(&test, 1);

	depth = quadtree_depth(&test.qt);
	assert_lt(depth, 10);

	uint32_t prev_depth = depth;
	for(int i = 0; i < 10; ++i)
	{
		quadtree_normalize(&test.qt);
		depth = quadtree_depth(&test.qt);
		if(depth == 1)
		{
			break;
		}

		assert_lt(depth, prev_depth, fprintf(stderr, "Depth failed to decrease on iteration %d (depth: %u)\n", i, depth););
		prev_depth = depth;
	}

	assert_eq(depth, 1);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_no_cascading_merge_on_no_normalize(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t)
		{
			.split_threshold = 2,
			.merge_threshold = 1,
			.max_depth = 10,
			.min_size = 0.1f,
			.merge_threshold_set = true
		}
		);

	qt_test_insert(&test, 0.0f, 0.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 0.0f, 0.0f, 1.0f, 1.0f);

	uint32_t depth = quadtree_depth(&test.qt);
	assert_eq(depth, 10);

	qt_test_remove(&test, 0);
	qt_test_remove(&test, 1);

	uint32_t initial_depth = quadtree_depth(&test.qt);
	assert_lt(initial_depth, 10);

	for(int i = 0; i < 10; ++i)
	{
		depth = quadtree_depth(&test.qt);
		assert_eq(depth, initial_depth, fprintf(stderr, "Depth changed without normalize on iteration %d (expected: %u, got: %u)\n", i, initial_depth, depth););
	}

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_query_circle_one(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 0.0f, 0.0f, 1.0f, 1.0f);
	qt_test_query_circle(&test, 0.0f, 0.0f, 5.0f, (uint32_t[]){ 0 }, 1);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_query_circle_none(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 30.0f, 30.0f, 5.0f, 5.0f);
	qt_test_query_circle(&test, -30.0f, -30.0f, 5.0f, NULL, 0);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_query_circle_partial(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 0.0f, 0.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 10.0f, 0.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 30.0f, 30.0f, 1.0f, 1.0f);
	qt_test_query_circle(&test, 5.0f, 0.0f, 10.0f, (uint32_t[]){ 0, 1 }, 2);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_query_circle_on_boundary(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 10.0f, 0.0f, 1.0f, 1.0f);
	qt_test_query_circle(&test, 0.0f, 0.0f, 9.0f, (uint32_t[]){ 0 }, 1);
	qt_test_query_circle(&test, 0.0f, 0.0f, 8.0f, NULL, 0);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_nearest_rect_one(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 10.0f, 10.0f, 1.0f, 1.0f);

	memset(test.queried, 0, sizeof(test.queried));
	quadtree_nearest_rect(&test.qt, half_to_rect_extent((half_extent_t){ .x = 0.0f, .y = 0.0f, .w = 64.0f, .h = 64.0f }), 1, qt_test_query_fn, NULL);
	assert_eq(test.queried[0], 1);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_nearest_rect_order(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 30.0f, 30.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 5.0f, 5.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 15.0f, 15.0f, 1.0f, 1.0f);

	memset(test.queried, 0, sizeof(test.queried));
	quadtree_nearest_rect(&test.qt, half_to_rect_extent((half_extent_t){ .x = 0.0f, .y = 0.0f, .w = 64.0f, .h = 64.0f }), 1, qt_test_query_fn, NULL);
	assert_eq(test.queried[1], 1);
	assert_eq(test.queried[0], 0);
	assert_eq(test.queried[2], 0);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_nearest_rect_max_results(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 5.0f, 5.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 10.0f, 10.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 15.0f, 15.0f, 1.0f, 1.0f);

	memset(test.queried, 0, sizeof(test.queried));
	quadtree_nearest_rect(&test.qt, half_to_rect_extent((half_extent_t){ .x = 0.0f, .y = 0.0f, .w = 64.0f, .h = 64.0f }), 2, qt_test_query_fn, NULL);
	assert_eq(test.queried[0], 1);
	assert_eq(test.queried[1], 1);
	assert_eq(test.queried[2], 0);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_nearest_rect_zero_results(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 5.0f, 5.0f, 1.0f, 1.0f);

	memset(test.queried, 0, sizeof(test.queried));
	quadtree_nearest_rect(&test.qt, half_to_rect_extent((half_extent_t){ .x = 0.0f, .y = 0.0f, .w = 64.0f, .h = 64.0f }), 0, qt_test_query_fn, NULL);
	assert_eq(test.queried[0], 0);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_nearest_circle_one(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 10.0f, 0.0f, 1.0f, 1.0f);

	memset(test.queried, 0, sizeof(test.queried));
	quadtree_nearest_circle(&test.qt, 0.0f, 0.0f, 100.0f, 1, qt_test_query_fn, NULL);
	assert_eq(test.queried[0], 1);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_nearest_circle_order(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 30.0f, 0.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 10.0f, 0.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 20.0f, 0.0f, 1.0f, 1.0f);

	memset(test.queried, 0, sizeof(test.queried));
	quadtree_nearest_circle(&test.qt, 0.0f, 0.0f, 100.0f, 1, qt_test_query_fn, NULL);
	assert_eq(test.queried[1], 1);
	assert_eq(test.queried[0], 0);
	assert_eq(test.queried[2], 0);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_nearest_circle_max_results(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 10.0f, 0.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 20.0f, 0.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 30.0f, 0.0f, 1.0f, 1.0f);

	memset(test.queried, 0, sizeof(test.queried));
	quadtree_nearest_circle(&test.qt, 0.0f, 0.0f, 100.0f, 2, qt_test_query_fn, NULL);
	assert_eq(test.queried[0], 1);
	assert_eq(test.queried[1], 1);
	assert_eq(test.queried[2], 0);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_nearest_circle_max_distance(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 10.0f, 0.0f, 1.0f, 1.0f);
	qt_test_insert(&test, 30.0f, 0.0f, 1.0f, 1.0f);

	memset(test.queried, 0, sizeof(test.queried));
	quadtree_nearest_circle(&test.qt, 0.0f, 0.0f, 15.0f, 10, qt_test_query_fn, NULL);
	assert_eq(test.queried[0], 1);
	assert_eq(test.queried[1], 0);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_raycast_hit_one(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 20.0f, 0.0f, 5.0f, 5.0f);

	memset(test.queried, 0, sizeof(test.queried));
	quadtree_raycast(&test.qt, 0.0f, 0.0f, 50.0f, 0.0f, qt_test_query_fn, NULL);
	assert_eq(test.queried[0], 1);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_raycast_miss(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 20.0f, 20.0f, 5.0f, 5.0f);

	memset(test.queried, 0, sizeof(test.queried));
	quadtree_raycast(&test.qt, 0.0f, 0.0f, 50.0f, 0.0f, qt_test_query_fn, NULL);
	assert_eq(test.queried[0], 0);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_raycast_length(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 30.0f, 0.0f, 5.0f, 5.0f);

	memset(test.queried, 0, sizeof(test.queried));
	quadtree_raycast(&test.qt, 0.0f, 0.0f, 20.0f, 0.0f, qt_test_query_fn, NULL);
	assert_eq(test.queried[0], 0);

	memset(test.queried, 0, sizeof(test.queried));
	quadtree_raycast(&test.qt, 0.0f, 0.0f, 40.0f, 0.0f, qt_test_query_fn, NULL);
	assert_eq(test.queried[0], 1);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_raycast_multiple(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 64.0f, 64.0f,
		(qt_test_opts_t){}
		);

	qt_test_insert(&test, 10.0f, 0.0f, 3.0f, 3.0f);
	qt_test_insert(&test, 25.0f, 0.0f, 3.0f, 3.0f);
	qt_test_insert(&test, 40.0f, 0.0f, 3.0f, 3.0f);

	memset(test.queried, 0, sizeof(test.queried));
	quadtree_raycast(&test.qt, 0.0f, 0.0f, 50.0f, 0.0f, qt_test_query_fn, NULL);
	assert_eq(test.queried[0], 1);
	assert_eq(test.queried[1], 1);
	assert_eq(test.queried[2], 1);

	qt_test_free(&test);
}
