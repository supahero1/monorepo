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

#include <shared/time.h>
#include <shared/color.h>
#include <shared/event.h>
#include <shared/extent.h>
#include <client/tex/base.h>
#include <client/window/base.h>

#include <stdint.h>


typedef struct vulkan_draw_data
{
	pair_t       pos;
	pair_t       size;
	color_argb_t white_color;
	float        white_depth;
	color_argb_t black_color;
	float        black_depth;
	tex_t        tex;
	float        angle;
	pair_t       tex_scale;
	pair_t       tex_offset;
}
vulkan_draw_data_t;


typedef struct vulkan* vulkan_t;

typedef struct vulkan_init_event_data
{
	vulkan_t vulkan;
}
vulkan_init_event_data_t;

typedef struct vulkan_free_event_data
{
	vulkan_t vulkan;
}
vulkan_free_event_data_t;

typedef struct vulkan_draw_event_data
{
	vulkan_t vulkan;
	float delta;
	float fps;
}
vulkan_draw_event_data_t;

typedef struct vulkan_event_table
{
	event_target_t init_target;
	event_target_t free_target;
	event_target_t draw_target;
}
vulkan_event_table_t;


extern vulkan_t
vulkan_init(
	window_t window,
	time_timers_t timers
	);


extern vulkan_event_table_t*
vulkan_get_event_table(
	vulkan_t vk
	);


extern void
vulkan_add_draw_data(
	vulkan_t vk,
	const vulkan_draw_data_t* data
	);


extern void
vulkan_set_buffering(
	vulkan_t vk,
	uint32_t buffering
	);
