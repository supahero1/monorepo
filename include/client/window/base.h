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

#include <shared/str.h>
#include <shared/sync.h>
#include <shared/debug.h>
#include <shared/event.h>
#include <shared/macro.h>
#include <shared/extent.h>
#include <shared/threads.h>

#include <stdint.h>


typedef enum window_cursor : uint32_t
{
	WINDOW_CURSOR_DEFAULT,
	WINDOW_CURSOR_TYPING,
	WINDOW_CURSOR_POINTING,
	MACRO_ENUM_BITS(WINDOW_CURSOR)
}
window_cursor_t;


typedef enum window_button : uint32_t
{
	WINDOW_BUTTON_UNKNOWN,
	WINDOW_BUTTON_LEFT,
	WINDOW_BUTTON_MIDDLE,
	WINDOW_BUTTON_RIGHT,
	WINDOW_BUTTON_X1,
	WINDOW_BUTTON_X2,
	MACRO_ENUM_BITS(WINDOW_BUTTON)
}
window_button_t;


#define __(x) WINDOW_KEY_##x
#define _(x) __(x),

typedef enum window_key : uint32_t
{
	_(UNKNOWN)_(RETURN)_(ESCAPE)_(BACKSPACE)_(TAB)_(SPACE)_(EXCLAIM)_(DBLAPOSTROPHE)_(HASH)_(DOLLAR)_(PERCENT)
	_(AMPERSAND)_(APOSTROPHE)_(LEFTPAREN)_(RIGHTPAREN)_(ASTERISK)_(PLUS)_(COMMA)_(MINUS)_(PERIOD)_(SLASH)_(0)_(1)_(2)
	_(3)_(4)_(5)_(6)_(7)_(8)_(9)_(COLON)_(SEMICOLON)_(LESS)_(EQUALS)_(GREATER)_(QUESTION)_(AT)_(LEFTBRACKET)_(BACKSLASH)
	_(RIGHTBRACKET)_(CARET)_(UNDERSCORE)_(GRAVE)_(A)_(B)_(C)_(D)_(E)_(F)_(G)_(H)_(I)_(J)_(K)_(L)_(M)_(N)_(O)_(P)_(Q)_(R)
	_(S)_(T)_(U)_(V)_(W)_(X)_(Y)_(Z)_(LEFTBRACE)_(PIPE)_(RIGHTBRACE)_(TILDE)_(DELETE)_(PLUSMINUS)_(CAPSLOCK)_(F1)_(F2)
	_(F3)_(F4)_(F5)_(F6)_(F7)_(F8)_(F9)_(F10)_(F11)_(F12)_(PRINTSCREEN)_(SCROLLLOCK)_(PAUSE)_(INSERT)_(HOME)_(PAGEUP)
	_(END)_(PAGEDOWN)_(RIGHT)_(LEFT)_(DOWN)_(UP)_(NUMLOCKCLEAR)_(KP_DIVIDE)_(KP_MULTIPLY)_(KP_MINUS)_(KP_PLUS)
	_(KP_ENTER)_(KP_1)_(KP_2)_(KP_3)_(KP_4)_(KP_5)_(KP_6)_(KP_7)_(KP_8)_(KP_9)_(KP_0)_(KP_PERIOD)_(APPLICATION)_(POWER)
	_(KP_EQUALS)_(F13)_(F14)_(F15)_(F16)_(F17)_(F18)_(F19)_(F20)_(F21)_(F22)_(F23)_(F24)_(EXECUTE)_(HELP)_(MENU)
	_(SELECT)_(STOP)_(AGAIN)_(UNDO)_(CUT)_(COPY)_(PASTE)_(FIND)_(MUTE)_(VOLUMEUP)_(VOLUMEDOWN)_(KP_COMMA)
	_(KP_EQUALSAS400)_(ALTERASE)_(SYSREQ)_(CANCEL)_(CLEAR)_(PRIOR)_(RETURN2)_(SEPARATOR)_(OUT)_(OPER)_(CLEARAGAIN)
	_(CRSEL)_(EXSEL)_(KP_00)_(KP_000)_(THOUSANDSSEPARATOR)_(DECIMALSEPARATOR)_(CURRENCYUNIT)_(CURRENCYSUBUNIT)
	_(KP_LEFTPAREN)_(KP_RIGHTPAREN)_(KP_LEFTBRACE)_(KP_RIGHTBRACE)_(KP_TAB)_(KP_BACKSPACE)_(KP_A)_(KP_B)_(KP_C)_(KP_D)
	_(KP_E)_(KP_F)_(KP_XOR)_(KP_POWER)_(KP_PERCENT)_(KP_LESS)_(KP_GREATER)_(KP_AMPERSAND)_(KP_DBLAMPERSAND)
	_(KP_VERTICALBAR)_(KP_DBLVERTICALBAR)_(KP_COLON)_(KP_HASH)_(KP_SPACE)_(KP_AT)_(KP_EXCLAM)_(KP_MEMSTORE)
	_(KP_MEMRECALL)_(KP_MEMCLEAR)_(KP_MEMADD)_(KP_MEMSUBTRACT)_(KP_MEMMULTIPLY)_(KP_MEMDIVIDE)_(KP_PLUSMINUS)_(KP_CLEAR)
	_(KP_CLEARENTRY)_(KP_BINARY)_(KP_OCTAL)_(KP_DECIMAL)_(KP_HEXADECIMAL)_(LCTRL)_(LSHIFT)_(LALT)_(LGUI)_(RCTRL)
	_(RSHIFT)_(RALT)_(RGUI)_(MODE)_(SLEEP)_(WAKE)_(CHANNEL_INCREMENT)_(CHANNEL_DECREMENT)_(MEDIA_PLAY)_(MEDIA_PAUSE)
	_(MEDIA_RECORD)_(MEDIA_FAST_FORWARD)_(MEDIA_REWIND)_(MEDIA_NEXT_TRACK)_(MEDIA_PREVIOUS_TRACK)_(MEDIA_STOP)
	_(MEDIA_EJECT)_(MEDIA_PLAY_PAUSE)_(MEDIA_SELECT)_(AC_NEW)_(AC_OPEN)_(AC_CLOSE)_(AC_EXIT)_(AC_SAVE)_(AC_PRINT)
	_(AC_PROPERTIES)_(AC_SEARCH)_(AC_HOME)_(AC_BACK)_(AC_FORWARD)_(AC_STOP)_(AC_REFRESH)_(AC_BOOKMARKS)_(SOFTLEFT)
	_(SOFTRIGHT)_(CALL)_(ENDCALL)
	MACRO_ENUM_BITS(WINDOW_KEY)
}
window_key_t;

#undef _
#undef __


typedef enum window_mod : uint32_t
{
	WINDOW_MOD_SHIFT,
	WINDOW_MOD_CTRL,
	WINDOW_MOD_ALT,
	WINDOW_MOD_GUI,
	WINDOW_MOD_CAPS_LOCK,
	MACRO_ENUM_BITS_EXP(WINDOW_MOD),

	WINDOW_MOD_SHIFT_BIT		= 1 << WINDOW_MOD_SHIFT,
	WINDOW_MOD_CTRL_BIT			= 1 << WINDOW_MOD_CTRL,
	WINDOW_MOD_ALT_BIT			= 1 << WINDOW_MOD_ALT,
	WINDOW_MOD_GUI_BIT			= 1 << WINDOW_MOD_GUI,
	WINDOW_MOD_CAPS_LOCK_BIT	= 1 << WINDOW_MOD_CAPS_LOCK
}
window_mod_t;


typedef struct window* window_t;

typedef struct window_history window_history_t;


typedef enum window_user_event : uint32_t
{
	WINDOW_USER_EVENT_WINDOW_INIT,
	WINDOW_USER_EVENT_WINDOW_CLOSE,
	WINDOW_USER_EVENT_WINDOW_FULLSCREEN,
	WINDOW_USER_EVENT_SET_CURSOR,
	WINDOW_USER_EVENT_SHOW_WINDOW,
	WINDOW_USER_EVENT_HIDE_WINDOW,
	WINDOW_USER_EVENT_START_TYPING,
	WINDOW_USER_EVENT_STOP_TYPING,
	WINDOW_USER_EVENT_SET_CLIPBOARD,
	WINDOW_USER_EVENT_GET_CLIPBOARD,
	WINDOW_USER_EVENT_RUN,
	MACRO_ENUM_BITS(WINDOW_USER_EVENT)
}
window_user_event_t;

typedef struct window_user_event_window_init_data
{
	str_t title;
	const window_history_t* history;
}
window_user_event_window_init_data_t;

typedef struct window_user_event_window_free_data
{
}
window_user_event_window_free_data_t;

typedef struct window_user_event_window_close_data
{
}
window_user_event_window_close_data_t;

typedef struct window_user_event_window_fullscreen_data
{
}
window_user_event_window_fullscreen_data_t;

typedef struct window_user_event_set_cursor_data
{
	window_cursor_t cursor;
}
window_user_event_set_cursor_data_t;

typedef struct window_user_event_show_window_data
{
}
window_user_event_show_window_data_t;

typedef struct window_user_event_hide_window_data
{
}
window_user_event_hide_window_data_t;

typedef struct window_user_event_start_typing_data
{
}
window_user_event_start_typing_data_t;

typedef struct window_user_event_stop_typing_data
{
}
window_user_event_stop_typing_data_t;

typedef struct window_user_event_set_clipboard_data
{
	str_t str;
}
window_user_event_set_clipboard_data_t;

typedef struct window_user_event_get_clipboard_data
{
}
window_user_event_get_clipboard_data_t;

typedef struct window_user_event_run_data
{
	thread_data_t run;
	sync_sem_t* sem;
	bool wait;
}
window_user_event_run_data_t;


typedef struct window_manager* window_manager_t;


extern window_manager_t
window_manager_init(
	void
	);


extern void
window_manager_free(
	window_manager_t manager
	);


extern void
window_manager_add(
	window_manager_t manager,
	window_t window,
	const char* title,
	const window_history_t* history
	);


extern void
window_manager_push_event(
	window_manager_t manager,
	window_user_event_t type,
	void* context,
	void* data
	);


extern bool
window_manager_is_running(
	window_manager_t manager
	);


extern void
window_manager_stop_running(
	window_manager_t manager
	);


extern void
window_manager_ref(
	window_manager_t manager
	);


extern void
window_manager_unref(
	window_manager_t manager
	);


extern void
window_manager_run(
	window_manager_t manager
	);


typedef struct window_init_event_data
{
	window_t window;
}
window_init_event_data_t;

typedef struct window_free_event_data
{
	window_t window;
}
window_free_event_data_t;

typedef struct window_move_event_data
{
	window_t window;
	pair_t old_pos;
	pair_t new_pos;
	pair_t rel_pos;
}
window_move_event_data_t;

typedef struct window_resize_event_data
{
	window_t window;
	pair_t old_size;
	pair_t new_size;
	pair_t rel_size;
}
window_resize_event_data_t;

typedef struct window_focus_event_data
{
	window_t window;
}
window_focus_event_data_t;

typedef struct window_blur_event_data
{
	window_t window;
}
window_blur_event_data_t;

typedef struct window_close_request_event_data
{
	window_t window;
}
window_close_request_event_data_t;

typedef struct window_closing_event_data
{
	window_t window;
}
window_closing_event_data_t;

typedef struct window_fullscreen_event_data
{
	window_t window;
	bool fullscreen;
}
window_fullscreen_event_data_t;

typedef struct window_key_down_event_data
{
	window_t window;
	window_key_t key;
	window_mod_t mods;
	uint8_t repeat;
}
window_key_down_event_data_t;

typedef struct window_key_up_event_data
{
	window_t window;
	window_key_t key;
	window_mod_t mods;
}
window_key_up_event_data_t;

typedef struct window_text_event_data
{
	window_t window;
	str_t str;
}
window_text_event_data_t;

typedef struct window_get_clipboard_event_data
{
	window_t window;
	str_t str;
}
window_get_clipboard_event_data_t;

typedef struct window_set_clipboard_event_data
{
	window_t window;
	bool success;
}
window_set_clipboard_event_data_t;

typedef struct window_mouse_down_event_data
{
	window_t window;
	window_button_t button;
	pair_t pos;
	uint8_t clicks;
}
window_mouse_down_event_data_t;

typedef struct window_mouse_up_event_data
{
	window_t window;
	window_button_t button;
	uint8_t clicks;
	pair_t pos;
}
window_mouse_up_event_data_t;

typedef struct window_mouse_move_event_data
{
	window_t window;
	pair_t old_pos;
	pair_t new_pos;
	pair_t rel_pos;
}
window_mouse_move_event_data_t;

typedef struct window_mouse_scroll_event_data
{
	window_t window;
	float offset_y;
}
window_mouse_scroll_event_data_t;


typedef struct window_info
{
	half_extent_t extent;
	pair_t mouse;

	bool fullscreen;
	bool rel_mouse_in_fullscreen;
}
window_info_t;

typedef struct window_event_table
{
	event_target_t init_target;
	event_target_t free_target;
	event_target_t move_target;
	event_target_t resize_target;
	event_target_t focus_target;
	event_target_t blur_target;
	event_target_t close_request_target;
	event_target_t closing_target;
	event_target_t fullscreen_target;
	event_target_t key_down_target;
	event_target_t key_up_target;
	event_target_t text_target;
	event_target_t get_clipboard_target;
	event_target_t set_clipboard_target;
	event_target_t mouse_down_target;
	event_target_t mouse_up_target;
	event_target_t mouse_move_target;
	event_target_t mouse_scroll_target;
}
window_event_table_t;

struct window_history
{
	half_extent_t extent;
	bool fullscreen;
	bool rel_mouse_in_fullscreen;
};


extern window_t
window_init(
	void
	);


extern void
window_close(
	window_t window
	);


extern void
window_set(
	window_t window,
	const char* name,
	void* data
	);


extern void*
window_get(
	window_t window,
	const char* name
	);


extern void
window_push_event(
	window_t window,
	window_user_event_t type,
	void* data
	);


extern void
window_set_cursor(
	window_t window,
	window_cursor_t cursor
	);


extern void
window_show(
	window_t window
	);


extern void
window_hide(
	window_t window
	);


extern void
window_start_typing(
	window_t window
	);


extern void
window_stop_typing(
	window_t window
	);


extern void
window_get_clipboard(
	window_t window
	);


extern void
window_set_clipboard(
	window_t window,
	str_t str
	);


extern void
window_toggle_fullscreen(
	window_t window
	);


extern void
window_get_info(
	window_t window,
	window_info_t* info
	);


extern window_event_table_t*
window_get_event_table(
	window_t window
	);


extern const char* const*
window_get_vulkan_extensions(
	uint32_t* count
	);


extern void*
window_get_vulkan_proc_addr_fn(
	void
	);


extern void
window_init_vulkan_surface(
	window_t window,
	void* instance,
	void* surface
	);


extern void
window_free_vulkan_surface(
	void* instance,
	void* surface
	);


extern void
window_run_on_main_thread(
	window_t window,
	thread_data_t data,
	bool wait
	);


extern void
window_ref(
	window_t window
	);


extern void
window_unref(
	window_t window
	);


extern bool
window_is_closed(
	window_t window
	);
