/* skip */

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

#include <shared/debug.h>
#include <client/window.h>
#include <client/ui/base.h>
#include <shared/threads.h>
#include <shared/alloc/base.h>

#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_clipboard.h>


float depth;
float DeltaTime;
uint64_t LastDrawAt;

UIElement* RootElement;

pair_t UIMouse;
UIElement* UIElementUnderMouse;
UIElement* UIClickableUnderMouse;
UIElement* UIScrollableUnderMouse;
UIElement* UISelectedElement;
UIElement* UISelectedSelectableElement;
bool UISameElement;


void
UIRootElementOnWindowResize(
	UIElement* Element,
	window_resize_event_data_t* data
	)
{
	pair_t HalfSize =
	(pair_t)
	{
		.w = data->new_size.w / 2.0f,
		.h = data->new_size.h / 2.0f
	};

	if(
		HalfSize.w != Element->Extent.w ||
		HalfSize.h != Element->Extent.h
		)
	{
		Element->Extent.size = HalfSize;
		UIResizeElement(Element);
	}
}


void
UISetRootElement(
	UIElement* Element
	)
{
	if(RootElement)
	{
		event_target_del(&window_resize_target,
			(void*) UIRootElementOnWindowResize, RootElement);
	}

	RootElement = Element;

	if(RootElement)
	{
		event_target_add(&window_resize_target,
			(void*) UIRootElementOnWindowResize, RootElement);


		pair_t WindowSize = window_get_size();

		Element->Extent.size =
		(pair_t)
		{
			.w = WindowSize.w / 2.0f,
			.h = WindowSize.h / 2.0f
		};

		UIResizeElement(Element);
	}
}


UIElement*
UIGetRootElement(
	void
	)
{
	return RootElement;
}


UIElement*
UIAllocElement(
	UIElementInfo info
	)
{
	UIElement* Element = alloc_malloc(sizeof(UIElement));
	assert_not_null(Element);

	*Element =
	(UIElement)
	{
		.Extent = info.Extent,
		.Margin = info.Margin,

		.BorderRadius = info.BorderRadius,
		.BorderColor = info.BorderColor,
		.Opacity = info.Opacity,

		.AlignX = info.AlignX,
		.AlignY = info.AlignY,
		.pos = info.pos,

		.Relative = info.Relative,
		.RelativeAlignX = info.RelativeAlignX,
		.RelativeAlignY = info.RelativeAlignY,

		.Clickable = info.Clickable,
		.ClickPassthrough = info.ClickPassthrough,
		.Selectable = info.Selectable,
		.Scrollable = info.Scrollable,
		.ScrollPassthrough = info.ScrollPassthrough,
		.InteractiveBorder = info.InteractiveBorder,
		.Inline = info.Inline,
		.block = info.block,
		.ClipToBorder = info.ClipToBorder
	};

	UIUpdateWidth(Element);
	UIUpdateHeight(Element);

	return Element;
}


void
UIFreeElementCallback(
	UIElement* Element,
	void* data
	)
{
	event_target_fire(&Element->FreeTarget, &((UIFreeData){0}));

	event_target_free(&Element->MouseDownTarget);
	event_target_free(&Element->MouseUpTarget);
	event_target_free(&Element->MouseMoveTarget);
	event_target_free(&Element->MouseInTarget);
	event_target_free(&Element->MouseOutTarget);
	event_target_free(&Element->MouseScrollTarget);

	event_target_free(&Element->ResizeTarget);
	event_target_free(&Element->change_target);
	event_target_free(&Element->SubmitTarget);
	event_target_free(&Element->FreeTarget);

	event_target_free(&Element->TextSelectAllTarget);
	event_target_free(&Element->TextMoveTarget);
	event_target_free(&Element->TextCopyTarget);
	event_target_free(&Element->TextPasteTarget);
	event_target_free(&Element->TextEscapeTarget);
	event_target_free(&Element->TextEnterTarget);
	event_target_free(&Element->TextDeleteTarget);
	event_target_free(&Element->TextUndoTarget);
	event_target_free(&Element->TextRedoTarget);

	alloc_free(sizeof(UIElement), Element);
}


void
UIFreeElement(
	UIElement* Element
	)
{
	Element->VirtualTable->BubbleDown(Element, UIFreeElementCallback, NULL);
}


extern void
UIInheritPosition(
	UIElement* Element,
	rect_extent_t Clip
	);


void
UIDrawElement(
	UIElement* Element,
	rect_extent_t Clip,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UIInheritPosition(Element, Clip);

	Element->VirtualTable->PreClip(Element, Scrollable);


	rect_extent_t Bounds =
	(rect_extent_t)
	{
		.min_x = Element->EndExtent.x - Element->Extent.w,
		.min_y = Element->EndExtent.y - Element->Extent.h,
		.max_x = Element->EndExtent.x + Element->Extent.w,
		.max_y = Element->EndExtent.y + Element->Extent.h
	};

	float Diameter = Element->BorderRadius * 2.0f;


	UIClipExplicit(
		Bounds.min_x - Diameter,
		Bounds.max_x + Diameter,
		Bounds.min_y - Diameter,
		Bounds.max_y + Diameter,
		Border
		);


	UIClipExplicit(
		Bounds.min_x,
		Bounds.max_x,
		Bounds.min_y,
		Bounds.max_y,
		content
		);


	rect_extent_t DrawClip;
	bool DrawPass;

	if(Element->ClipToBorder)
	{
		DrawClip = BorderPostClip;
		DrawPass = BorderClipPass;
	}
	else
	{
		DrawClip = ContentPostClip;
		DrawPass = ContentClipPass;
	}


	Element->VirtualTable->PostClip(
		Element,
		DrawClip,
		Opacity,
		Scrollable
		);


	if((Element->InteractiveBorder && BorderClipPass) || ContentClipPass)
	{
		rect_extent_t MouseClip;

		if(Element->InteractiveBorder)
		{
			MouseClip = BorderPostClip;
		}
		else
		{
			MouseClip = ContentPostClip;
		}

		if(
			UIMouse.x >= MouseClip.min_x &&
			UIMouse.y >= MouseClip.min_y &&
			UIMouse.x <= MouseClip.max_x &&
			UIMouse.y <= MouseClip.max_y
			)
		{
			bool OldHovered = Element->Hovered;
			Element->Hovered = Element->VirtualTable->MouseOver(Element);

			if(OldHovered != Element->Hovered)
			{
				if(Element->Hovered)
				{
					event_target_fire(&Element->MouseInTarget, &((UIMouseInData){0}));
				}
				else
				{
					event_target_fire(&Element->MouseOutTarget, &((UIMouseOutData){0}));
				}
			}

			if(Element->Hovered)
			{
				UIElementUnderMouse = Element;

				if(Element->Clickable && !Element->ClickPassthrough)
				{
					UIClickableUnderMouse = Element;
				}

				if(Element->Scrollable && !Element->ScrollPassthrough)
				{
					UIScrollableUnderMouse = Element;
				}
			}
			else
			{
				if(Element->Hovered)
				{
					event_target_fire(&Element->MouseOutTarget, &((UIMouseOutData){0}));
				}
			}
		}
	}

	if(BorderClipPass)
	{
		UIDrawBorder(
			Element,
			BorderPostClip,
			Opacity
			);
	}

	if(DrawPass)
	{
		Element->VirtualTable->DrawDetail(
			Element,
			DrawClip,
			Opacity,
			Scrollable
			);
	}

	Element->VirtualTable->DrawChildren(
		Element,
		DrawClip,
		Opacity,
		Scrollable
		);
}


void
UIDrawCallback(
	void* _,
	void* __
	)
{
	if(LastDrawAt == 0)
	{
		LastDrawAt = WindowGetTime();
	}
	else
	{
		uint64_t Now = WindowGetTime();
		uint64_t delta = Now - LastDrawAt;
		LastDrawAt = Now;
		DeltaTime = (double) delta / 16666666.6;
	}

	UIElement* Root = UIGetRootElement();
	if(!Root)
	{
		return;
	}

	depth = DRAW_DEPTH_LEAP;

	pair_t Mouse = window_get_mouse_pos();

	UIMouse =
	(pair_t)
	{
		.x = MACRO_CLAMP_SYM(Mouse.x, Root->Extent.w),
		.y = MACRO_CLAMP_SYM(Mouse.y, Root->Extent.h)
	};

	UIElementUnderMouse = NULL;
	UIClickableUnderMouse = NULL;
	UIScrollableUnderMouse = NULL;

	UIDrawElement(
		Root,
		(rect_extent_t)
		{
			.min = {{ -Root->Extent.w, -Root->Extent.h }},
			.max = {{  Root->Extent.w,  Root->Extent.h }}
		},
		0xFF,
		NULL
		);

	assert_not_null(UIElementUnderMouse);

	if(UIElementUnderMouse->Selectable)
	{
		window_set_cursor(WINDOW_CURSOR_TYPING);
	}
	else if(UIElementUnderMouse->Clickable)
	{
		window_set_cursor(WINDOW_CURSOR_POINTING);
	}
	else
	{
		window_set_cursor(WINDOW_CURSOR_DEFAULT);
	}
}


void
UIMouseDownCallback(
	void* _,
	window_mouse_down_event_data_t* data
	)
{
	if(data->button != WINDOW_BUTTON_LEFT)
	{
		return;
	}

	UISelectedElement = UIClickableUnderMouse;

	if(UISelectedElement)
	{
		UISelectedElement->Held = true;

		UIMouseDownData event_data =
		(UIMouseDownData)
		{
			.button = data->button,
			.pos = data->pos,
			.clicks = data->clicks
		};

		event_target_fire(&UISelectedElement->MouseDownTarget, &event_data);
	}
}


void
UIMouseUpCallback(
	void* _,
	window_mouse_up_event_data_t* data
	)
{
	if(data->button != WINDOW_BUTTON_LEFT)
	{
		return;
	}

	if(UISelectedElement)
	{
		UISameElement = UIClickableUnderMouse == UISelectedElement;

		UIMouseUpData event_data =
		(UIMouseUpData)
		{
			.button = data->button,
			.pos = data->pos
		};

		event_target_fire(&UISelectedElement->MouseUpTarget, &event_data);

		UISelectedElement = NULL;
	}
}


void // TODO convert to constructor
UIInit(
	void
	)
{
	event_target_add(&window_draw_target, (void*) UIDrawCallback, NULL);
	event_target_add(&window_mouse_down_target, (void*) UIMouseDownCallback, NULL);
	event_target_add(&window_mouse_up_target, (void*) UIMouseUpCallback, NULL);
}


void
UIFree(
	void
	)
{
	event_target_del(&window_draw_target, UIDrawCallback, NULL);

	UIElement* Root = UIGetRootElement();
	if(Root)
	{
		UISetRootElement(NULL);
		UIFreeElement(Root);
	}
}


void
UIUpdateWidth( // TODO apply modifiers
	UIElement* Element
	)
{
	Element->DrawMargin.left = Element->Margin.left + Element->BorderRadius;
	Element->DrawMargin.right = Element->Margin.right + Element->BorderRadius;

	Element->EndExtent.w =
		Element->DrawMargin.left + Element->Extent.w + Element->DrawMargin.right;
}


float
UIEndWidthToWidth(
	UIElement* Element,
	float OldEndWidth
	)
{
	return OldEndWidth - Element->DrawMargin.left - Element->DrawMargin.right;
}


void
UIUpdateHeight(
	UIElement* Element
	)
{
	Element->DrawMargin.top = Element->Margin.top + Element->BorderRadius;
	Element->DrawMargin.bottom = Element->Margin.bottom + Element->BorderRadius;

	Element->EndExtent.h =
		Element->DrawMargin.top + Element->Extent.h + Element->DrawMargin.bottom;
}


float
UIEndHeightToHeight(
	UIElement* Element,
	float OldEndHeight
	)
{
	return OldEndHeight - Element->DrawMargin.top - Element->DrawMargin.bottom;
}


void
UIResizeElement(
	UIElement* Element
	)
{
	pair_t old_size = Element->EndExtent.size;

	UIUpdateWidth(Element);
	UIUpdateHeight(Element);

	if(
		Element->EndExtent.w == old_size.w &&
		Element->EndExtent.h == old_size.h
		)
	{
		return;
	}

	UIResizeData event_data =
	(UIResizeData)
	{
		.old_size =
		(pair_t)
		{
			.w = UIEndWidthToWidth(Element, old_size.w),
			.h = UIEndHeightToHeight(Element, old_size.h)
		},
		.new_size = Element->Extent.size
	};

	event_target_fire(&Element->ResizeTarget, &event_data);

	if(Element->Parent)
	{
		Element->VirtualTable->PropagateSize(
			Element->Parent,
			Element,
			(pair_t)
			{
				.w = Element->EndExtent.w - old_size.w,
				.h = Element->EndExtent.h - old_size.h
			}
			);
	}
}


void
UIDrawBorder(
	UIElement* Element,
	rect_extent_t Clip,
	uint8_t Opacity
	)
{
	if(!Element->BorderRadius)
	{
		return;
	}

	rect_extent_t Border =
	(rect_extent_t)
	{
		.min =
		(pair_t)
		{
			.x = Element->EndExtent.x - Element->Extent.w - Element->BorderRadius,
			.y = Element->EndExtent.y - Element->Extent.h - Element->BorderRadius
		},
		.max =
		(pair_t)
		{
			.x = Element->EndExtent.x + Element->Extent.w + Element->BorderRadius,
			.y = Element->EndExtent.y + Element->Extent.h + Element->BorderRadius
		}
	};

	float Diameter = Element->BorderRadius * 2.0f;

	color_argb_t color = color_argb_mul_a(Element->BorderColor, Opacity);

	UIClipTexture(Element->EndExtent.x, Border.min_y,
		Element->Extent.w, Element->BorderRadius,
		.white_color = color,
		.white_depth = depth,
		.tex = TEXTURE_RECT
		);

	UIClipTextureExplicit(Border.max_x, Border.min_y, Diameter,
		Diameter, 0.5f, 0.5f, 0.75f, 0.25f,
		.white_color = color,
		.white_depth = depth,
		.tex = TEXTURE_CIRCLE
		);

	UIClipTexture(Border.max_x, Element->EndExtent.y,
		Element->BorderRadius, Element->Extent.h,
		.white_color = color,
		.white_depth = depth,
		.tex = TEXTURE_RECT
		);

	UIClipTextureExplicit(Border.max_x, Border.max_y, Diameter,
		Diameter, 0.5f, 0.5f, 0.75f, 0.75f,
		.white_color = color,
		.white_depth = depth,
		.tex = TEXTURE_CIRCLE
		);

	UIClipTexture(Element->EndExtent.x, Border.max_y,
		Element->Extent.w, Element->BorderRadius,
		.white_color = color,
		.white_depth = depth,
		.tex = TEXTURE_RECT
		);

	UIClipTextureExplicit(Border.min_x, Border.max_y, Diameter,
		Diameter, 0.5f, 0.5f, 0.25f, 0.75f,
		.white_color = color,
		.white_depth = depth,
		.tex = TEXTURE_CIRCLE
		);

	UIClipTexture(Border.min_x, Element->EndExtent.y,
		Element->BorderRadius, Element->Extent.h,
		.white_color = color,
		.white_depth = depth,
		.tex = TEXTURE_RECT
		);

	UIClipTextureExplicit(Border.min_x, Border.min_y, Diameter,
		Diameter, 0.5f, 0.5f, 0.25f, 0.25f,
		.white_color = color,
		.white_depth = depth,
		.tex = TEXTURE_CIRCLE
		);
}


bool
UIMouseOverElement(
	UIElement* Element
	)
{
	/* This function assumes that the mouse is over the bounding box of the
	 * element, as is the case during drawing (and this function is only used
	 * during drawing). Due to that, the first if statement here is optimized.
	 */

	{ /* Bound check */
		float Diameter = Element->BorderRadius * 2.0f;
		float min_x = Element->EndExtent.x - Element->Extent.w - Diameter;
		float min_y = Element->EndExtent.y - Element->Extent.h - Diameter;
		float max_x = Element->EndExtent.x + Element->Extent.w + Diameter;
		float max_y = Element->EndExtent.y + Element->Extent.h + Diameter;
		assert_false((UIMouse.x < min_x || UIMouse.y < min_y || UIMouse.x > max_x || UIMouse.y > max_y));
	}

	float min_x = Element->EndExtent.x - Element->Extent.w;
	float min_y = Element->EndExtent.y - Element->Extent.h;
	float max_x = Element->EndExtent.x + Element->Extent.w;
	float max_y = Element->EndExtent.y + Element->Extent.h;

	if(
		Element->BorderRadius == 0.0f ||
		(UIMouse.x >= min_x && UIMouse.x <= max_x) ||
		(UIMouse.y >= min_y && UIMouse.y <= max_y)
		)
	{
		return true;
	}

	/* at this point, only need to check corners. */

	float DiffX = UIMouse.x;
	if(UIMouse.x < min_x)
	{
		DiffX -= min_x;
	}
	else
	{
		DiffX -= max_x;
	}

	float DiffY = UIMouse.y;
	if(UIMouse.y < min_y)
	{
		DiffY -= min_y;
	}
	else
	{
		DiffY -= max_y;
	}

	return DiffX * DiffX + DiffY * DiffY <=
		Element->BorderRadius * Element->BorderRadius * 4.0f;
}
