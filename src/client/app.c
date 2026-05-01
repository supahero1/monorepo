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

#include <client/app.h>
#include <shared/file.h>
#include <shared/time.h>
#include <shared/debug.h>
#include <shared/event.h>
#include <shared/macro.h>
#include <shared/extent.h>
#include <shared/options.h>
#include <shared/settings.h>
#include <client/tex/tex_4.h>
#include <shared/alloc/base.h>
#include <client/window/base.h>
#include <client/window/vulkan.h>

#include <libgen.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>


struct app
{
	window_event_table_t* window_event_table;
	event_listener_t* window_close_request_once_listener;
	event_listener_t* window_move_listener;
	event_listener_t* window_resize_listener;
	event_listener_t* window_fullscreen_listener;

	time_timers_t timers;
	settings_t settings;

	setting_t* window_pos_x;
	setting_t* window_pos_y;
	setting_t* window_w;
	setting_t* window_h;
	setting_t* window_fullscreen;

	window_manager_t manager;
	window_t window;
	vulkan_t vulkan;

	event_listener_t* vulkan_draw_listener;
};


void
app_window_close_request_once_fn(
	app_t app,
	window_close_request_event_data_t* event_data
	)
{
	assert_not_null(app);
	assert_not_null(event_data);

	app->window_close_request_once_listener = NULL;
	window_close(event_data->window);
}


void
app_window_free_once_fn(
	app_t app,
	window_free_event_data_t* event_data
	)
{
	assert_not_null(app);
	assert_not_null(event_data);

	window_event_table_t* table = app->window_event_table;

	event_target_del(&table->fullscreen_target, app->window_fullscreen_listener);
	event_target_del(&table->resize_target, app->window_resize_listener);
	event_target_del(&table->move_target, app->window_move_listener);
	event_target_del_once(&table->close_request_target, app->window_close_request_once_listener);
}


void
app_window_on_move_fn(
	app_t app,
	window_move_event_data_t* event_data
	)
{
	assert_not_null(app);
	assert_not_null(event_data);

	settings_modify_f32(app->settings, app->window_pos_x, event_data->new_pos.x);
	settings_modify_f32(app->settings, app->window_pos_y, event_data->new_pos.y);
}


void
app_window_on_resize_fn(
	app_t app,
	window_resize_event_data_t* event_data
	)
{
	assert_not_null(app);
	assert_not_null(event_data);

	settings_modify_f32(app->settings, app->window_w, event_data->new_size.w);
	settings_modify_f32(app->settings, app->window_h, event_data->new_size.h);
}


void
app_window_on_fullscreen_fn(
	app_t app,
	window_focus_event_data_t* event_data
	)
{
	assert_not_null(app);
	assert_not_null(event_data);

	settings_modify_boolean(app->settings, app->window_fullscreen, true);
}


void
app_vulkan_on_free_fn(
	app_t app,
	vulkan_free_event_data_t* event_data
	)
{
	assert_not_null(app);
	assert_not_null(event_data);

	vulkan_event_table_t* table = vulkan_get_event_table(event_data->vulkan);

	event_target_del(&table->draw_target, app->vulkan_draw_listener);
}


void
app_vulkan_on_draw_fn(
	app_t app,
	vulkan_draw_event_data_t* event_data
	)
{
	assert_not_null(app);
	assert_not_null(event_data);

	vulkan_t vk = event_data->vulkan;

	float w = setting_get_f32(app->window_w);
	float h = setting_get_f32(app->window_h);

	vulkan_draw_data_t data =
	{
		.pos =
		{
			.x = 0.0f,
			.y = 0.0f
		},
		.size =
		{
			.w = w * 0.9f,
			.h = h * 0.9f
		},
		.white_color = { 255, 255, 255, 255 },
		.white_depth = 0.5f,
		.black_color = { 255, 255, 255, 255 },
		.black_depth = 0.5f,
		.tex = TEX_RECT,
		.angle = 0.0f,
		.tex_scale = {{ 1.0f, 1.0f }},
		.tex_offset = {{ 0.5f, 0.5f }}
	};
	vulkan_add_draw_data(vk, &data);
}


app_t
app_init(
	int argc,
	char** argv
	)
{
	app_t app = alloc_malloc(app, 1);
	assert_ptr(app, 1);

	assert_ge(argc, 1);
	assert_not_null(argv);
	assert_not_null(argv[0]);

	char exe_path[PATH_MAX];
	realpath(argv[0], exe_path);
	char* dir = dirname(exe_path);

	int status = chdir(dir);
	hard_assert_eq(status, 0);

	global_options = options_init(argc, (void*) argv);

	hard_assert_true(dir_exists("textures"));
	hard_assert_true(dir_exists("shaders"));

	if(!dir_exists("cache"))
	{
		dir_create("cache");
	}

	if(!dir_exists("settings"))
	{
		dir_create("settings");
	}

	app->timers = time_timers_init();
	app->settings = settings_init("settings/window.bin", app->timers);

	app->window_pos_x = settings_add_f32(app->settings, "main_window_pos_x", 0.0f, -16384.0f, 16384.0f, NULL);
	app->window_pos_y = settings_add_f32(app->settings, "main_window_pos_y", 0.0f, -16384.0f, 16384.0f, NULL);
	app->window_w = settings_add_f32(app->settings, "main_window_w", 1280.0f, 480.0f, 16384.0f, NULL);
	app->window_h = settings_add_f32(app->settings, "main_window_h", 720.0f, 270.0f, 16384.0f, NULL);
	app->window_fullscreen = settings_add_boolean(app->settings, "main_window_fullscreen", false, NULL);

	settings_seal(app->settings);

	settings_load(app->settings);

	window_history_t history =
	{
		.extent =
		{
			.x = setting_get_f32(app->window_pos_x),
			.y = setting_get_f32(app->window_pos_y),
			.w = setting_get_f32(app->window_w),
			.h = setting_get_f32(app->window_h)
		},
		.fullscreen = setting_get_boolean(app->window_fullscreen)
	};

	app->manager = window_manager_init();
	app->window = window_init();
	window_manager_add(app->manager, app->window, "Test", &history);
	window_show(app->window);

	app->window_event_table = window_get_event_table(app->window);
	window_event_table_t* table = app->window_event_table;

	event_listener_data_t close_request_once_data =
	{
		.fn = (void*) app_window_close_request_once_fn,
		.data = app
	};
	app->window_close_request_once_listener =
		event_target_once(&table->close_request_target, close_request_once_data);

	event_listener_data_t free_once_data =
	{
		.fn = (void*) app_window_free_once_fn,
		.data = app
	};
	event_target_once(&table->free_target, free_once_data);

	event_listener_data_t move_data =
	{
		.fn = (void*) app_window_on_move_fn,
		.data = app
	};
	app->window_move_listener = event_target_add(&table->move_target, move_data);

	event_listener_data_t resize_data =
	{
		.fn = (void*) app_window_on_resize_fn,
		.data = app
	};
	app->window_resize_listener = event_target_add(&table->resize_target, resize_data);

	event_listener_data_t fullscreen_data =
	{
		.fn = (void*) app_window_on_fullscreen_fn,
		.data = app
	};
	app->window_fullscreen_listener = event_target_add(&table->fullscreen_target, fullscreen_data);

	app->vulkan = vulkan_init(app->window, app->timers);
	vulkan_event_table_t* vulkan_table = vulkan_get_event_table(app->vulkan);

	event_listener_data_t vulkan_free_data =
	{
		.fn = (void*) app_vulkan_on_free_fn,
		.data = app
	};
	event_target_once(&vulkan_table->free_target, vulkan_free_data);

	event_listener_data_t vulkan_draw_data =
	{
		.fn = (void*) app_vulkan_on_draw_fn,
		.data = app
	};
	app->vulkan_draw_listener = event_target_add(&vulkan_table->draw_target, vulkan_draw_data);

	return app;
}


void
app_free(
	app_t app
	)
{
	assert_not_null(app);

	window_manager_free(app->manager);

	settings_free(app->settings);
	time_timers_free(app->timers);

	options_free(global_options);
	global_options = NULL;

	alloc_free(app, 1);
}


void
app_run(
	app_t app
	)
{
	assert_not_null(app);

	window_manager_run(app->manager);
}
