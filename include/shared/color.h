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

#pragma once

#include <shared/extent.h>

#include <stdint.h>


typedef struct color_hsv
{
	float h;
	float s;
	float v;
}
color_hsv_t;


typedef struct color_argb
{
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t a;
}
color_argb_t;


extern color_hsv_t
color_argb_to_hsv(
	color_argb_t argb
	);


extern color_argb_t
color_hsv_to_argb(
	color_hsv_t hsv
	);


extern pair_t
color_hsv_to_pos(
	color_hsv_t hsv
	);


extern color_hsv_t
color_pos_to_hsv(
	pair_t pos
	);


extern uint8_t
color_a_mul_a(
	uint8_t a_1,
	uint8_t a_2
	);


extern color_argb_t
color_argb_mul_a(
	color_argb_t argb,
	uint8_t a
	);
