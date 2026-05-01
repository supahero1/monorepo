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

#include <shared/rand.h>
#include <shared/debug.h>

#include <math.h>


uint32_t rand_seed;


void
rand_set_seed(
	uint32_t seed
	)
{
	rand_seed = seed;
}


uint32_t
rand_u32(
	void
	)
{
	return rand_seed = (1103515245 * rand_seed + 12345) & 0x7FFFFFFF;
}


float
rand_f32(
	void
	)
{
	return (double) rand_u32() / (double) 0x7FFFFFFF;
}


bool
rand_bool(
	void
	)
{
	return (rand_u32() & 64) == 0;
}


float
rand_angle(
	void
	)
{
	return (rand_f32() - 0.5) * M_PI * 2.0;
}
