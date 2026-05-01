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
#include <shared/heap.h>
#include <shared/debug.h>
#include <shared/macro.h>

#include <string.h>


typedef struct heap_test_el
{
	int value;
}
heap_test_el_t;


int
heap_test_cmp(
	const heap_test_el_t* a,
	const heap_test_el_t* b
	)
{
	return a->value - b->value;
}


heap_t
heap_test_init(
	void
	)
{
	heap_t heap;
	heap.cmp_fn = (void*) heap_test_cmp;
	heap.el_size = sizeof(heap_test_el_t);
	heap_init(&heap);
	return heap;
}


void
heap_test_push(
	heap_t* heap,
	int value
	)
{
	heap_push(heap, &(heap_test_el_t){ .value = value });
}


int
heap_test_pop(
	heap_t* heap
	)
{
	heap_test_el_t* el = heap_pop(heap);
	if(el == NULL)
	{
		return -1;
	}
	return el->value;
}


int
heap_test_peek(
	heap_t* heap
	)
{
	heap_test_el_t* el = heap_peek(heap);
	if(el == NULL)
	{
		return -1;
	}
	return el->value;
}


void
heap_test_replace(
	heap_t* heap,
	int value
	)
{
	heap_replace(heap, &(heap_test_el_t){ .value = value });
}


void attr_test_fn
test_normal_pass__heap_init_free(
	void
	)
{
	heap_t heap = heap_test_init();
	heap_free(&heap);
}


void attr_test_fn
test_normal_fail__heap_init_null(
	void
	)
{
	heap_init(NULL);
}


void attr_test_fn
test_normal_fail__heap_init_null_cmp_fn(
	void
	)
{
	heap_t heap;
	heap.cmp_fn = NULL;
	heap.el_size = sizeof(heap_test_el_t);
	heap_init(&heap);
}


void attr_test_fn
test_normal_fail__heap_init_zero_el_size(
	void
	)
{
	heap_t heap;
	heap.cmp_fn = (void*) heap_test_cmp;
	heap.el_size = 0;
	heap_init(&heap);
}


void attr_test_fn
test_normal_fail__heap_free_null(
	void
	)
{
	heap_free(NULL);
}


void attr_test_fn
test_normal_fail__heap_push_null_heap(
	void
	)
{
	heap_push(NULL, TEST_PTR);
}


void attr_test_fn
test_normal_fail__heap_push_null_el(
	void
	)
{
	heap_t heap = heap_test_init();
	heap_push(&heap, NULL);
}


void attr_test_fn
test_normal_fail__heap_pop_null(
	void
	)
{
	heap_pop(NULL);
}


void attr_test_fn
test_normal_fail__heap_peek_null(
	void
	)
{
	heap_peek(NULL);
}


void attr_test_fn
test_normal_fail__heap_replace_null_heap(
	void
	)
{
	heap_replace(NULL, TEST_PTR);
}


void attr_test_fn
test_normal_fail__heap_replace_null_el(
	void
	)
{
	heap_t heap = heap_test_init();
	heap_replace(&heap, NULL);
}


void attr_test_fn
test_normal_pass__heap_push_one(
	void
	)
{
	heap_t heap = heap_test_init();

	heap_test_push(&heap, 42);
	assert_eq(heap.used, 1);

	heap_free(&heap);
}


void attr_test_fn
test_normal_pass__heap_pop_empty(
	void
	)
{
	heap_t heap = heap_test_init();

	heap_test_el_t* el = heap_pop(&heap);
	assert_eq(el, NULL);

	heap_free(&heap);
}


void attr_test_fn
test_normal_pass__heap_peek_empty(
	void
	)
{
	heap_t heap = heap_test_init();

	heap_test_el_t* el = heap_peek(&heap);
	assert_eq(el, NULL);

	heap_free(&heap);
}


void attr_test_fn
test_normal_pass__heap_push_pop_one(
	void
	)
{
	heap_t heap = heap_test_init();

	heap_test_push(&heap, 42);
	int val = heap_test_pop(&heap);
	assert_eq(val, 42);
	assert_eq(heap.used, 0);

	heap_free(&heap);
}


void attr_test_fn
test_normal_pass__heap_push_peek_one(
	void
	)
{
	heap_t heap = heap_test_init();

	heap_test_push(&heap, 42);
	int val = heap_test_peek(&heap);
	assert_eq(val, 42);
	assert_eq(heap.used, 1);

	heap_free(&heap);
}


void attr_test_fn
test_normal_pass__heap_min_order(
	void
	)
{
	heap_t heap = heap_test_init();

	heap_test_push(&heap, 5);
	heap_test_push(&heap, 3);
	heap_test_push(&heap, 7);
	heap_test_push(&heap, 1);
	heap_test_push(&heap, 9);

	assert_eq(heap_test_pop(&heap), 1);
	assert_eq(heap_test_pop(&heap), 3);
	assert_eq(heap_test_pop(&heap), 5);
	assert_eq(heap_test_pop(&heap), 7);
	assert_eq(heap_test_pop(&heap), 9);
	assert_eq(heap.used, 0);

	heap_free(&heap);
}


void attr_test_fn
test_normal_pass__heap_reverse_order_insert(
	void
	)
{
	heap_t heap = heap_test_init();

	heap_test_push(&heap, 10);
	heap_test_push(&heap, 9);
	heap_test_push(&heap, 8);
	heap_test_push(&heap, 7);
	heap_test_push(&heap, 6);
	heap_test_push(&heap, 5);
	heap_test_push(&heap, 4);
	heap_test_push(&heap, 3);
	heap_test_push(&heap, 2);
	heap_test_push(&heap, 1);

	for(int i = 1; i <= 10; ++i)
	{
		assert_eq(heap_test_pop(&heap), i);
	}

	heap_free(&heap);
}


void attr_test_fn
test_normal_pass__heap_already_sorted_insert(
	void
	)
{
	heap_t heap = heap_test_init();

	for(int i = 1; i <= 10; ++i)
	{
		heap_test_push(&heap, i);
	}

	for(int i = 1; i <= 10; ++i)
	{
		assert_eq(heap_test_pop(&heap), i);
	}

	heap_free(&heap);
}


void attr_test_fn
test_normal_pass__heap_duplicates(
	void
	)
{
	heap_t heap = heap_test_init();

	heap_test_push(&heap, 5);
	heap_test_push(&heap, 5);
	heap_test_push(&heap, 3);
	heap_test_push(&heap, 5);
	heap_test_push(&heap, 3);

	assert_eq(heap_test_pop(&heap), 3);
	assert_eq(heap_test_pop(&heap), 3);
	assert_eq(heap_test_pop(&heap), 5);
	assert_eq(heap_test_pop(&heap), 5);
	assert_eq(heap_test_pop(&heap), 5);

	heap_free(&heap);
}


void attr_test_fn
test_normal_pass__heap_replace_empty(
	void
	)
{
	heap_t heap = heap_test_init();

	heap_test_replace(&heap, 42);
	assert_eq(heap.used, 0);

	heap_free(&heap);
}


void attr_test_fn
test_normal_pass__heap_replace_single(
	void
	)
{
	heap_t heap = heap_test_init();

	heap_test_push(&heap, 10);
	heap_test_replace(&heap, 5);

	assert_eq(heap_test_peek(&heap), 5);
	assert_eq(heap.used, 1);

	heap_free(&heap);
}


void attr_test_fn
test_normal_pass__heap_replace_maintains_order(
	void
	)
{
	heap_t heap = heap_test_init();

	heap_test_push(&heap, 1);
	heap_test_push(&heap, 5);
	heap_test_push(&heap, 10);

	heap_test_replace(&heap, 7);

	assert_eq(heap_test_pop(&heap), 5);
	assert_eq(heap_test_pop(&heap), 7);
	assert_eq(heap_test_pop(&heap), 10);

	heap_free(&heap);
}


void attr_test_fn
test_normal_pass__heap_interleaved_push_pop(
	void
	)
{
	heap_t heap = heap_test_init();

	heap_test_push(&heap, 5);
	heap_test_push(&heap, 3);
	assert_eq(heap_test_pop(&heap), 3);

	heap_test_push(&heap, 7);
	heap_test_push(&heap, 1);
	assert_eq(heap_test_pop(&heap), 1);
	assert_eq(heap_test_pop(&heap), 5);

	heap_test_push(&heap, 2);
	assert_eq(heap_test_pop(&heap), 2);
	assert_eq(heap_test_pop(&heap), 7);

	heap_free(&heap);
}


void attr_test_fn
test_normal_pass__heap_large_count(
	void
	)
{
	heap_t heap = heap_test_init();

	for(int i = 100; i >= 1; --i)
	{
		heap_test_push(&heap, i);
	}

	assert_eq(heap.used, 100);

	for(int i = 1; i <= 100; ++i)
	{
		assert_eq(heap_test_pop(&heap), i);
	}

	heap_free(&heap);
}


void attr_test_fn
test_normal_pass__heap_peek_doesnt_remove(
	void
	)
{
	heap_t heap = heap_test_init();

	heap_test_push(&heap, 42);

	for(int i = 0; i < 5; ++i)
	{
		assert_eq(heap_test_peek(&heap), 42);
		assert_eq(heap.used, 1);
	}

	heap_free(&heap);
}


void attr_test_fn
test_normal_pass__heap_negative_values(
	void
	)
{
	heap_t heap = heap_test_init();

	heap_test_push(&heap, -5);
	heap_test_push(&heap, 3);
	heap_test_push(&heap, -10);
	heap_test_push(&heap, 0);
	heap_test_push(&heap, 7);

	assert_eq(heap_test_pop(&heap), -10);
	assert_eq(heap_test_pop(&heap), -5);
	assert_eq(heap_test_pop(&heap), 0);
	assert_eq(heap_test_pop(&heap), 3);
	assert_eq(heap_test_pop(&heap), 7);

	heap_free(&heap);
}


void attr_test_fn
test_normal_pass__heap_single_element_many_ops(
	void
	)
{
	heap_t heap = heap_test_init();

	for(int i = 0; i < 10; ++i)
	{
		heap_test_push(&heap, i);
		assert_eq(heap_test_pop(&heap), i);
	}

	assert_eq(heap.used, 0);

	heap_free(&heap);
}
