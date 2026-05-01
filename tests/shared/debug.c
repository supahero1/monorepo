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

#include <shared/attr.h>
#include <shared/debug.h>
#include <shared/macro.h>

#include <stddef.h>


void attr_test_fn
test_normal_pass__assert_eq(
	void
	)
{
	assert_eq(0, 0);
	assert_eq(false, false);

	assert_eq(1, 1);
	assert_eq(true, true);

	assert_eq(-1, -1);

	assert_eq(1.0f, 1.0f);
}


void attr_test_fn
test_normal_fail__assert_eq_int(
	void
	)
{
	assert_eq(0, 1);
}


void attr_test_fn
test_normal_fail__assert_eq_bool(
	void
	)
{
	assert_eq(false, true);
}


void attr_test_fn
test_normal_fail__assert_eq_negative(
	void
	)
{
	assert_eq(1, -1);
}


void attr_test_fn
test_normal_fail__assert_eq_float(
	void
	)
{
	assert_eq(1.0f, 2.0f);
}


void attr_test_fn
test_normal_pass__assert_true(
	void
	)
{
	assert_true(1);
	assert_true(-1);
	assert_true(true);
	assert_true(1.0f);
}


void attr_test_fn
test_normal_fail__assert_true_int(
	void
	)
{
	assert_true(0);
}


void attr_test_fn
test_normal_fail__assert_true_false(
	void
	)
{
	assert_true(false);
}


void attr_test_fn
test_normal_fail__assert_true_float(
	void
	)
{
	assert_true(0.0f);
}


void attr_test_fn
test_normal_pass__assert_false(
	void
	)
{
	assert_false(0);
	assert_false(false);
	assert_false(0.0f);
}


void attr_test_fn
test_normal_fail__assert_false_int(
	void
	)
{
	assert_false(1);
}


void attr_test_fn
test_normal_fail__assert_false_true(
	void
	)
{
	assert_false(true);
}


void attr_test_fn
test_normal_fail__assert_false_negative(
	void
	)
{
	assert_false(-1);
}


void attr_test_fn
test_normal_fail__assert_false_float(
	void
	)
{
	assert_false(1.0f);
}


void attr_test_fn
test_normal_pass__assert_null(
	void
	)
{
	assert_null(NULL);
}


void attr_test_fn
test_normal_fail__assert_null(
	void
	)
{
	assert_null((void *) 1);
}


void attr_test_fn
test_normal_pass__assert_not_null(
	void
	)
{
	/* Expect no casting warnings */
	assert_not_null((void *) 1);
	assert_not_null((const void*) 1);
	assert_not_null((volatile void*) -1);
	assert_not_null((const void* const*) 1);
	assert_not_null((const volatile void*) 1);
	assert_not_null((const void* volatile*) 1);
	assert_not_null((volatile void* const*) -1);
	assert_not_null((volatile void* volatile*) 1);
	assert_not_null((const volatile void* const*) 1);
	assert_not_null((const volatile void* volatile*) -1);
	assert_not_null((const volatile void* const volatile*) 1);
}


void attr_test_fn
test_normal_fail__assert_not_null(
	void
	)
{
	assert_not_null(NULL);
}


void attr_test_fn
test_normal_pass__assert_ptr(
	void
	)
{
	assert_ptr((void *) 1, 0);
	assert_ptr((void *) 1, 1);

	assert_ptr(NULL, 0);
}


void attr_test_fn
test_normal_fail__assert_ptr_null_with_non_zero_size(
	void
	)
{
	assert_ptr(NULL, 1);
}


void attr_test_fn
test_normal_pass__assert_lt(
	void
	)
{
	assert_lt(0, 1);
	assert_lt(-1, 0);
	assert_lt(-2, -1);

	assert_lt((void*) 0, (void*) 1);

	assert_lt(1.0f, 2.0f);
}


void attr_test_fn
test_normal_fail__assert_lt_different_int(
	void
	)
{
	assert_lt(1, 0);
}


void attr_test_fn
test_normal_fail__assert_lt_same_int(
	void
	)
{
	assert_lt(0, 0);
}


void attr_test_fn
test_normal_fail__assert_lt_different_negative_int(
	void
	)
{
	assert_lt(-1, -2);
}


void attr_test_fn
test_normal_fail__assert_lt_same_negative_int(
	void
	)
{
	assert_lt(-1, -1);
}


void attr_test_fn
test_normal_fail__assert_lt_different_ptr(
	void
	)
{
	assert_lt((void*) 1, (void*) 0);
}


void attr_test_fn
test_normal_fail__assert_lt_same_ptr(
	void
	)
{
	assert_lt((void*) 0, (void*) 0);
}


void attr_test_fn
test_normal_fail__assert_lt_different_float(
	void
	)
{
	assert_lt(2.0f, 1.0f);
}


void attr_test_fn
test_normal_fail__assert_lt_same_float(
	void
	)
{
	assert_lt(1.0f, 1.0f);
}


void attr_test_fn
test_normal_pass__assert_le(
	void
	)
{
	assert_le(0, 1);
	assert_le(-1, 0);
	assert_le(-2, -1);

	assert_le(1, 1);
	assert_le(0, 0);
	assert_le(-1, -1);

	assert_le((void*) 0, (void*) 1);
	assert_le((void*) 0, (void*) 0);

	assert_le(1.0f, 2.0f);
	assert_le(1.0f, 1.0f);
}


void attr_test_fn
test_normal_fail__assert_le_different_int(
	void
	)
{
	assert_le(1, 0);
}


void attr_test_fn
test_normal_fail__assert_le_different_negative_int(
	void
	)
{
	assert_le(-1, -2);
}


void attr_test_fn
test_normal_fail__assert_le_different_ptr(
	void
	)
{
	assert_le((void*) 1, (void*) 0);
}


void attr_test_fn
test_normal_fail__assert_le_different_float(
	void
	)
{
	assert_le(2.0f, 1.0f);
}


void attr_test_fn
test_normal_pass__assert_gt(
	void
	)
{
	assert_gt(1, 0);
	assert_gt(0, -1);
	assert_gt(-1, -2);

	assert_gt((void*) 1, (void*) 0);

	assert_gt(2.0f, 1.0f);
}


void attr_test_fn
test_normal_fail__assert_gt_different_int(
	void
	)
{
	assert_gt(0, 1);
}


void attr_test_fn
test_normal_fail__assert_gt_same_int(
	void
	)
{
	assert_gt(0, 0);
}


void attr_test_fn
test_normal_fail__assert_gt_different_negative_int(
	void
	)
{
	assert_gt(-2, -1);
}


void attr_test_fn
test_normal_fail__assert_gt_same_negative_int(
	void
	)
{
	assert_gt(-1, -1);
}


void attr_test_fn
test_normal_fail__assert_gt_different_ptr(
	void
	)
{
	assert_gt((void*) 0, (void*) 1);
}


void attr_test_fn
test_normal_fail__assert_gt_same_ptr(
	void
	)
{
	assert_gt((void*) 0, (void*) 0);
}


void attr_test_fn
test_normal_fail__assert_gt_different_float(
	void
	)
{
	assert_gt(1.0f, 2.0f);
}


void attr_test_fn
test_normal_fail__assert_gt_same_float(
	void
	)
{
	assert_gt(1.0f, 1.0f);
}


void attr_test_fn
test_normal_pass__assert_ge(
	void
	)
{
	assert_ge(1, 0);
	assert_ge(0, -1);
	assert_ge(-1, -2);

	assert_ge(1, 1);
	assert_ge(0, 0);
	assert_ge(-1, -1);

	assert_ge((void*) 1, (void*) 0);
	assert_ge((void*) 0, (void*) 0);

	assert_ge(2.0f, 1.0f);
	assert_ge(1.0f, 1.0f);
}


void attr_test_fn
test_normal_fail__assert_ge_different_int(
	void
	)
{
	assert_ge(0, 1);
}


void attr_test_fn
test_normal_fail__assert_ge_different_negative_int(
	void
	)
{
	assert_ge(-2, -1);
}


void attr_test_fn
test_normal_fail__assert_ge_different_ptr(
	void
	)
{
	assert_ge((void*) 0, (void*) 1);
}


void attr_test_fn
test_normal_fail__assert_ge_different_float(
	void
	)
{
	assert_ge(1.0f, 2.0f);
}


void attr_test_fn
test_normal_pass__assert_unreachable(
	void
	)
{
	if(0)
	{
		assert_unreachable();
	}
}


void attr_test_fn
test_normal_fail__assert_unreachable(
	void
	)
{
	assert_unreachable();
}


void attr_test_fn
test_normal_pass__assert_log(
	void
	)
{
	assert_log("test");
}
