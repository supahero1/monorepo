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
#include <tests/quadtree_dynamic.h>

#include <math.h>
#include <stdint.h>
#include <string.h>

#define MAX_ENTITIES 16


typedef struct qt_test
{
	quadtree_t qt;
	uint32_t next_idx;
	uint32_t queried[MAX_ENTITIES];
	uint32_t queried_count;
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
	float h,
	float vx,
	float vy
	)
{
	half_extent_t half_extent = { .x = x, .y = y, .w = w, .h = h };
	quadtree_insert(&test->qt, &(
		(qt_dyn_test_entity_data_t)
		{
			.rect_extent = half_to_rect_extent(half_extent),
			.idx = test->next_idx++,
			.vx = vx,
			.vy = vy
		}
		));
}


void
qt_test_collide_fn(
	const quadtree_t* qt,
	quadtree_entity_info_t a,
	quadtree_entity_info_t b,
	void* user_data
	)
{
	(void) qt;
	(void) user_data;

	float a_center_x = (a.data->rect_extent.min_x + a.data->rect_extent.max_x) * 0.5f;
	float a_center_y = (a.data->rect_extent.min_y + a.data->rect_extent.max_y) * 0.5f;
	float b_center_x = (b.data->rect_extent.min_x + b.data->rect_extent.max_x) * 0.5f;
	float b_center_y = (b.data->rect_extent.min_y + b.data->rect_extent.max_y) * 0.5f;

	float dx = a_center_x - b_center_x;
	float dy = a_center_y - b_center_y;

	float abs_dx = fabsf(dx);
	float abs_dy = fabsf(dy);

	if(abs_dx > abs_dy)
	{
		float temp = a.data->vx;
		a.data->vx = b.data->vx;
		b.data->vx = temp;
	}
	else
	{
		float temp = a.data->vy;
		a.data->vy = b.data->vy;
		b.data->vy = temp;
	}
}


void
qt_test_collide(
	qt_test_t* test
	)
{
	quadtree_collide(&test->qt, qt_test_collide_fn, NULL);
}


quadtree_status_t
qt_test_update_fn(
	quadtree_t* qt,
	quadtree_entity_info_t info,
	void* user_data
	)
{
	(void) qt;
	(void) user_data;

	info.data->rect_extent.min_x += info.data->vx;
	info.data->rect_extent.max_x += info.data->vx;
	info.data->rect_extent.min_y += info.data->vy;
	info.data->rect_extent.max_y += info.data->vy;

	return QUADTREE_STATUS_CHANGED;
}


void
qt_test_update(
	qt_test_t* test
	)
{
	quadtree_update(&test->qt, qt_test_update_fn, NULL);
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


quadtree_status_t
qt_test_query_fn(
	quadtree_t* qt,
	quadtree_entity_info_t info,
	void* user_data
	)
{
	(void) user_data;

	qt_test_t* test = (qt_test_t*) qt;
	test->queried[test->queried_count++] = info.idx;

	return QUADTREE_STATUS_NOT_CHANGED;
}


void
qt_test_query(
	qt_test_t* test,
	float x,
	float y,
	float w,
	float h
	)
{
	test->queried_count = 0;
	memset(test->queried, 0, sizeof(test->queried));
	quadtree_query_rect(&test->qt, half_to_rect_extent((half_extent_t){ .x = x, .y = y, .w = w, .h = h }), qt_test_query_fn, NULL);
}


void attr_test_fn
test_normal_pass__quadtree_dynamic_init_free(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 100.0f, 100.0f,
		(qt_test_opts_t)
		{
			.split_threshold = 4,
			.max_depth = 8,
			.dfs_length = 32,
			.merge_ht_size = 64,
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_dynamic_reinsertion_step_on_boundary(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 100.0f, 100.0f,
		(qt_test_opts_t)
		{
			.split_threshold = 1,
			.max_depth = 8,
			.dfs_length = 32,
			.merge_ht_size = 64,
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_test_insert(&test, -15.0f, -40.0f, 10.0f, 10.0f, 10.0f, 0.0f);

	qt_test_query(&test, -50.0f, -50.0f, 50.0f, 50.0f);
	assert_eq(test.queried_count, 1);

	qt_test_query(&test, 50.0f, -50.0f, 50.0f, 50.0f);
	assert_eq(test.queried_count, 0);

	qt_test_update(&test);
	qt_test_normalize(&test);

	qt_test_query(&test, -50.0f, -50.0f, 50.0f, 50.0f);
	uint32_t left_count = test.queried_count;
	assert_eq(left_count, 1);

	qt_test_query(&test, 50.0f, -50.0f, 50.0f, 50.0f);
	uint32_t right_count = test.queried_count;
	assert_eq(right_count, 1);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_dynamic_reinsertion_step_off_boundary(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 100.0f, 100.0f,
		(qt_test_opts_t)
		{
			.split_threshold = 1,
			.max_depth = 8,
			.dfs_length = 32,
			.merge_ht_size = 64,
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_test_insert(&test, -10.0f, -40.0f, 10.0f, 10.0f, 21.0f, 0.0f);

	qt_test_update(&test);
	qt_test_normalize(&test);

	qt_test_query(&test, -50.0f, -50.0f, 50.0f, 50.0f);
	assert_eq(test.queried_count, 0);

	qt_test_query(&test, 50.0f, -50.0f, 50.0f, 50.0f);
	assert_eq(test.queried_count, 1);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_dynamic_reinsertion_step_on_boundary_outside(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 100.0f, 100.0f,
		(qt_test_opts_t)
		{
			.split_threshold = 1,
			.max_depth = 8,
			.dfs_length = 32,
			.merge_ht_size = 64,
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_test_insert(&test, 20.0f, 20.0f, 5.0f, 5.0f, 0.0f, 0.0f);
	qt_test_insert(&test, 110.0f, -15.0f, 10.0f, 10.0f, 0.0f, 10.0f);

	qt_test_query(&test, 50.0f, -50.0f, 50.0f, 50.0f);
	assert_eq(test.queried_count, 1);

	qt_test_query(&test, 50.0f, 50.0f, 50.0f, 50.0f);
	assert_eq(test.queried_count, 1);

	qt_test_update(&test);
	qt_test_normalize(&test);

	qt_test_query(&test, 50.0f, -50.0f, 50.0f, 50.0f);
	uint32_t bottom_count = test.queried_count;
	assert_eq(bottom_count, 1);

	qt_test_query(&test, 50.0f, 50.0f, 50.0f, 50.0f);
	uint32_t top_count = test.queried_count;
	assert_eq(top_count, 2);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_dynamic_reinsertion_step_off_boundary_outside(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 100.0f, 100.0f,
		(qt_test_opts_t)
		{
			.split_threshold = 1,
			.max_depth = 8,
			.dfs_length = 32,
			.merge_ht_size = 64,
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_test_insert(&test, 20.0f, 20.0f, 5.0f, 5.0f, 0.0f, 0.0f);
	qt_test_insert(&test, 110.0f, -10.0f, 10.0f, 10.0f, 0.0f, 21.0f);

	qt_test_update(&test);
	qt_test_normalize(&test);

	qt_test_query(&test, 50.0f, -50.0f, 50.0f, 50.0f);
	assert_eq(test.queried_count, 0);

	qt_test_query(&test, 50.0f, 50.0f, 50.0f, 50.0f);
	assert_eq(test.queried_count, 2);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_dynamic_removal_during_reinsertion(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 100.0f, 100.0f,
		(qt_test_opts_t)
		{
			.split_threshold = 2,
			.max_depth = 8,
			.dfs_length = 32,
			.merge_ht_size = 64,
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_test_insert(&test, -40.0f, -40.0f, 10.0f, 10.0f, 15.0f, 0.0f);

	qt_test_query(&test, -100.0f, -100.0f, 200.0f, 200.0f);
	assert_eq(test.queried_count, 1);

	quadtree_remove(&test.qt, 1);

	qt_test_update(&test);
	qt_test_normalize(&test);

	qt_test_query(&test, -100.0f, -100.0f, 200.0f, 200.0f);
	assert_eq(test.queried_count, 0);

	qt_test_free(&test);
}


qt_dyn_test_entity_data_t*
qt_test_find_entity(
	qt_test_t* test,
	uint32_t idx
	)
{
	for(uint32_t i = 1; i < test->qt.entities_used; ++i)
	{
		qt_dyn_test_entity_data_t* data = &test->qt.entities[i].data;
		if(data->idx == idx)
		{
			return data;
		}
	}

	return NULL;
}


void attr_test_fn
test_normal_pass__quadtree_dynamic_collision_bounce(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 100.0f, 100.0f,
		(qt_test_opts_t)
		{
			.split_threshold = 2,
			.max_depth = 8,
			.dfs_length = 32,
			.merge_ht_size = 64,
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_test_insert(&test, -60.0f, 40.0f, 10.0f, 10.0f, 5.0f, 0.0f);
	qt_test_insert(&test, 60.0f, 40.0f, 10.0f, 10.0f, -5.0f, 0.0f);

	qt_test_normalize(&test);

	qt_dyn_test_entity_data_t* e0 = qt_test_find_entity(&test, 0);
	qt_dyn_test_entity_data_t* e1 = qt_test_find_entity(&test, 1);

	float initial_vx0 = e0->vx;
	float initial_vx1 = e1->vx;

	for(int i = 0; i < 20; ++i)
	{
		qt_test_collide(&test);
		qt_test_update(&test);
		qt_test_normalize(&test);
	}

	e0 = qt_test_find_entity(&test, 0);
	e1 = qt_test_find_entity(&test, 1);

	float final_vx0 = e0->vx;
	float final_vx1 = e1->vx;
	assert_true(initial_vx0 != final_vx0 || initial_vx1 != final_vx1);

	float center0_x = (e0->rect_extent.min_x + e0->rect_extent.max_x) * 0.5f;
	float center1_x = (e1->rect_extent.min_x + e1->rect_extent.max_x) * 0.5f;
	assert_lt(fabsf(center0_x - center1_x), 150.0f);

	qt_test_free(&test);
}


void attr_test_fn
test_normal_pass__quadtree_dynamic_rapid_cross_tree_movement(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 100.0f, 100.0f,
		(qt_test_opts_t)
		{
			.split_threshold = 1,
			.max_depth = 8,
			.dfs_length = 32,
			.merge_ht_size = 64,
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_test_insert(&test, -40.0f, -40.0f, 5.0f, 5.0f, 80.0f, 80.0f);
	qt_test_insert(&test, 40.0f, 40.0f, 5.0f, 5.0f, 0.0f, 0.0f);
	qt_test_normalize(&test);

	qt_test_query(&test, -50.0f, -50.0f, 50.0f, 50.0f);
	assert_eq(test.queried_count, 1);
	qt_test_query(&test, 50.0f, 50.0f, 50.0f, 50.0f);
	assert_eq(test.queried_count, 1);

	qt_test_update(&test);
	qt_test_normalize(&test);

	qt_test_query(&test, -50.0f, -50.0f, 50.0f, 50.0f);
	assert_eq(test.queried_count, 0);
	qt_test_query(&test, 50.0f, 50.0f, 50.0f, 50.0f);
	assert_eq(test.queried_count, 2);

	qt_test_free(&test);
}
