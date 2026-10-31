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

#include <tests/quadtree_static.h>

#include <string.h>


bool qt_test_simd = true;


int
qt_test_cpu_supports(
	const char* feature
	)
{
	if(!qt_test_simd)
	{
		return 0;
	}

	if(!strcmp(feature, "avx"))
	{
		return __builtin_cpu_supports("avx");
	}

	if(!strcmp(feature, "sse"))
	{
		return __builtin_cpu_supports("sse");
	}

	return 0;
}

#define __builtin_cpu_supports(feature) qt_test_cpu_supports(feature)

#include <shared/quadtree.c>
