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
#include <client/window/base.h>


typedef enum ui_align : uint8_t
{
	UI_ALIGN_TOP,
	UI_ALIGN_LEFT = UI_ALIGN_TOP,

	UI_ALIGN_MIDDLE,
	UI_ALIGN_CENTER = UI_ALIGN_MIDDLE,

	UI_ALIGN_BOTTOM,
	UI_ALIGN_RIGHT = UI_ALIGN_BOTTOM
}
UIAlign;

typedef enum ui_axis : uint8_t
{
	UI_AXIS_VERTICAL,
	UI_AXIS_HORIZONTAL
}
UIAxis;

typedef enum ui_position : uint8_t
{
	UI_POSITION_INHERIT,
	UI_POSITION_ABSOLUTE,
	UI_POSITION_RELATIVE,
}
UIPosition;

/*
typedef enum UISliderType
{
	UI_SLIDER_TYPE_NORMAL,
	UI_SLIDER_TYPE_BRIGHTNESS,
	UI_SLIDER_TYPE_OPACITY,
	UI_SLIDER_TYPE_RED,
	UI_SLIDER_TYPE_GREEN,
	UI_SLIDER_TYPE_BLUE
}
UISliderType;

typedef struct UITextOffset
{
	uint32_t LineIdx;
	uint32_t Offset;
	uint32_t Idx;
	float Stride;
}
UITextOffset;

typedef struct UITextGlyph
{
	float top;
	float left;
	float Stride;
	float size;
	TexInfo tex;
}
UITextGlyph;

typedef struct UITextLine
{
	float x;
	float y;
	float width;

	uint32_t Idx;

	uint32_t Separator;
	uint32_t len;
	UITextGlyph Glyphs[];
}
UITextLine;

typedef struct UITextMod
{
	uint32_t Start;

	const uint32_t* OldCodepoints;
	uint32_t OldLength;

	const uint32_t* NewCodepoints;
	uint32_t NewLength;
}
UITextMod;

typedef uint32_t
(*UITextFilter)(
	uint32_t Codepoint
	);

typedef struct UIElement UIElement;

typedef struct UIContainer
{
	UIAxis Axis;

	float ContentW;
	float ContentH;

	float MaxInlineW;
	float MaxInlineH;

	float GoalOffsetX;
	float GoalOffsetY;

	float OffsetX;
	float offset_y;

	float OffsetMin;
	float OffsetMax;

	color_argb_t white_color;
	color_argb_t black_color;
	TexInfo tex;

	UIElement* head;
	UIElement* tail;
}
UIContainer;

typedef struct UIText
{
	uint32_t* Codepoints;
	uint32_t len;
	uint32_t max_len;
	float MaxWidth;

	float FontSize;
	UIAlign AlignX;

	bool Selectable;
	bool Editable;
	bool AllowNewline;

	color_argb_t Stroke;
	color_argb_t InverseStroke;
	color_argb_t Fill;
	color_argb_t InverseFill;
	color_argb_t Background;

	uint32_t LineCount;
	UITextLine** Lines;

	uint32_t CurrentMod;
	uint32_t ModCount;
	UITextMod* mods;

	UITextFilter Filter;
}
UIText;

typedef struct UICheckbox
{
	uint32_t Checked;

	color_argb_t CheckYes;
	color_argb_t CheckNo;
	color_argb_t Background;
}
UICheckbox;

typedef struct UISlider
{
	UISliderType type;
	UIAxis Axis;

	color_argb_t color;
	color_argb_t BgColor;

	float Sections;
	float value;
}
UISlider;

typedef struct UIScrollbar
{
	UIAxis Axis;

	color_argb_t color;
	color_argb_t AltColor;

	float ViewMin;
	float ViewMax;

	float Offset;
	float Mouse;

	bool Hovered;
	bool Held;
}
UIScrollbar;

typedef struct UIColorPicker
{
	color_xy_t pos;
	color_argb_t ColorRGB;
	color_hsv_t ColorHSV;
}
UIColorPicker;

typedef struct UITexture
{
	float SW;
	float SH;
	float OX;
	float OY;

	color_argb_t white_color;
	color_argb_t black_color;
	TexInfo tex;
	float angle;
}
UITexture;
*/

typedef struct UIMouseDownData
{
	window_button_t button;
	pair_t pos;
	uint8_t clicks;
}
UIMouseDownData;

typedef struct UIMouseUpData
{
	window_button_t button;
	pair_t pos;
}
UIMouseUpData;

typedef struct UIMouseMoveData
{
	pair_t old_pos;
	pair_t new_pos;
}
UIMouseMoveData;

typedef struct UIMouseInData
{
	pair_t pos;
}
UIMouseInData;

typedef struct UIMouseOutData
{
	pair_t pos;
}
UIMouseOutData;

typedef struct UIMouseScrollData
{
	float offset_y;
}
UIMouseScrollData;

typedef struct UIResizeData
{
	pair_t old_size;
	pair_t new_size;
}
UIResizeData;

typedef struct UIChangeData
{
	uint8_t _;
}
UIChangeData;

typedef struct UISubmitData
{
	uint8_t _;
}
UISubmitData;

typedef struct UIFreeData
{
	uint8_t _;
}
UIFreeData;

typedef struct UITextSelectAllData
{
	uint8_t _;
}
UITextSelectAllData;

typedef struct UITextMoveData
{
	UIAxis Axis;
	uint32_t Direction;

	bool Select;
	bool WordBound;
}
UITextMoveData;

typedef struct UITextCopyData
{
	uint8_t _;
}
UITextCopyData;

typedef struct UITextPasteData
{
	String Text;
}
UITextPasteData;

typedef struct UITextEscapeData
{
	uint8_t _;
}
UITextEscapeData;

typedef struct UITextEnterData
{
	uint8_t _;
}
UITextEnterData;

typedef struct UITextDeleteData
{
	uint32_t Direction;

	bool WordBound;
}
UITextDeleteData;

typedef struct UITextUndoData
{
	uint8_t _;
}
UITextUndoData;

typedef struct UITextRedoData
{
	uint8_t _;
}
UITextRedoData;

typedef struct UIElement UIElement;

typedef void
(*UIBubbleCallback)(
	UIElement* Element,
	void* data
	);

typedef void
(*UIBubbleFn)(
	UIElement* Element,
	UIBubbleCallback Callback,
	void* data
	);

typedef void
(*UIPropagateSizeFn)(
	UIElement* Element,
	UIElement* Child,
	pair_t delta
	);

typedef void
(*UIPreClipFn)(
	UIElement* Element,
	UIElement* Scrollable
	);

typedef void
(*UIPostClipFn)(
	UIElement* Element,
	rect_extent_t Clip,
	uint8_t Opacity,
	UIElement* Scrollable
	);

typedef bool
(*UIMouseOver)(
	UIElement* Element
	);

typedef void
(*UIDrawFn)(
	UIElement* Element,
	rect_extent_t Clip,
	uint8_t Opacity,
	UIElement* Scrollable
	);

typedef struct UIVirtualTable
{
	UIBubbleFn BubbleDown;
	UIPropagateSizeFn PropagateSize;
	UIPreClipFn PreClip;
	UIPostClipFn PostClip;
	UIMouseOver MouseOver;
	UIDrawFn DrawDetail;
	UIDrawFn DrawChildren;
}
UIVirtualTable;

struct UIElement
{
	half_extent_t Extent;
	half_extent_t EndExtent;
	half_extent_t Margin;
	half_extent_t DrawMargin;

	float BorderRadius;
	color_argb_t BorderColor;
	uint32_t Opacity;

	UIAlign AlignX;
	UIAlign AlignY;
	UIPosition pos;

	UIElement* Relative;
	UIAlign RelativeAlignX;
	UIAlign RelativeAlignY;

	bool Hovered;
	bool Held;
	bool Clickable;
	bool ClickPassthrough;
	bool Selectable;
	bool Scrollable;
	bool ScrollPassthrough;
	bool InteractiveBorder;
	bool Inline;
	bool block;
	bool ClipToBorder;

	UIElement* Parent;
	UIElement* prev;
	UIElement* next;

	event_target_t MouseDownTarget;
	event_target_t MouseUpTarget;
	event_target_t MouseMoveTarget;
	event_target_t MouseInTarget;
	event_target_t MouseOutTarget;
	event_target_t MouseScrollTarget;

	event_target_t ResizeTarget;
	event_target_t change_target;
	event_target_t SubmitTarget;
	event_target_t FreeTarget;

	event_target_t TextSelectAllTarget;
	event_target_t TextMoveTarget;
	event_target_t TextCopyTarget;
	event_target_t TextPasteTarget;
	event_target_t TextEscapeTarget;
	event_target_t TextEnterTarget;
	event_target_t TextDeleteTarget;
	event_target_t TextUndoTarget;
	event_target_t TextRedoTarget;

	UIVirtualTable* VirtualTable;
	void* TypeData;
};


typedef struct UIElementInfo
{
	half_extent_t Extent;
	half_extent_t Margin;

	float BorderRadius;
	color_argb_t BorderColor;
	uint32_t Opacity;

	UIAlign AlignX;
	UIAlign AlignY;
	UIPosition pos;

	UIElement* Relative;
	UIAlign RelativeAlignX;
	UIAlign RelativeAlignY;

	bool Clickable;
	bool ClickPassthrough;
	bool Selectable;
	bool Scrollable;
	bool ScrollPassthrough;
	bool InteractiveBorder;
	bool Inline;
	bool block;
	bool ClipToBorder;
}
UIElementInfo;


extern pair_t UIMouse;
extern UIElement* UIElementUnderMouse;
extern UIElement* UIClickableUnderMouse;
extern UIElement* UIScrollableUnderMouse;
extern UIElement* UISelectedElement;
extern UIElement* UISelectedSelectableElement;
extern bool UISameElement;


extern void
UISetRootElement(
	UIElement* Element
	);


extern UIElement*
UIGetRootElement(
	void
	);


extern UIElement*
UIAllocElement(
	UIElementInfo info
	);


extern void
UIFreeElement(
	UIElement* Element
	);


extern void
UIDrawElement(
	UIElement* Element,
	rect_extent_t Clip,
	uint8_t Opacity,
	UIElement* Scrollable
	);


extern void
UIInit(
	void
	);


extern void
UIFree(
	void
	);


extern void
UIUpdateWidth(
	UIElement* Element
	);


extern void
UIUpdateHeight(
	UIElement* Element
	);


extern void
UIResizeElement(
	UIElement* Element
	);


#define UIClipExplicit(_MinX, _MaxX, _MinY, _MaxY, Prefix)	\
rect_extent_t Prefix##PostClip =								\
(rect_extent_t)												\
{															\
	.min_x = MACRO_MAX((_MinX), Clip.min_x),						\
	.min_y = MACRO_MAX((_MinY), Clip.min_y),						\
	.max_x = MACRO_MIN((_MaxX), Clip.max_x),						\
	.max_y = MACRO_MIN((_MaxY), Clip.max_y)							\
};															\
															\
bool Prefix##ClipPass =										\
	Prefix##PostClip.min_x < Prefix##PostClip.max_x &&		\
	Prefix##PostClip.min_y < Prefix##PostClip.max_y

#define UIClip(x, y, w, h)	\
UIClipExplicit(				\
	(x) - (w),				\
	(x) + (w),				\
	(y) - (h),				\
	(y) + (h),				\
	)

#define UIAfterClip(Prefix)												\
half_extent_t Prefix##DrawClip =											\
(half_extent_t)															\
{																		\
	.x = ( Prefix##PostClip .min_x + Prefix##PostClip .max_x ) * 0.5f,	\
	.y = ( Prefix##PostClip .min_y + Prefix##PostClip .max_y ) * 0.5f,	\
	.w = Prefix##PostClip .max_x - Prefix##PostClip .min_x,				\
	.h = Prefix##PostClip .max_y - Prefix##PostClip .min_y				\
}

#define UIClipTextureExplicit(_X, _Y, _W, _H, SW, SH, OX, OY, ...)	\
({																	\
	UIClip((_X), (_Y), (_W) * (SW), (_H) * (SH));					\
																	\
	if(ClipPass)													\
	{																\
		UIAfterClip();												\
																	\
		VkVertexInstanceInput In =									\
		(VkVertexInstanceInput)										\
		{															\
			.pos = { DrawClip.x, DrawClip.y },					\
			.size = { DrawClip.w, DrawClip.h },				\
			.tex_scale = { DrawClip.w / (_W), DrawClip.h / (_H) },	\
			.tex_offset = {											\
				(OX) + (DrawClip.x - (_X)) * 0.5f / (_W),			\
				(OY) + (DrawClip.y - (_Y)) * 0.5f / (_H)			\
			} __VA_OPT__(,)											\
			__VA_ARGS__												\
		};															\
																	\
		if(In.white_color.a || In.black_color.a)						\
		{															\
			window_add_draw_event_data(&In);									\
		}															\
	}																\
																	\
	ClipPass;														\
})

#define UIClipTexture(x, y, w, h, ...)	\
UIClipTextureExplicit((x), (y), (w), (h), 1.0f, 1.0f, 0.5f, 0.5f __VA_OPT__(,) __VA_ARGS__)


#define DRAW_DEPTH_LEAP (1.0f / (1U << 20U))
#define DRAW_DEPTH_JIFFIE (1.0f / (1U << 22U))
#define DRAW_DEPTH_2_JIFFIE (DRAW_DEPTH_JIFFIE * 2.0f)
#define DRAW_DEPTH_3_JIFFIE (DRAW_DEPTH_JIFFIE * 3.0f)


extern float depth;
extern float DeltaTime;
extern uint64_t LastDrawAt;


extern void
UIDrawBorder(
	UIElement* Element,
	rect_extent_t Clip,
	uint8_t Opacity
	);



extern bool
UIMouseOverElement(
	UIElement* Element
	);












/*
extern void
UIUpdateElement(
	UIElement* Element
	);


extern void
UIActivate(
	void
	);


extern void
UIDeactivate(
	void
	);


extern void
UIAddElementFirst(
	UIElement* Element,
	UIElement* Parent
	);


extern void
UIAddElementLast(
	UIElement* Element,
	UIElement* Parent
	);


extern void
UIAddElementBefore(
	UIElement* Element,
	UIElement* Before
	);


extern void
UIAddElementAfter(
	UIElement* Element,
	UIElement* After
	);


extern int
UIIsLinked(
	UIElement* Element
	);


extern void
UIUnlinkElement(
	UIElement* Element
	);


extern void
UIRemoveElement(
	UIElement* Element
	);





typedef enum UIKey
{
	UI_KEY_UNKNOWN,
	UI_KEY_RETURN,
	UI_KEY_ESCAPE,
	UI_KEY_BACKSPACE,
	UI_KEY_TAB,
	UI_KEY_SPACE,
	UI_KEY_EXCLAIM,
	UI_KEY_DBLAPOSTROPHE,
	UI_KEY_HASH,
	UI_KEY_DOLLAR,
	UI_KEY_PERCENT,
	UI_KEY_AMPERSAND,
	UI_KEY_APOSTROPHE,
	UI_KEY_LEFTPAREN,
	UI_KEY_RIGHTPAREN,
	UI_KEY_ASTERISK,
	UI_KEY_PLUS,
	UI_KEY_COMMA,
	UI_KEY_MINUS,
	UI_KEY_PERIOD,
	UI_KEY_SLASH,
	UI_KEY_0,
	UI_KEY_1,
	UI_KEY_2,
	UI_KEY_3,
	UI_KEY_4,
	UI_KEY_5,
	UI_KEY_6,
	UI_KEY_7,
	UI_KEY_8,
	UI_KEY_9,
	UI_KEY_COLON,
	UI_KEY_SEMICOLON,
	UI_KEY_LESS,
	UI_KEY_EQUALS,
	UI_KEY_GREATER,
	UI_KEY_QUESTION,
	UI_KEY_AT,
	UI_KEY_LEFTBRACKET,
	UI_KEY_BACKSLASH,
	UI_KEY_RIGHTBRACKET,
	UI_KEY_CARET,
	UI_KEY_UNDERSCORE,
	UI_KEY_GRAVE,
	UI_KEY_A,
	UI_KEY_B,
	UI_KEY_C,
	UI_KEY_D,
	UI_KEY_E,
	UI_KEY_F,
	UI_KEY_G,
	UI_KEY_H,
	UI_KEY_I,
	UI_KEY_J,
	UI_KEY_K,
	UI_KEY_L,
	UI_KEY_M,
	UI_KEY_N,
	UI_KEY_O,
	UI_KEY_P,
	UI_KEY_Q,
	UI_KEY_R,
	UI_KEY_S,
	UI_KEY_T,
	UI_KEY_U,
	UI_KEY_V,
	UI_KEY_W,
	UI_KEY_X,
	UI_KEY_Y,
	UI_KEY_Z,
	UI_KEY_LEFTBRACE,
	UI_KEY_PIPE,
	UI_KEY_RIGHTBRACE,
	UI_KEY_TILDE,
	UI_KEY_DELETE,
	UI_KEY_PLUSMINUS,
	UI_KEY_CAPSLOCK,
	UI_KEY_F1,
	UI_KEY_F2,
	UI_KEY_F3,
	UI_KEY_F4,
	UI_KEY_F5,
	UI_KEY_F6,
	UI_KEY_F7,
	UI_KEY_F8,
	UI_KEY_F9,
	UI_KEY_F10,
	UI_KEY_F11,
	UI_KEY_F12,
	UI_KEY_PRINTSCREEN,
	UI_KEY_SCROLLLOCK,
	UI_KEY_PAUSE,
	UI_KEY_INSERT,
	UI_KEY_HOME,
	UI_KEY_PAGEUP,
	UI_KEY_END,
	UI_KEY_PAGEDOWN,
	UI_KEY_RIGHT,
	UI_KEY_LEFT,
	UI_KEY_DOWN,
	UI_KEY_UP,
	UI_KEY_NUMLOCKCLEAR,
	UI_KEY_KP_DIVIDE,
	UI_KEY_KP_MULTIPLY,
	UI_KEY_KP_MINUS,
	UI_KEY_KP_PLUS,
	UI_KEY_KP_ENTER,
	UI_KEY_KP_1,
	UI_KEY_KP_2,
	UI_KEY_KP_3,
	UI_KEY_KP_4,
	UI_KEY_KP_5,
	UI_KEY_KP_6,
	UI_KEY_KP_7,
	UI_KEY_KP_8,
	UI_KEY_KP_9,
	UI_KEY_KP_0,
	UI_KEY_KP_PERIOD,
	UI_KEY_APPLICATION,
	UI_KEY_POWER,
	UI_KEY_KP_EQUALS,
	UI_KEY_F13,
	UI_KEY_F14,
	UI_KEY_F15,
	UI_KEY_F16,
	UI_KEY_F17,
	UI_KEY_F18,
	UI_KEY_F19,
	UI_KEY_F20,
	UI_KEY_F21,
	UI_KEY_F22,
	UI_KEY_F23,
	UI_KEY_F24,
	UI_KEY_EXECUTE,
	UI_KEY_HELP,
	UI_KEY_MENU,
	UI_KEY_SELECT,
	UI_KEY_STOP,
	UI_KEY_AGAIN,
	UI_KEY_UNDO,
	UI_KEY_CUT,
	UI_KEY_COPY,
	UI_KEY_PASTE,
	UI_KEY_FIND,
	UI_KEY_MUTE,
	UI_KEY_VOLUMEUP,
	UI_KEY_VOLUMEDOWN,
	UI_KEY_KP_COMMA,
	UI_KEY_KP_EQUALSAS400,
	UI_KEY_ALTERASE,
	UI_KEY_SYSREQ,
	UI_KEY_CANCEL,
	UI_KEY_CLEAR,
	UI_KEY_PRIOR,
	UI_KEY_RETURN2,
	UI_KEY_SEPARATOR,
	UI_KEY_OUT,
	UI_KEY_OPER,
	UI_KEY_CLEARAGAIN,
	UI_KEY_CRSEL,
	UI_KEY_EXSEL,
	UI_KEY_KP_00,
	UI_KEY_KP_000,
	UI_KEY_THOUSANDSSEPARATOR,
	UI_KEY_DECIMALSEPARATOR,
	UI_KEY_CURRENCYUNIT,
	UI_KEY_CURRENCYSUBUNIT,
	UI_KEY_KP_LEFTPAREN,
	UI_KEY_KP_RIGHTPAREN,
	UI_KEY_KP_LEFTBRACE,
	UI_KEY_KP_RIGHTBRACE,
	UI_KEY_KP_TAB,
	UI_KEY_KP_BACKSPACE,
	UI_KEY_KP_A,
	UI_KEY_KP_B,
	UI_KEY_KP_C,
	UI_KEY_KP_D,
	UI_KEY_KP_E,
	UI_KEY_KP_F,
	UI_KEY_KP_XOR,
	UI_KEY_KP_POWER,
	UI_KEY_KP_PERCENT,
	UI_KEY_KP_LESS,
	UI_KEY_KP_GREATER,
	UI_KEY_KP_AMPERSAND,
	UI_KEY_KP_DBLAMPERSAND,
	UI_KEY_KP_VERTICALBAR,
	UI_KEY_KP_DBLVERTICALBAR,
	UI_KEY_KP_COLON,
	UI_KEY_KP_HASH,
	UI_KEY_KP_SPACE,
	UI_KEY_KP_AT,
	UI_KEY_KP_EXCLAM,
	UI_KEY_KP_MEMSTORE,
	UI_KEY_KP_MEMRECALL,
	UI_KEY_KP_MEMCLEAR,
	UI_KEY_KP_MEMADD,
	UI_KEY_KP_MEMSUBTRACT,
	UI_KEY_KP_MEMMULTIPLY,
	UI_KEY_KP_MEMDIVIDE,
	UI_KEY_KP_PLUSMINUS,
	UI_KEY_KP_CLEAR,
	UI_KEY_KP_CLEARENTRY,
	UI_KEY_KP_BINARY,
	UI_KEY_KP_OCTAL,
	UI_KEY_KP_DECIMAL,
	UI_KEY_KP_HEXADECIMAL,
	UI_KEY_LCTRL,
	UI_KEY_LSHIFT,
	UI_KEY_LALT,
	UI_KEY_LGUI,
	UI_KEY_RCTRL,
	UI_KEY_RSHIFT,
	UI_KEY_RALT,
	UI_KEY_RGUI,
	UI_KEY_MODE,
	UI_KEY_SLEEP,
	UI_KEY_WAKE,
	UI_KEY_CHANNEL_INCREMENT,
	UI_KEY_CHANNEL_DECREMENT,
	UI_KEY_MEDIA_PLAY,
	UI_KEY_MEDIA_PAUSE,
	UI_KEY_MEDIA_RECORD,
	UI_KEY_MEDIA_FAST_FORWARD,
	UI_KEY_MEDIA_REWIND,
	UI_KEY_MEDIA_NEXT_TRACK,
	UI_KEY_MEDIA_PREVIOUS_TRACK,
	UI_KEY_MEDIA_STOP,
	UI_KEY_MEDIA_EJECT,
	UI_KEY_MEDIA_PLAY_PAUSE,
	UI_KEY_MEDIA_SELECT,
	UI_KEY_AC_NEW,
	UI_KEY_AC_OPEN,
	UI_KEY_AC_CLOSE,
	UI_KEY_AC_EXIT,
	UI_KEY_AC_SAVE,
	UI_KEY_AC_PRINT,
	UI_KEY_AC_PROPERTIES,
	UI_KEY_AC_SEARCH,
	UI_KEY_AC_HOME,
	UI_KEY_AC_BACK,
	UI_KEY_AC_FORWARD,
	UI_KEY_AC_STOP,
	UI_KEY_AC_REFRESH,
	UI_KEY_AC_BOOKMARKS,
	UI_KEY_SOFTLEFT,
	UI_KEY_SOFTRIGHT,
	UI_KEY_CALL,
	UI_KEY_ENDCALL,
	UI_KEY__COUNT
}
UIKey;


extern void
UIOnKeyDown(
	UIKey key
	);


extern void
UIOnKeyUp(
	UIKey key
	);


extern void
UIOnMouseDown(
	UIMouseDownData* data
	);


extern void
UIOnMouseUp(
	UIMouseUpData* data
	);


extern void
UIOnMouseMove(
	UIMouseMoveData* data
	);


extern void
UIOnMouseScroll(
	UIMouseScrollData* data
	);
*/
