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

#include <shared/color.h>
#include <shared/event.h>
#include <shared/extent.h>
#include <client/window/base.h>

#include <stdint.h>


typedef enum ui_align : uint8_t
{
	UI_ALIGN_TOP,
	UI_ALIGN_LEFT = UI_ALIGN_TOP,

	UI_ALIGN_MIDDLE,
	UI_ALIGN_CENTER = UI_ALIGN_MIDDLE,

	UI_ALIGN_BOTTOM,
	UI_ALIGN_RIGHT = UI_ALIGN_BOTTOM
}
ui_align_t;

typedef enum ui_axis : uint8_t
{
	UI_AXIS_VERTICAL,
	UI_AXIS_HORIZONTAL
}
ui_axis_t;

typedef enum ui_position : uint8_t
{
	UI_POSITION_INHERIT,
	UI_POSITION_ABSOLUTE,
	UI_POSITION_RELATIVE
}
ui_position_t;


typedef struct ui* ui_t;

typedef struct ui_element ui_element_t;

typedef union ui_pos
{
	pair_t abs;
	pair_t rel;
}
ui_pos_t;


typedef struct ui_mouse_down_event_data
{
	ui_t ui;
	ui_element_t* element;
	window_button_t button;
	ui_pos_t pos;
	uint8_t clicks;
}
ui_mouse_down_event_data_t;

typedef struct ui_mouse_up_event_data
{
	ui_t ui;
	ui_element_t* element;
	window_button_t button;
	ui_pos_t pos;
}
ui_mouse_up_event_data_t;

typedef struct ui_mouse_move_event_data
{
	ui_t ui;
	ui_element_t* element;
	ui_pos_t old_pos;
	ui_pos_t new_pos;
}
ui_mouse_move_event_data_t;

typedef struct ui_mouse_in_event_data
{
	ui_t ui;
	ui_element_t* element;
	ui_pos_t pos;
}
ui_mouse_in_event_data_t;

typedef struct ui_mouse_out_event_data
{
	ui_t ui;
	ui_element_t* element;
	ui_pos_t pos;
}
ui_mouse_out_event_data_t;

typedef struct ui_mouse_scroll_event_data
{
	ui_t ui;
	ui_element_t* element;
	pair_t offset;
}
ui_mouse_scroll_event_data_t;

typedef struct ui_resize_event_data
{
	ui_t ui;
	ui_element_t* element;
	pair_t old_size;
	pair_t new_size;
}
ui_resize_event_data_t;

typedef struct ui_change_event_data
{
	ui_t ui;
	ui_element_t* element;
}
ui_change_event_data_t;

typedef struct ui_submit_event_data
{
	ui_t ui;
	ui_element_t* element;
}
ui_submit_event_data_t;

typedef struct ui_free_event_data
{
	ui_t ui;
	ui_element_t* element;
}
ui_free_event_data_t;

typedef struct ui_text_select_all_event_data
{
	ui_t ui;
	ui_element_t* element;
}
ui_text_select_all_event_data_t;

typedef struct ui_text_move_event_data
{
	ui_t ui;
	ui_element_t* element;
	int32_t direction;
	int32_t select;
}
ui_text_move_event_data_t;

typedef struct ui_text_copy_event_data
{
	ui_t ui;
	ui_element_t* element;
}
ui_text_copy_event_data_t;

typedef struct ui_text_paste_event_data
{
	ui_t ui;
	ui_element_t* element;
}
ui_text_paste_event_data_t;

typedef struct ui_text_escape_event_data
{
	ui_t ui;
	ui_element_t* element;
}
ui_text_escape_event_data_t;

typedef struct ui_text_enter_event_data
{
	ui_t ui;
	ui_element_t* element;
}
ui_text_enter_event_data_t;

typedef struct ui_text_delete_event_data
{
	ui_t ui;
	ui_element_t* element;
	int32_t direction;
}
ui_text_delete_event_data_t;

typedef struct ui_text_undo_event_data
{
	ui_t ui;
	ui_element_t* element;
}
ui_text_undo_event_data_t;

typedef struct ui_text_redo_event_data
{
	ui_t ui;
	ui_element_t* element;
}
ui_text_redo_event_data_t;


typedef void
(*ui_bubble_callback_t)(
	ui_element_t* element,
	void* data
	);

typedef void
(*ui_propagate_size_fn_t)(
	ui_element_t* element,
	ui_element_t* child,
	pair_t delta
	);

typedef void
(*ui_pre_clip_fn_t)(
	ui_element_t* element,
	rect_extent_t* clip,
	ui_element_t* scrollable
	);

typedef void
(*ui_post_clip_fn_t)(
	ui_element_t* element,
	rect_extent_t clip,
	uint8_t opacity,
	ui_element_t* scrollable
	);

typedef bool
(*ui_mouse_over_fn_t)(
	ui_element_t* element
	);

typedef void
(*ui_draw_fn_t)(
	ui_element_t* element,
	rect_extent_t clip,
	uint8_t opacity,
	ui_element_t* scrollable
	);

typedef struct ui_virtual_table
{
	ui_bubble_callback_t bubble_down;
	ui_propagate_size_fn_t propagate_size;
	ui_pre_clip_fn_t pre_clip;
	ui_post_clip_fn_t post_clip;
	ui_mouse_over_fn_t mouse_over;
	ui_draw_fn_t draw_detail;
	ui_draw_fn_t draw_children;
}
ui_virtual_table_t;


struct ui_element
{
	half_extent_t extent;
	half_extent_t end_extent;

	half_extent_t margin;
	half_extent_t draw_margin;

	float border_radius;
	color_argb_t border_color;
	uint32_t opacity;

	ui_align_t align_x;
	ui_align_t align_y;
	ui_position_t position;

	ui_element_t* relative;
	ui_align_t relative_align_x;
	ui_align_t relative_align_y;

	bool hovered;
	bool held;
	bool clickable;
	bool click_passthrough;
	bool selectable;
	bool scrollable;
	bool scoll_passthrough;
	bool interactive_border;

	ui_element_t* parent;
	ui_element_t* prev;
	ui_element_t* next;

	event_target_t mouse_down_target;
	event_target_t mouse_up_target;
	event_target_t mouse_move_target;
	event_target_t mouse_in_target;
	event_target_t mouse_out_target;
	event_target_t mouse_scroll_target;

	event_target_t resize_target;
	event_target_t change_target;
	event_target_t submit_target;
	event_target_t free_target;

	event_target_t text_select_all_target;
	event_target_t text_move_target;
	event_target_t text_copy_target;
	event_target_t text_paste_target;
	event_target_t text_escape_target;
	event_target_t text_enter_target;
	event_target_t text_delete_target;
	event_target_t text_undo_target;
	event_target_t text_redo_target;

	ui_virtual_table_t* virtual_table;
	void* type_data;
};

typedef struct ui_element_info
{
	half_extent_t extent;
	half_extent_t margin;

	float border_radius;
	color_argb_t border_color;
	uint32_t opacity;

	ui_align_t align_x;
	ui_align_t align_y;
	ui_position_t position;

	ui_element_t* relative;
	ui_align_t relative_align_x;
	ui_align_t relative_align_y;

	bool clickable;
	bool click_passthrough;
	bool selectable;
	bool scrollable;
	bool scroll_passthrough;
	bool interactive_border;
}
ui_element_info_t;
