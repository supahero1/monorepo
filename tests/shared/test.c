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

#include <unistd.h>


void attr_test_fn
test_normal_pass__test_normal_pass(
	void
	)
{
}


void attr_test_fn
test_normal_fail__test_normal_fail(
	void
	)
{
	assert_unreachable();
}


void attr_test_fn
test_normal_timeout__test_normal_timeout(
	void
	)
{
	test_set_timeout(1);
	sleep(99);
	assert_unreachable();
}
