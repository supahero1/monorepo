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
#include <shared/rand.h>
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
	test.qt.min_size = opts.min_size;

	test.qt.merge_threshold_set = opts.merge_threshold_set;

	test.qt.rect_extent = half_to_rect_extent((half_extent_t){ .x = x, .y = y, .w = w, .h = h });
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
	quadtree_insert(&test->qt, half_to_rect_extent(half_extent), &(
		(qt_dyn_test_entity_data_t)
		{
			.idx = test->next_idx++,
			.vx = vx,
			.vy = vy
		}
		));
}


void
qt_test_collide_fn(
	const quadtree_t* qt,
	rect_extent_t* extent_a,
	qt_dyn_test_entity_data_t* data_a,
	rect_extent_t* extent_b,
	qt_dyn_test_entity_data_t* data_b,
	void* user_data
	)
{
	(void) qt;
	(void) user_data;

	float a_center_x = (extent_a->min_x + extent_a->max_x) * 0.5f;
	float a_center_y = (extent_a->min_y + extent_a->max_y) * 0.5f;
	float b_center_x = (extent_b->min_x + extent_b->max_x) * 0.5f;
	float b_center_y = (extent_b->min_y + extent_b->max_y) * 0.5f;

	float dx = a_center_x - b_center_x;
	float dy = a_center_y - b_center_y;

	float abs_dx = fabsf(dx);
	float abs_dy = fabsf(dy);

	if(abs_dx > abs_dy)
	{
		float temp = data_a->vx;
		data_a->vx = data_b->vx;
		data_b->vx = temp;
	}
	else
	{
		float temp = data_a->vy;
		data_a->vy = data_b->vy;
		data_b->vy = temp;
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
	rect_extent_t* extent,
	qt_dyn_test_entity_data_t* data,
	void* user_data
	)
{
	(void) qt;
	(void) user_data;

	extent->min_x += data->vx;
	extent->max_x += data->vx;
	extent->min_y += data->vy;
	extent->max_y += data->vy;

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
	rect_extent_t* extent,
	qt_dyn_test_entity_data_t* data,
	void* user_data
	)
{
	(void) extent;
	(void) user_data;

	qt_test_t* test = (qt_test_t*) qt;
	test->queried[test->queried_count++] = data->idx;

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
test_pass__quadtree_dynamic_init_free(
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
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_test_free(&test);
}


void attr_test_fn
test_pass__quadtree_dynamic_reinsertion_step_on_boundary(
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
test_pass__quadtree_dynamic_reinsertion_step_off_boundary(
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
test_pass__quadtree_dynamic_reinsertion_step_on_boundary_outside(
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
test_pass__quadtree_dynamic_reinsertion_step_off_boundary_outside(
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
test_pass__quadtree_dynamic_removal_during_reinsertion(
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
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_test_insert(&test, -40.0f, -40.0f, 10.0f, 10.0f, 15.0f, 0.0f);

	qt_test_query(&test, -100.0f, -100.0f, 200.0f, 200.0f);
	assert_eq(test.queried_count, 1);

	quadtree_remove(&test.qt, test.qt.data + 1);

	qt_test_update(&test);
	qt_test_normalize(&test);

	qt_test_query(&test, -100.0f, -100.0f, 200.0f, 200.0f);
	assert_eq(test.queried_count, 0);

	qt_test_free(&test);
}


uint32_t
qt_test_find_entity(
	qt_test_t* test,
	uint32_t idx
	)
{
	for(uint32_t i = 1; i < test->qt.entities_used; ++i)
	{
		if(test->qt.entities[i].in_nodes && test->qt.data[i].idx == idx)
		{
			return i;
		}
	}

	return 0;
}


void attr_test_fn
test_pass__quadtree_dynamic_collision_bounce(
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
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_test_insert(&test, -60.0f, 40.0f, 10.0f, 10.0f, 5.0f, 0.0f);
	qt_test_insert(&test, 60.0f, 40.0f, 10.0f, 10.0f, -5.0f, 0.0f);

	qt_test_normalize(&test);

	uint32_t e0 = qt_test_find_entity(&test, 0);
	uint32_t e1 = qt_test_find_entity(&test, 1);

	float initial_vx0 = test.qt.data[e0].vx;
	float initial_vx1 = test.qt.data[e1].vx;

	for(int i = 0; i < 20; ++i)
	{
		qt_test_collide(&test);
		qt_test_update(&test);
		qt_test_normalize(&test);
	}

	e0 = qt_test_find_entity(&test, 0);
	e1 = qt_test_find_entity(&test, 1);

	float final_vx0 = test.qt.data[e0].vx;
	float final_vx1 = test.qt.data[e1].vx;
	assert_true(initial_vx0 != final_vx0 || initial_vx1 != final_vx1);

	float center0_x = (test.qt.entities[e0].extent.min_x + test.qt.entities[e0].extent.max_x) * 0.5f;
	float center1_x = (test.qt.entities[e1].extent.min_x + test.qt.entities[e1].extent.max_x) * 0.5f;
	assert_lt(fabsf(center0_x - center1_x), 150.0f);

	qt_test_free(&test);
}


void attr_test_fn
test_pass__quadtree_dynamic_rapid_cross_tree_movement(
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


void attr_test_fn
test_pass__quadtree_dynamic_regrow_after_shrink(
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
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	for(uint32_t i = 0; i < 1024; ++i)
	{
		qt_test_insert(&test, (i % 32) * 6.0f - 95.0f, (i / 32) * 6.0f - 95.0f, 0.1f, 0.1f, 0.0f, 0.0f);
	}

	qt_test_normalize(&test);

	uint32_t entities_used = test.qt.entities_used;
	for(uint32_t i = 1; i < entities_used; ++i)
	{
		quadtree_remove(&test.qt, test.qt.data + i);
	}

	for(uint32_t i = 0; i < 16 && test.qt.nodes_used > 1; ++i)
	{
		qt_test_normalize(&test);
	}

	for(uint32_t i = 0; i < 1024; ++i)
	{
		qt_test_insert(&test, (i % 32) * 6.0f - 95.0f, (i / 32) * 6.0f - 95.0f, 0.1f, 0.1f, 0.0f, 0.0f);
	}

	qt_test_normalize(&test);

	quadtree_check(&test.qt);

	qt_test_free(&test);
}


void
qt_test_insert_rect(
	qt_test_t* test,
	rect_extent_t extent
	)
{
	quadtree_insert(&test->qt, extent, &(
		(qt_dyn_test_entity_data_t)
		{
			.idx = test->next_idx++
		}
		));
}


void
qt_test_count_collide_fn(
	const quadtree_t* qt,
	rect_extent_t* extent_a,
	qt_dyn_test_entity_data_t* data_a,
	rect_extent_t* extent_b,
	qt_dyn_test_entity_data_t* data_b,
	void* user_data
	)
{
	(void) qt;
	(void) extent_a;
	(void) extent_b;

	qt_test_t* test = user_data;

	assert_lt(test->queried_count + 2, MAX_ENTITIES + 1);
	test->queried[test->queried_count++] = MACRO_MIN(data_a->idx, data_b->idx);
	test->queried[test->queried_count++] = MACRO_MAX(data_a->idx, data_b->idx);
}


void attr_test_fn
test_pass__quadtree_dynamic_collide_after_move_within_leaf(
	void
	)
{
	qt_test_t test = qt_test_init(
		0.0f, 0.0f, 100.0f, 100.0f,
		(qt_test_opts_t)
		{
			.split_threshold = 16,
			.max_depth = 8,
			.dfs_length = 32,
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	/* All in one leaf, sorted by y as 0, 1, 2. Moving 2 below 1 without
	 * crossing any node boundary must still let 0 and 2 collide.
	 */
	qt_test_insert(&test, 0.0f, 5.0f, 10.0f, 5.0f, 0.0f, 0.0f);
	qt_test_insert(&test, 0.0f, 25.0f, 10.0f, 5.0f, 0.0f, 0.0f);
	qt_test_insert(&test, 0.0f, 45.0f, 10.0f, 5.0f, 0.0f, -35.0f);
	qt_test_normalize(&test);

	test.queried_count = 0;
	quadtree_collide(&test.qt, qt_test_count_collide_fn, &test);
	assert_eq(test.queried_count, 0);

	qt_test_update(&test);
	qt_test_normalize(&test);

	test.queried_count = 0;
	quadtree_collide(&test.qt, qt_test_count_collide_fn, &test);
	assert_eq(test.queried_count, 2);
	assert_eq(test.queried[0], 0);
	assert_eq(test.queried[1], 2);

	qt_test_free(&test);
}


void attr_test_fn
test_pass__quadtree_dynamic_non_dyadic_shared_edges(
	void
	)
{
	/* With a size like this, center +- half of neighbouring nodes rounds
	 * to different floats, so the nodes don't share their edges exactly.
	 * Entities lying exactly on a split line must keep a consistent count
	 * of the nodes they are in.
	 */
	float half = 100000.0f / 3.0f;
	float split = half * 0.5f;

	qt_test_t test = qt_test_init(
		0.0f, 0.0f, half, half,
		(qt_test_opts_t)
		{
			.split_threshold = 1,
			.max_depth = 3,
			.dfs_length = 32,
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_test_insert_rect(&test, (rect_extent_t){ .min_x = split - 10.0f, .min_y = 1000.0f, .max_x = split, .max_y = 1010.0f });
	qt_test_insert_rect(&test, (rect_extent_t){ .min_x = -split, .min_y = 1000.0f, .max_x = -split + 10.0f, .max_y = 1010.0f });
	qt_test_insert_rect(&test, (rect_extent_t){ .min_x = 1000.0f, .min_y = split - 10.0f, .max_x = 1010.0f, .max_y = split });
	qt_test_insert_rect(&test, (rect_extent_t){ .min_x = 1000.0f, .min_y = -split, .max_x = 1010.0f, .max_y = -split + 10.0f });
	qt_test_normalize(&test);
	quadtree_check(&test.qt);

	for(int i = 0; i < 4; ++i)
	{
		qt_test_update(&test);
		qt_test_normalize(&test);
		quadtree_check(&test.qt);
	}

	qt_test_query(&test, 0.0f, 0.0f, half, half);
	assert_eq(test.queried_count, 4);

	qt_test_free(&test);
}


#define QT_SIM_MAX_ENTITIES 1024


typedef struct qt_sim
{
	qt_test_t test;
	float half_size;
	bool push_apart;
	bool freeze_odd;

	uint32_t entities_count;
	uint32_t alive_count;
	bool alive[QT_SIM_MAX_ENTITIES];
	rect_extent_t extents[QT_SIM_MAX_ENTITIES];
	qt_dyn_test_entity_data_t* found[QT_SIM_MAX_ENTITIES];
	uint8_t pairs[QT_SIM_MAX_ENTITIES][QT_SIM_MAX_ENTITIES];
}
qt_sim_t;

qt_sim_t qt_sim;


qt_sim_t*
qt_sim_init(
	uint32_t seed,
	float half_size,
	qt_test_opts_t opts
	)
{
	qt_sim_t* sim = &qt_sim;
	memset(sim, 0, sizeof(*sim));

	rand_set_seed(seed);

	sim->test = qt_test_init(0.0f, 0.0f, half_size, half_size, opts);
	sim->half_size = half_size;

	return sim;
}


void
qt_sim_insert(
	qt_sim_t* sim,
	float min_size,
	float max_size,
	float speed
	)
{
	uint32_t idx = sim->entities_count++;
	assert_lt(idx, QT_SIM_MAX_ENTITIES);
	assert_eq(idx, sim->test.next_idx);

	float w = (min_size + rand_f32() * (max_size - min_size)) * 0.5f;
	float h = (min_size + rand_f32() * (max_size - min_size)) * 0.5f;
	float x = (rand_f32() * 2.0f - 1.0f) * (sim->half_size - w);
	float y = (rand_f32() * 2.0f - 1.0f) * (sim->half_size - h);
	float vx = (rand_f32() * 2.0f - 1.0f) * speed;
	float vy = (rand_f32() * 2.0f - 1.0f) * speed;

	qt_test_insert(&sim->test, x, y, w, h, vx, vy);

	sim->extents[idx] = half_to_rect_extent((half_extent_t){ .x = x, .y = y, .w = w, .h = h });
	sim->alive[idx] = true;
	++sim->alive_count;
}


void
qt_sim_insert_many(
	qt_sim_t* sim,
	uint32_t count,
	float min_size,
	float max_size,
	float big_min_size,
	float big_max_size,
	float speed
	)
{
	for(uint32_t i = 0; i < count; ++i)
	{
		if(i % 10 == 9)
		{
			qt_sim_insert(sim, big_min_size, big_max_size, speed);
		}
		else
		{
			qt_sim_insert(sim, min_size, max_size, speed);
		}
	}
}


bool
qt_sim_extent_eq(
	rect_extent_t a,
	rect_extent_t b
	)
{
	return a.min_x == b.min_x && a.min_y == b.min_y && a.max_x == b.max_x && a.max_y == b.max_y;
}


quadtree_status_t
qt_sim_update_fn(
	quadtree_t* qt,
	rect_extent_t* extent,
	qt_dyn_test_entity_data_t* data,
	void* user_data
	)
{
	(void) qt;

	qt_sim_t* sim = user_data;
	assert_lt(data->idx, sim->entities_count);

	if(sim->freeze_odd && (data->idx & 1))
	{
		return QUADTREE_STATUS_NOT_CHANGED;
	}

	extent->min_x += data->vx;
	extent->max_x += data->vx;
	extent->min_y += data->vy;
	extent->max_y += data->vy;

	if(extent->min_x < -sim->half_size)
	{
		float shift = -sim->half_size - extent->min_x;
		extent->min_x += shift;
		extent->max_x += shift;
		data->vx = fabsf(data->vx);
	}
	else if(extent->max_x > sim->half_size)
	{
		float shift = extent->max_x - sim->half_size;
		extent->min_x -= shift;
		extent->max_x -= shift;
		data->vx = -fabsf(data->vx);
	}

	if(extent->min_y < -sim->half_size)
	{
		float shift = -sim->half_size - extent->min_y;
		extent->min_y += shift;
		extent->max_y += shift;
		data->vy = fabsf(data->vy);
	}
	else if(extent->max_y > sim->half_size)
	{
		float shift = extent->max_y - sim->half_size;
		extent->min_y -= shift;
		extent->max_y -= shift;
		data->vy = -fabsf(data->vy);
	}

	sim->extents[data->idx] = *extent;

	return QUADTREE_STATUS_CHANGED;
}


void
qt_sim_tick(
	qt_sim_t* sim
	)
{
	quadtree_update(&sim->test.qt, qt_sim_update_fn, sim);
	quadtree_normalize(&sim->test.qt);
	quadtree_check(&sim->test.qt);
}


void
qt_sim_collide_fn(
	const quadtree_t* qt,
	rect_extent_t* extent_a,
	qt_dyn_test_entity_data_t* data_a,
	rect_extent_t* extent_b,
	qt_dyn_test_entity_data_t* data_b,
	void* user_data
	)
{
	(void) qt;

	qt_sim_t* sim = user_data;

	uint32_t a = data_a->idx;
	uint32_t b = data_b->idx;

	assert_neq(a, b);
	assert_lt(a, sim->entities_count);
	assert_lt(b, sim->entities_count);
	assert_true(sim->alive[a]);
	assert_true(sim->alive[b]);
	assert_true(qt_sim_extent_eq(*extent_a, sim->extents[a]));
	assert_true(qt_sim_extent_eq(*extent_b, sim->extents[b]));

	++sim->pairs[MACRO_MIN(a, b)][MACRO_MAX(a, b)];

	if(!sim->push_apart)
	{
		return;
	}

	float overlap_x = MACRO_MIN(extent_a->max_x, extent_b->max_x) - MACRO_MAX(extent_a->min_x, extent_b->min_x);
	float overlap_y = MACRO_MIN(extent_a->max_y, extent_b->max_y) - MACRO_MAX(extent_a->min_y, extent_b->min_y);

	if(overlap_x < overlap_y)
	{
		float push = overlap_x * 0.5f + 0.01f;
		if(extent_a->min_x + extent_a->max_x < extent_b->min_x + extent_b->max_x)
		{
			push = -push;
		}

		extent_a->min_x += push;
		extent_a->max_x += push;
		extent_b->min_x -= push;
		extent_b->max_x -= push;
	}
	else
	{
		float push = overlap_y * 0.5f + 0.01f;
		if(extent_a->min_y + extent_a->max_y < extent_b->min_y + extent_b->max_y)
		{
			push = -push;
		}

		extent_a->min_y += push;
		extent_a->max_y += push;
		extent_b->min_y -= push;
		extent_b->max_y -= push;
	}

	sim->extents[a] = *extent_a;
	sim->extents[b] = *extent_b;
}


bool
qt_sim_overlaps(
	rect_extent_t a,
	rect_extent_t b
	)
{
	return a.max_x >= b.min_x && b.max_x >= a.min_x && a.max_y >= b.min_y && b.max_y >= a.min_y;
}


void
qt_sim_collide_check(
	qt_sim_t* sim
	)
{
	for(uint32_t a = 0; a < sim->entities_count; ++a)
	{
		memset(sim->pairs[a], 0, sim->entities_count);
	}

	quadtree_collide(&sim->test.qt, qt_sim_collide_fn, sim);

	for(uint32_t a = 0; a < sim->entities_count; ++a)
	{
		for(uint32_t b = a + 1; b < sim->entities_count; ++b)
		{
			bool expected = sim->alive[a] && sim->alive[b] &&
				qt_sim_overlaps(sim->extents[a], sim->extents[b]);

			assert_eq(sim->pairs[a][b], expected);
		}
	}
}


quadtree_status_t
qt_sim_query_fn(
	quadtree_t* qt,
	rect_extent_t* extent,
	qt_dyn_test_entity_data_t* data,
	void* user_data
	)
{
	(void) qt;

	qt_sim_t* sim = user_data;
	uint32_t idx = data->idx;

	assert_lt(idx, sim->entities_count);
	assert_true(sim->alive[idx]);
	assert_null(sim->found[idx]);
	assert_true(qt_sim_extent_eq(*extent, sim->extents[idx]));

	sim->found[idx] = data;

	return QUADTREE_STATUS_NOT_CHANGED;
}


void
qt_sim_query_check(
	qt_sim_t* sim
	)
{
	memset(sim->found, 0, sizeof(sim->found));

	quadtree_query_rect(&sim->test.qt, sim->test.qt.rect_extent, qt_sim_query_fn, sim);

	uint32_t found = 0;
	for(uint32_t idx = 0; idx < sim->entities_count; ++idx)
	{
		found += sim->found[idx] != NULL;
	}

	assert_eq(found, sim->alive_count);
}


void
qt_sim_remove(
	qt_sim_t* sim,
	uint32_t idx
	)
{
	assert_true(sim->alive[idx]);
	assert_not_null(sim->found[idx]);

	quadtree_remove(&sim->test.qt, sim->found[idx]);

	sim->alive[idx] = false;
	--sim->alive_count;
}


void attr_test_fn
test_pass__quadtree_dynamic_collide_matches_brute_force(
	void
	)
{
	qt_sim_t* sim = qt_sim_init(1, 1000.0f,
		(qt_test_opts_t)
		{
			.split_threshold = 8,
			.max_depth = 8,
			.dfs_length = 32,
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_sim_insert_many(sim, 300, 4.0f, 24.0f, 60.0f, 300.0f, 6.0f);
	quadtree_normalize(&sim->test.qt);

	for(int i = 0; i < 30; ++i)
	{
		qt_sim_collide_check(sim);
		qt_sim_tick(sim);
		qt_sim_query_check(sim);
	}

	qt_test_free(&sim->test);
}


void attr_test_fn
test_pass__quadtree_dynamic_collide_matches_brute_force_large_leaf(
	void
	)
{
	/* The root never splits, so one leaf holds more entities than the
	 * collision buffers start with, and they reorder as they move.
	 */
	qt_sim_t* sim = qt_sim_init(2, 300.0f,
		(qt_test_opts_t)
		{
			.split_threshold = 8,
			.max_depth = 1,
			.dfs_length = 32,
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_sim_insert_many(sim, 600, 4.0f, 30.0f, 4.0f, 30.0f, 3.0f);
	quadtree_normalize(&sim->test.qt);

	for(int i = 0; i < 8; ++i)
	{
		qt_sim_collide_check(sim);
		qt_sim_tick(sim);
		qt_sim_query_check(sim);
	}

	qt_test_free(&sim->test);
}


void attr_test_fn
test_pass__quadtree_dynamic_not_changed_entities_stay(
	void
	)
{
	qt_sim_t* sim = qt_sim_init(3, 1000.0f,
		(qt_test_opts_t)
		{
			.split_threshold = 8,
			.max_depth = 8,
			.dfs_length = 32,
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	sim->freeze_odd = true;

	qt_sim_insert_many(sim, 300, 4.0f, 24.0f, 60.0f, 300.0f, 6.0f);
	quadtree_normalize(&sim->test.qt);

	for(int i = 0; i < 30; ++i)
	{
		qt_sim_collide_check(sim);
		qt_sim_tick(sim);
		qt_sim_query_check(sim);
	}

	qt_test_free(&sim->test);
}


void attr_test_fn
test_pass__quadtree_dynamic_remove_before_update(
	void
	)
{
	/* Entities can be moved to other slots during the update, so removals
	 * queued before it must still find them.
	 */
	qt_sim_t* sim = qt_sim_init(4, 1000.0f,
		(qt_test_opts_t)
		{
			.split_threshold = 8,
			.max_depth = 8,
			.dfs_length = 32,
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_sim_insert_many(sim, 200, 4.0f, 24.0f, 60.0f, 300.0f, 6.0f);
	quadtree_normalize(&sim->test.qt);

	for(int i = 0; i < 3; ++i)
	{
		qt_sim_query_check(sim);

		for(uint32_t idx = i; idx < sim->entities_count; idx += 3)
		{
			if(sim->alive[idx] && rand_bool())
			{
				qt_sim_remove(sim, idx);
			}
		}

		qt_sim_tick(sim);
		qt_sim_query_check(sim);
		qt_sim_collide_check(sim);
	}

	qt_test_free(&sim->test);
}


void attr_test_fn
test_pass__quadtree_dynamic_churn(
	void
	)
{
	/* Removes some entities and inserts more every tick, so the tree keeps
	 * growing past the room it reserved for its entities.
	 */
	qt_sim_t* sim = qt_sim_init(5, 1000.0f,
		(qt_test_opts_t)
		{
			.split_threshold = 8,
			.max_depth = 8,
			.dfs_length = 32,
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_sim_insert_many(sim, 200, 4.0f, 24.0f, 60.0f, 300.0f, 6.0f);
	quadtree_normalize(&sim->test.qt);

	for(int i = 0; i < 40; ++i)
	{
		qt_sim_query_check(sim);

		for(int j = 0; j < 5; ++j)
		{
			uint32_t idx = rand_u32() % sim->entities_count;
			if(sim->alive[idx])
			{
				qt_sim_remove(sim, idx);
			}
		}

		qt_sim_insert_many(sim, 15, 4.0f, 24.0f, 60.0f, 300.0f, 6.0f);

		qt_sim_tick(sim);
		qt_sim_query_check(sim);
		qt_sim_collide_check(sim);
	}

	qt_test_free(&sim->test);
}


void attr_test_fn
test_pass__quadtree_dynamic_collide_push_apart(
	void
	)
{
	/* The collision callback moves entities, which the next update and
	 * normalize have to pick up.
	 */
	qt_sim_t* sim = qt_sim_init(6, 400.0f,
		(qt_test_opts_t)
		{
			.split_threshold = 8,
			.max_depth = 8,
			.dfs_length = 32,
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	sim->push_apart = true;

	qt_sim_insert_many(sim, 400, 4.0f, 24.0f, 40.0f, 120.0f, 4.0f);
	quadtree_normalize(&sim->test.qt);

	for(int i = 0; i < 30; ++i)
	{
		quadtree_collide(&sim->test.qt, qt_sim_collide_fn, sim);
		qt_sim_tick(sim);
		qt_sim_query_check(sim);
	}

	qt_test_free(&sim->test);
}


typedef struct qt_test_count
{
	qt_test_t* test;
	uint32_t count;
	uint64_t seen;
}
qt_test_count_t;


quadtree_status_t
qt_test_count_query_fn(
	quadtree_t* qt,
	rect_extent_t* extent,
	qt_dyn_test_entity_data_t* data,
	void* user_data
	)
{
	(void) qt;
	(void) extent;

	qt_test_count_t* count = user_data;

	assert_lt(data->idx, 64);
	assert_false(count->seen & (UINT64_C(1) << data->idx));

	count->seen |= UINT64_C(1) << data->idx;
	++count->count;

	return QUADTREE_STATUS_NOT_CHANGED;
}


uint32_t
qt_test_count_all(
	qt_test_t* test
	)
{
	qt_test_count_t count = { .test = test };
	quadtree_query_rect(&test->qt, test->qt.rect_extent, qt_test_count_query_fn, &count);

	return count.count;
}


void
qt_test_insert_grid(
	qt_test_t* test,
	uint32_t count
	)
{
	for(uint32_t i = 0; i < count; ++i)
	{
		float x = (i % 8) * 24.0f - 84.0f;
		float y = (i / 8) * 24.0f - 84.0f;
		float vx = (i % 3) * 3.0f - 3.0f;
		float vy = (i % 5) * 2.0f - 4.0f;

		qt_test_insert(test, x, y, 8.0f, 8.0f, vx, vy);
	}

	qt_test_normalize(test);
}


quadtree_status_t
qt_test_query_in_update_fn(
	quadtree_t* qt,
	rect_extent_t* extent,
	qt_dyn_test_entity_data_t* data,
	void* user_data
	)
{
	qt_test_t* test = user_data;

	/* The tree doesn't change until the update is over, so everything is
	 * still there exactly once, wherever the update has moved it in memory.
	 */
	assert_eq(qt_test_count_all(test), 40);

	if(data->idx == 0)
	{
		qt_test_insert(test, 0.0f, 0.0f, 4.0f, 4.0f, 0.0f, 0.0f);
	}
	else if(data->idx == 1)
	{
		quadtree_remove(qt, data);
	}

	extent->min_x += data->vx;
	extent->max_x += data->vx;
	extent->min_y += data->vy;
	extent->max_y += data->vy;

	return QUADTREE_STATUS_CHANGED;
}


void attr_test_fn
test_pass__quadtree_dynamic_query_in_update(
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
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_test_insert_grid(&test, 40);

	quadtree_update(&test.qt, qt_test_query_in_update_fn, &test);
	qt_test_normalize(&test);
	quadtree_check(&test.qt);

	assert_eq(qt_test_count_all(&test), 40);

	qt_test_free(&test);
}


void
qt_test_query_in_collide_fn(
	const quadtree_t* qt,
	rect_extent_t* extent_a,
	qt_dyn_test_entity_data_t* data_a,
	rect_extent_t* extent_b,
	qt_dyn_test_entity_data_t* data_b,
	void* user_data
	)
{
	(void) qt;
	(void) extent_a;
	(void) extent_b;
	(void) data_b;

	qt_test_count_t* collisions = user_data;
	qt_test_t* test = collisions->test;

	++collisions->count;

	if(collisions->count == 1)
	{
		qt_test_insert(test, 50.0f, 50.0f, 4.0f, 4.0f, 0.0f, 0.0f);
		quadtree_remove(&test->qt, data_a);
	}

	assert_eq(qt_test_count_all(test), 8);
}


void attr_test_fn
test_pass__quadtree_dynamic_query_in_collide(
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
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	/* Four overlapping pairs */
	for(uint32_t i = 0; i < 4; ++i)
	{
		float x = i * 40.0f - 60.0f;
		qt_test_insert(&test, x, 0.0f, 5.0f, 5.0f, 0.0f, 0.0f);
		qt_test_insert(&test, x + 4.0f, 4.0f, 5.0f, 5.0f, 0.0f, 0.0f);
	}

	qt_test_normalize(&test);

	qt_test_count_t collisions = { .test = &test };
	quadtree_collide(&test.qt, qt_test_query_in_collide_fn, &collisions);
	assert_eq(collisions.count, 4);

	assert_eq(qt_test_count_all(&test), 8);
	quadtree_check(&test.qt);

	qt_test_free(&test);
}


quadtree_status_t
qt_test_query_in_node_query_fn(
	quadtree_t* qt,
	const quadtree_node_info_t* info,
	void* user_data
	)
{
	qt_test_count_t* count = user_data;

	qt_test_count_t found = { .test = count->test };
	quadtree_query_rect(qt, info->extent, qt_test_count_query_fn, &found);
	count->count += found.count;

	return QUADTREE_STATUS_NOT_CHANGED;
}


quadtree_status_t
qt_test_count_node_query_fn(
	quadtree_t* qt,
	const quadtree_node_info_t* info,
	void* user_data
	)
{
	(void) qt;
	(void) info;

	qt_test_count_t* count = user_data;
	++count->count;

	return QUADTREE_STATUS_NOT_CHANGED;
}


quadtree_status_t
qt_test_node_query_in_query_fn(
	quadtree_t* qt,
	rect_extent_t* extent,
	qt_dyn_test_entity_data_t* data,
	void* user_data
	)
{
	(void) data;

	qt_test_count_t* count = user_data;
	++count->count;

	qt_test_count_t nodes = { .test = count->test };
	quadtree_query_nodes_rect(qt, *extent, qt_test_count_node_query_fn, &nodes);
	assert_ge(nodes.count, 1);

	return QUADTREE_STATUS_NOT_CHANGED;
}


void attr_test_fn
test_pass__quadtree_dynamic_query_nodes_nesting(
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
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_test_insert_grid(&test, 40);

	/* Node queries don't mark entities, so they can go inside a query and
	 * a query can go inside them.
	 */
	qt_test_count_t count = { .test = &test };
	quadtree_query_rect(&test.qt, test.qt.rect_extent, qt_test_node_query_in_query_fn, &count);
	assert_eq(count.count, 40);

	count = (qt_test_count_t){ .test = &test };
	quadtree_query_nodes_rect(&test.qt, test.qt.rect_extent, qt_test_query_in_node_query_fn, &count);
	assert_ge(count.count, 40);

	qt_test_free(&test);
}


quadtree_status_t
qt_test_query_in_query_fn(
	quadtree_t* qt,
	rect_extent_t* extent,
	qt_dyn_test_entity_data_t* data,
	void* user_data
	)
{
	(void) data;

	qt_test_count_t count = { .test = user_data };
	quadtree_query_rect(qt, *extent, qt_test_count_query_fn, &count);

	return QUADTREE_STATUS_NOT_CHANGED;
}


void attr_test_fn
test_fail__quadtree_dynamic_query_in_query(
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
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_test_insert_grid(&test, 40);

	/* Queries share one tick to skip entities they already reported */
	quadtree_query_circle(&test.qt, 0.0f, 0.0f, 50.0f, qt_test_query_in_query_fn, &test);

	qt_test_free(&test);
}


quadtree_status_t
qt_test_normalize_in_update_fn(
	quadtree_t* qt,
	rect_extent_t* extent,
	qt_dyn_test_entity_data_t* data,
	void* user_data
	)
{
	(void) extent;
	(void) data;
	(void) user_data;

	quadtree_normalize(qt);

	return QUADTREE_STATUS_NOT_CHANGED;
}


void attr_test_fn
test_fail__quadtree_dynamic_normalize_in_update(
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
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_test_insert_grid(&test, 40);

	quadtree_update(&test.qt, qt_test_normalize_in_update_fn, NULL);

	qt_test_free(&test);
}


quadtree_status_t
qt_test_update_in_query_fn(
	quadtree_t* qt,
	rect_extent_t* extent,
	qt_dyn_test_entity_data_t* data,
	void* user_data
	)
{
	(void) extent;
	(void) data;
	(void) user_data;

	quadtree_update(qt, qt_test_update_fn, NULL);

	return QUADTREE_STATUS_NOT_CHANGED;
}


void attr_test_fn
test_fail__quadtree_dynamic_update_in_query(
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
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_test_insert_grid(&test, 40);

	quadtree_query_rect(&test.qt, test.qt.rect_extent, qt_test_update_in_query_fn, NULL);

	qt_test_free(&test);
}


void
qt_test_collide_in_collide_fn(
	const quadtree_t* qt,
	rect_extent_t* extent_a,
	qt_dyn_test_entity_data_t* data_a,
	rect_extent_t* extent_b,
	qt_dyn_test_entity_data_t* data_b,
	void* user_data
	)
{
	(void) qt;
	(void) extent_a;
	(void) data_a;
	(void) extent_b;
	(void) data_b;

	qt_test_t* test = user_data;
	quadtree_collide(&test->qt, qt_test_collide_fn, NULL);
}


void attr_test_fn
test_fail__quadtree_dynamic_collide_in_collide(
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
			.min_size = 1.0f,
			.merge_threshold_set = false
		}
	);

	qt_test_insert(&test, 0.0f, 0.0f, 5.0f, 5.0f, 0.0f, 0.0f);
	qt_test_insert(&test, 4.0f, 4.0f, 5.0f, 5.0f, 0.0f, 0.0f);
	qt_test_normalize(&test);

	quadtree_collide(&test.qt, qt_test_collide_in_collide_fn, &test);

	qt_test_free(&test);
}
