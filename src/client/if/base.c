/* skip */

#include <shared/debug.h>
#include <client/if/base.h>

#include <string.h>
#include <byteswap.h>
#include <inttypes.h>
#include <utf8proc.h>


UIElement* ColorPickerContainer;
UIElement* ColorPickerElement;
UIElement* ColorPickerBackground;
IHandle ColorPickerHexText;

typedef struct IColorPickerLine
{
	UISlider* Slider;
	IHandle value;
}
IColorPickerLine;

IColorPickerLine ColorPickerBrightness;
IColorPickerLine ColorPickerOpacity;
IColorPickerLine ColorPickerRed;
IColorPickerLine ColorPickerGreen;
IColorPickerLine ColorPickerBlue;

UIElement* CurrentColorContainer;

UIElement* CurrentDropdown;

const char NoNewlineCharFilter[256] =
{
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 , ' ',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	' ', '!', '"', '#', '$', '%', '&','\'', '(', ')', '*', '+', ',', '-', '.', '/',
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?',
	'@', 'A', 'b', 'C', 'D', 'E', 'F', 'g', 'h', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
	'P', 'Q', 'r', 's', 'T', 'U', 'v', 'w', 'x', 'y', 'Z', '[','\\', ']', '^', '_',
	'`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~',  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
};

const char IntegerCharFilter[256] =
{
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 , '+',  0 , '-',  0 ,  0 ,
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
};

const char HexColorCharFilter[256] =
{
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 , '#',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 , 'A', 'b', 'C', 'D', 'E', 'F',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 , 'a', 'b', 'c', 'd', 'e', 'f',  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
	 0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,  0 ,
};

const char* TextCharFilters[] =
{
	[I_TEXT_TYPE_MULTILINE_TEXT] = NULL,
	[I_TEXT_TYPE_SINGLELINE_TEXT] = NoNewlineCharFilter,
	[I_TEXT_TYPE_INTEGER] = IntegerCharFilter,
	[I_TEXT_TYPE_HEX_COLOR] = HexColorCharFilter,
};


void
IUpdateElement(
	IHandle* Element
	)
{
	UIUpdateElement(Element->source);
}


void
IActivateElement(
	IHandle* Element
	)
{
	UIActivateElement(Element->source);
}


void
IDeactivateElement(
	IHandle* Element
	)
{
	UIDeactivateElement(Element->source);
}


void
IAddElementFirst(
	IHandle* Element,
	IHandle* Parent
	)
{
	UIAddElementFirst(Element->source, Parent->Destination);
}


void
IAddElementLast(
	IHandle* Element,
	IHandle* Parent
	)
{
	UIAddElementLast(Element->source, Parent->Destination);
}


void
IAddElementBefore(
	IHandle* Element,
	IHandle* Before
	)
{
	UIAddElementBefore(Element->source, Before->source);
}


void
IAddElementAfter(
	IHandle* Element,
	IHandle* After
	)
{
	UIAddElementAfter(Element->source, After->source);
}


void
IUnlinkElement(
	IHandle* Element
	)
{
	UIUnlinkElement(Element->source);
}


void
IRemoveElement(
	IHandle* Element
	)
{
	UIRemoveElement(Element->source);
}





typedef struct IPrivateContainerData
{
	UICallback Callback;
	void* data;
}
IPrivateContainerData;


void
IContainerCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	assert_not_null(Element->data);
	IPrivateContainerData* data = Element->data;

	assert_true(Element->Scrollable);

	assert_eq(Element->type, UI_TYPE_CONTAINER);
	UIContainer* Parent = &Element->Container;
	assert_not_null(Parent->head);

	UIElement* Scrollable = Parent->head;
	assert_eq(Scrollable->type, UI_TYPE_CONTAINER);
	UIContainer* Container = &Scrollable->Container;

	UIElement* Bar = Parent->tail;
	assert_eq(Scrollable->next, Bar);


	switch(Event)
	{

	case UI_EVENT_FREE:
	{
		data->Callback(Element, UI_EVENT_FREE);
		free(data);

		break;
	}

	case UI_EVENT_SCROLL:
	{
		if(Container->Axis == UI_AXIS_VERTICAL)
		{
			Container->GoalOffsetY += ScrollOffset * SCROLL_STRENGTH;
		}
		else
		{
			Container->GoalOffsetX += ScrollOffset * SCROLL_STRENGTH;
		}

		fallthrough();
	}

	default:
	{
		IPrivateContainerData* data = Element->data;
		data->Callback(Element, Event);
		break;
	}

	}
}


void
IScrollableContainerCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	assert_eq(Element->type, UI_TYPE_CONTAINER);

	UIElement* Parent = Element->Parent;
	assert_not_null(Parent);
	assert_eq(Parent->type, UI_TYPE_CONTAINER);

	assert_not_null(Parent->data);
	IPrivateContainerData* data = Parent->data;

	UIElement* Scrollbar = Element->next;
	assert_eq(Scrollbar->type, UI_TYPE_SCROLLBAR);


	switch(Event)
	{

	case UI_EVENT_MOUSE_IN:
	case UI_EVENT_MOUSE_OUT:
		/* Handled by the parent, don't want duplicates */
		break;

	case UI_EVENT_RESIZE:
	{
		UIScrollbarUpdate(Scrollbar);

		fallthrough();
	}

	case UI_EVENT_FREE: break;

	default:
	{
		data->Callback(Parent, Event);
		break;
	}

	}
}


IHandle
ICreateContainer(
	const IElement* Element,
	const IContainer* Container
	)
{
	IHandle handle;

	if(!Container->Scrollable)
	{
		UIElement* C = UIGetElement();
		*C =
		(UIElement)
		{
			.x = Element->x,
			.y = Element->y,
			.w = Element->w,
			.h = Element->h,

			.MarginTop = Element->MarginTop,
			.MarginLeft = Element->MarginLeft,
			.MarginRight = Element->MarginRight,
			.MarginBottom = Element->MarginBottom,

			.BorderTop = Element->BorderTop,
			.BorderLeft = Element->BorderLeft,
			.BorderRight = Element->BorderRight,
			.BorderBottom = Element->BorderBottom,

			.Opacity = Element->Opacity,
			.BorderColor = Element->BorderColor,

			.AlignX = Element->AlignX,
			.AlignY = Element->AlignY,
			.pos = Element->pos,
			.RelativeAlignX = Element->RelativeAlignX,
			.RelativeAlignY = Element->RelativeAlignY,

			.AutoW = Container->AutoW,
			.AutoH = Container->AutoH,
			.Clickable = Container->Clickable,
			.ClickPassthrough = Container->ClickPassthrough,
			.Selectable = Container->Selectable,
			.ScrollPassthrough = Container->ScrollPassthrough,
			.InteractiveBorder = Element->InteractiveBorder,

			.TextFocus = Container->TextFocus,
			.Relative = Element->Relative ? Element->Relative->source : NULL,

			.Callback = Element->Callback,
			.data = Element->data,

			.type = UI_TYPE_CONTAINER,
			.Container =
			(UIContainer)
			{
				.Axis = Container->Axis,

				.white_color = Container->white_color,
				.black_color = Container->black_color,
				.tex = Container->tex
			}
		};

		UIInitialize(C);

		handle.source = handle.Destination = C;
	}
	else
	{
		assert_false((Container->AutoW || Container->AutoH));

		IPrivateContainerData* data = malloc(sizeof(IPrivateContainerData));
		assert_not_null(data);

		data->Callback = Element->Callback ? Element->Callback : UIEmptyCallback;
		data->data = Element->data;

		UIElement* P = UIGetElement();
		*P =
		(UIElement)
		{
			.x = Element->x,
			.y = Element->y,
			.w = Element->w,
			.h = Element->h,

			.MarginTop = Element->MarginTop,
			.MarginLeft = Element->MarginLeft,
			.MarginRight = Element->MarginRight,
			.MarginBottom = Element->MarginBottom,

			.BorderTop = Element->BorderTop,
			.BorderLeft = Element->BorderLeft,
			.BorderRight = Element->BorderRight,
			.BorderBottom = Element->BorderBottom,

			.Opacity = Element->Opacity,
			.BorderColor = Element->BorderColor,

			.AlignX = Element->AlignX,
			.AlignY = Element->AlignY,
			.pos = Element->pos,
			.RelativeAlignX = Element->RelativeAlignX,
			.RelativeAlignY = Element->RelativeAlignY,

			.AutoW = Container->AutoW,
			.AutoH = Container->AutoH,
			.Clickable = Container->Clickable,
			.ClickPassthrough = Container->ClickPassthrough,
			.Selectable = Container->Selectable,
			.Scrollable = true,
			.ScrollPassthrough = Container->ScrollPassthrough,
			.InteractiveBorder = Element->InteractiveBorder,

			.Relative = Element->Relative ? Element->Relative->source : NULL,

			.Callback = IContainerCallback,
			.data = data,

			.type = UI_TYPE_CONTAINER,
			.Container =
			(UIContainer)
			{
				.Axis = !Container->Axis,

				.white_color = Container->white_color,
				.black_color = Container->black_color,
				.tex = Container->tex
			}
		};

		UIInitialize(P);

		float w;
		float h;
		bool AutoW;
		bool AutoH;
		float ScrollbarW;
		float ScrollbarH;

		if(Container->Axis == UI_AXIS_HORIZONTAL)
		{
			/* No point in making it scrollable if content never overflows */
			assert_false(Container->AutoW);

			w = 0.0f;
			AutoW = true;

			if(Container->AutoH)
			{
				h = 0.0f;
				AutoH = true;
			}
			else
			{
				h = Element->h - SCROLLBAR_SIZE;
				AutoH = false;
			}

			ScrollbarW = Element->w;
			ScrollbarH = SCROLLBAR_SIZE;
		}
		else
		{
			assert_false(Container->AutoH);

			if(Container->AutoW)
			{
				w = 0.0f;
				AutoW = true;
			}
			else
			{
				w = Element->w - SCROLLBAR_SIZE;
				AutoW = false;
			}

			h = 0.0f;
			AutoH = true;

			ScrollbarW = SCROLLBAR_SIZE;
			ScrollbarH = Element->h;
		}

		UIElement* C = UIGetElement();
		*C =
		(UIElement)
		{
			.w = w,
			.h = h,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP,

			.AutoW = AutoW,
			.AutoH = AutoH,
			.Scrollable = true,
			.ScrollPassthrough = true,

			.Callback = IScrollableContainerCallback,

			.type = UI_TYPE_CONTAINER,
			.Container =
			(UIContainer)
			{
				.Axis = Container->Axis
			}
		};

		UIInitialize(C);

		UIElement* s = UIGetElement();
		*s =
		(UIElement)
		{
			.w = ScrollbarW,
			.h = ScrollbarH,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP,

			.Clickable = true,
			.ScrollPassthrough = true,

			.Callback = UIScrollbarCallback,

			.type = UI_TYPE_SCROLLBAR,
			.Scrollbar =
			(UIScrollbar)
			{
				.Axis = Container->Axis,

				.color = Container->ScrollbarColor,
				.AltColor = Container->ScrollbarAltColor
			}
		};

		UIInitialize(s);

		UIAddElementLast(C, P);
		UIAddElementLast(s, P);

		handle.source = P;
		handle.Destination = C;
	}

	return handle;
}





typedef struct IPrivateTextData
{
	UICallback Callback;
	void* data;

	ITextType type;
	ITextData TextData;

	UIElement* Placeholder;
}
IPrivateTextData;


const uint32_t*
SkipCodepoint(
	const uint32_t* Str,
	const uint32_t Char
	)
{
	const uint32_t* s = Str;

	while(*s && *s == Char)
	{
		++s;
	}

	if(s != Str)
	{
		--s;
	}

	return s;
}


void
ITextWrite(
	UIElement* Element,
	char* Str,
	uint32_t len,
	bool Changed
	)
{
	assert_eq(Element->type, UI_TYPE_TEXT);
	UIText* Text = &Element->Text;

	uint32_t* Codepoints = malloc(len * sizeof(uint32_t));
	assert_not_null(Codepoints);

	uint32_t count = utf8proc_decompose((const uint8_t*) Str,
		len, (int32_t*) Codepoints, len, UTF8PROC_NULLTERM);
	assert_le(count, len);

	Codepoints = realloc(Codepoints, count * sizeof(uint32_t));
	assert_not_null(Codepoints);

	free(Text->Codepoints);
	Text->Codepoints = Codepoints;
	Text->len = count;

	UIUpdateElement(Element);

	IPrivateTextData* data = Element->data;
	assert_not_null(data);

	if(Changed)
	{
		data->Callback(Element, UI_EVENT_CHANGE);
	}
}


typedef char*
(*ValueToTextFunc)(
	char* Str,
	uint32_t* len,
	const ITextData* TextData
	);

typedef void
(*TextSubmitFunc)(
	UIElement* Element
	);


char*
IntegerToText(
	char* Str,
	uint32_t* len,
	const ITextData* TextData
	)
{
	if(!Str)
	{
		Str = malloc(32);
		assert_not_null(Str);
	}

	int64_t value = TextData->Integer.value;

	int Len = snprintf(Str, 32, "%" PRIi64, value);
	assert_gt(Len, 0);

	if(len)
	{
		*len = Len;
	}

	return Str;
}


void
ITextOnIntegerSubmit(
	UIElement* Element
	)
{
	assert_eq(Element->type, UI_TYPE_TEXT);
	UIText* Text = &Element->Text;

	assert_not_null(Element->data);
	IPrivateTextData* data = Element->data;
	assert_eq(data->type, I_TEXT_TYPE_INTEGER);
	ITextData* TextData = &data->TextData;
	ITextInteger* Integer = &TextData->Integer;

	const uint32_t* s = SkipCodepoint(Text->Codepoints, ' ');
	s = SkipCodepoint(s, '-');
	s = SkipCodepoint(s, '+');

	int64_t value = MACRO_MIN(MACRO_MAX(strtoll(s, NULL, 10), Integer->min), Integer->max);
	bool Changed = Integer->value != value;
	Integer->value = value;

	char Str[32];
	uint32_t len;
	IntegerToText(Str, &len, TextData);

	ITextWrite(Element, Str, len, Changed);
}


char*
HexColorToText(
	char* Str,
	uint32_t* len,
	const ITextData* TextData
	)
{
	if(!Str)
	{
		Str = malloc(16);
		assert_not_null(Str);
	}

	color_argb_t color = TextData->HexColor.color;

	int Len = snprintf(Str, 16, "#%02X%02X%02X%02X", color.r, color.g, color.b, color.a);
	assert_gt(Len, 0);
	*len = Len;

	return Str;
}


void
ITextOnHexColorSubmit(
	UIElement* Element
	)
{
	assert_eq(Element->type, UI_TYPE_TEXT);
	UIText* Text = &Element->Text;

	assert_not_null(Element->data);
	IPrivateTextData* data = Element->data;
	assert_eq(data->type, I_TEXT_TYPE_HEX_COLOR);
	ITextData* TextData = &data->TextData;
	ITextHexColor* HexColor = &TextData->HexColor;

	const char* s = SkipCodepoint(Text->Str, ' ');
	s = SkipCodepoint(s, '#');
	if(*s == '#')
	{
		++s;
	}


	uint32_t value = strtoul(s, NULL, 16);
	color_argb_t color;


	switch(strlen(s))
	{

	case 1:
	{
		color =
		(color_argb_t)
		{
			.b = value * 0x11,
			.g = value * 0x11,
			.r = value * 0x11,
			.a = 0xFF
		};

		break;
	}

	case 3:
	{
		color =
		(color_argb_t)
		{
			.b = (value & 0xF) * 0x11,
			.g = ((value >> 4) & 0xF) * 0x11,
			.r = ((value >> 8) & 0xF) * 0x11,
			.a = 0xFF
		};

		break;
	}

	case 4:
	{
		color =
		(color_argb_t)
		{
			.b = ((value >> 4) & 0xF) * 0x11,
			.g = ((value >> 8) & 0xF) * 0x11,
			.r = ((value >> 12) & 0xF) * 0x11,
			.a = (value & 0xF) * 0x11
		};

		break;
	}

	case 6:
	{
		color =
		(color_argb_t)
		{
			.b = value & 0xFF,
			.g = (value >> 8) & 0xFF,
			.r = (value >> 16) & 0xFF,
			.a = 0xFF
		};

		break;
	}

	case 8:
	{
		color =
		(color_argb_t)
		{
			.b = (value >> 8) & 0xFF,
			.g = (value >> 16) & 0xFF,
			.r = (value >> 24) & 0xFF,
			.a = value & 0xFF
		};

		break;
	}

	default:
	{
		color = HexColor->color;
		break;
	}

	}


	bool Changed = HexColor->color.color_argb_t != color.color_argb_t;
	HexColor->color = color;

	char Str[16];
	uint32_t len;
	HexColorToText(Str, &len, TextData);

	ITextWrite(Element, Str, len, Changed);
}


ValueToTextFunc ValueToText[] =
{
	[I_TEXT_TYPE_MULTILINE_TEXT] = NULL,
	[I_TEXT_TYPE_SINGLELINE_TEXT] = NULL,
	[I_TEXT_TYPE_INTEGER] = IntegerToText,
	[I_TEXT_TYPE_HEX_COLOR] = HexColorToText,
};


TextSubmitFunc TextSubmit[] =
{
	[I_TEXT_TYPE_MULTILINE_TEXT] = NULL,
	[I_TEXT_TYPE_SINGLELINE_TEXT] = NULL,
	[I_TEXT_TYPE_INTEGER] = ITextOnIntegerSubmit,
	[I_TEXT_TYPE_HEX_COLOR] = ITextOnHexColorSubmit,
};


void
ITextCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	assert_eq(Element->type, UI_TYPE_TEXT);
	UIText* Text = &Element->Text;

	assert_not_null(Element->data);
	IPrivateTextData* data = Element->data;

	UITextCallback(Element, Event);


	switch(Event)
	{

	case UI_EVENT_CHANGE:
	{
		if(!data->Placeholder)
		{
			break;
		}

		if(Text->Str[0] == 0)
		{
			if(!UIIsLinked(data->Placeholder))
			{
				UIAddElementBefore(data->Placeholder, Element);
			}
		}
		else
		{
			if(UIIsLinked(data->Placeholder))
			{
				UIUnlinkElement(data->Placeholder);
			}
		}

		break;
	}

	case UI_EVENT_SUBMIT:
	{
		TextSubmitFunc OnSubmit = TextSubmit[data->type];
		if(OnSubmit)
		{
			OnSubmit(Element);
		}

		data->Callback(Element, UI_EVENT_SUBMIT);
		break;
	}

	case UI_EVENT_FREE:
	{
		free(Text->Str);

		data->Callback(Element, UI_EVENT_FREE);
		free(data);

		break;
	}

	default:
	{
		data->Callback(Element, Event);
		break;
	}

	}
}


void
ITextPlaceholderCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	assert_eq(Element->type, UI_TYPE_TEXT);
	UIText* Text = &Element->Text;

	UITextCallback(Element, Event);


	switch(Event)
	{

	case UI_EVENT_MOUSE_DOWN:
	{
		assert_not_null(Element->next);
		UIElement* ActualText = Element->next;
		UIFocusText(ActualText);
		break;
	}

	case UI_EVENT_FREE:
	{
		free(Text->Str);
		break;
	}

	default: break;

	}
}


void
ITextContainerCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	assert_eq(Element->type, UI_TYPE_CONTAINER);
	UIContainer* Container = &Element->Container;

	if(Event != UI_EVENT_MOUSE_DOWN)
	{
		return;
	}

	assert_not_null(Container->tail);
	UIElement* ActualText = Container->tail;
	UIFocusText(ActualText);
}


IHandle
ICreateText(
	const IElement* Element,
	const IContainer* Container,
	const IText* Text
	)
{
	IPrivateTextData* data = malloc(sizeof(IPrivateTextData));
	assert_not_null(data);

	data->Callback = Element->Callback ? Element->Callback : UIEmptyCallback;
	data->data = Element->data;

	data->type = Text->type;
	data->TextData = Text->data;

	assert_lt(Text->type, I_TEXT_TYPE__COUNT);
	const char* CharFilter = TextCharFilters[Text->type];

	char* Str;
	uint32_t len;
	if(!Text->Str)
	{
		ValueToTextFunc ToText = ValueToText[Text->type];
		if(ToText)
		{
			Str = ToText(NULL, &len, &Text->data);
		}
		else
		{
			Str = malloc(1);
			assert_not_null(Str);

			Str[0] = 0;
			len = 0;
		}
	}
	else
	{
		if(!Text->len)
		{
			len = strlen(Text->Str);
		}
		else
		{
			len = Text->len;
		}

		Str = malloc(len + 1);
		assert_not_null(Str);

		memcpy(Str, Text->Str, len + 1);
	}

	UIElement* T = UIGetElement();
	*T =
	(UIElement)
	{
		.Opacity = Element->Opacity,

		.AlignX = Element->AlignX,
		.AlignY = Element->AlignY,
		.pos = Element->pos,
		.RelativeAlignX = Element->RelativeAlignX,
		.RelativeAlignY = Element->RelativeAlignY,

		.AutoH = true,
		.Clickable = true,
		.Selectable = true,
		.ScrollPassthrough = true,

		.Callback = ITextCallback,
		.data = data,

		.type = UI_TYPE_TEXT,
		.Text =
		(UIText)
		{
			.Str = Str,
			.len = Text->len,
			.max_len = Text->max_len,
			.MaxWidth = Text->MaxWidth,

			.FontSize = Text->FontSize,
			.AlignX = Text->AlignX,

			.Selectable = Text->Selectable,
			.Editable = Text->Editable,

			.Stroke = Text->Stroke,
			.InverseStroke = Text->InverseStroke,
			.Fill = Text->Fill,
			.InverseFill = Text->InverseFill,
			.Background = Text->Background,

			.CharFilter = CharFilter
		}
	};

	UIInitialize(T);

	if(Text->Placeholder)
	{
		UIElement* P = UIGetElement();
		*P =
		(UIElement)
		{
			.Opacity = color_a_mul_a(Element->Opacity, 0xB0),

			.AlignX = Text->AlignX,
			.AlignY = UI_ALIGN_TOP,
			.pos = UI_POSITION_INLINE,

			.AutoH = true,
			.Clickable = true,
			.Selectable = true,
			.ScrollPassthrough = true,

			.TextFocus = T,

			.Callback = ITextPlaceholderCallback,

			.type = UI_TYPE_TEXT,
			.Text =
			(UIText)
			{
				.Str = strdup(Text->Placeholder),
				.max_len = Text->max_len,
				.MaxWidth = Text->MaxWidth,

				.FontSize = Text->FontSize,
				.AlignX = Text->AlignX,

				.Stroke = Text->Stroke,
				.InverseStroke = Text->InverseStroke,
				.Fill = Text->Fill,
				.InverseFill = Text->InverseFill,
				.Background = Text->Background
			}
		};

		UIInitialize(P);
		UIActivateElement(P);

		data->Placeholder = P;
	}
	else
	{
		data->Placeholder = NULL;
	}

	IHandle C = ICreateContainer(
		&((IElement)
		{
			.x = Element->x,
			.y = Element->y,
			.w = Element->w,
			.h = Element->h,

			.MarginTop = Element->MarginTop,
			.MarginLeft = Element->MarginLeft,
			.MarginRight = Element->MarginRight,
			.MarginBottom = Element->MarginBottom,

			.BorderTop = Element->BorderTop,
			.BorderLeft = Element->BorderLeft,
			.BorderRight = Element->BorderRight,
			.BorderBottom = Element->BorderBottom,

			.Opacity = Element->Opacity,
			.BorderColor = Element->BorderColor,

			.AlignX = Element->AlignX,
			.AlignY = Element->AlignY,
			.pos = Element->pos,
			.RelativeAlignX = Element->RelativeAlignX,
			.RelativeAlignY = Element->RelativeAlignY,

			.InteractiveBorder = Element->InteractiveBorder,

			.Relative = Element->Relative,

			.Callback = Container ? ITextContainerCallback : NULL
		}
		), Container ?
		&((IContainer)
		{
			.AutoW = Container->AutoW,
			.AutoH = Container->AutoH,
			.Clickable = true,
			.Selectable = true,
			.ScrollPassthrough = true,

			.white_color = Container->white_color,
			.black_color = Container->black_color,
			.tex = Container->tex,

			.TextFocus = T
		}) :
		&((IContainer)
		{
			.AutoW = true,
			.AutoH = true,
			.ClickPassthrough = true,
			.ScrollPassthrough = true,

			.TextFocus = T
		})
	);

	UIAddElementLast(T, C.Destination);

	return (IHandle) { .source = C.source, .Destination = NULL };
}





typedef struct IPrivateCheckboxData
{
	UICallback Callback;
	void* data;
}
IPrivateCheckboxData;


void
ICheckboxCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	assert_eq(Element->type, UI_TYPE_CHECKBOX);
	UICheckbox* Checkbox = &Element->Checkbox;

	(void) Checkbox;

	assert_not_null(Element->data);
	IPrivateCheckboxData* data = Element->data;

	UICheckboxCallback(Element, Event);

	data->Callback(Element, Event);
}


IHandle
ICreateCheckbox(
	const IElement* Element,
	const ICheckbox* Checkbox
	)
{
	IPrivateCheckboxData* data = malloc(sizeof(IPrivateCheckboxData));
	assert_not_null(data);

	data->Callback = Element->Callback ? Element->Callback : UIEmptyCallback;
	data->data = Element->data;

	UIElement* T = UIGetElement();
	*T =
	(UIElement)
	{
		.x = Element->x,
		.y = Element->y,
		.w = Element->w,
		.h = Element->h,

		.MarginTop = Element->MarginTop,
		.MarginLeft = Element->MarginLeft,
		.MarginRight = Element->MarginRight,
		.MarginBottom = Element->MarginBottom,

		.BorderTop = Element->BorderTop,
		.BorderLeft = Element->BorderLeft,
		.BorderRight = Element->BorderRight,
		.BorderBottom = Element->BorderBottom,

		.Opacity = Element->Opacity,
		.BorderColor = Element->BorderColor,

		.AlignX = Element->AlignX,
		.AlignY = Element->AlignY,
		.pos = Element->pos,
		.RelativeAlignX = Element->RelativeAlignX,
		.RelativeAlignY = Element->RelativeAlignY,

		.Clickable = true,
		.InteractiveBorder = Element->InteractiveBorder,

		.Relative = Element->Relative ? Element->Relative->source : NULL,

		.Callback = ICheckboxCallback,
		.data = data,

		.type = UI_TYPE_CHECKBOX,
		.Checkbox =
		(UICheckbox)
		{
			.Checked = Checkbox->Checked,

			.CheckYes = Checkbox->CheckYes,
			.CheckNo = Checkbox->CheckNo,
			.Background = Checkbox->Background
		}
	};

	UIInitialize(T);

	return (IHandle) { .source = T, .Destination = NULL };
}


UIElement*
ITextGetTextElement(
	IHandle Text
	)
{
	UIElement* T;

	if(Text.source->type == UI_TYPE_CONTAINER)
	{
		T = Text.source->Container.head;
	}
	else
	{
		T = Text.source;
	}
	assert_eq(T->type, UI_TYPE_TEXT);

	return T;
}


int64_t
ITextGetInteger(
	IHandle Text
	)
{
	UIElement* T = ITextGetTextElement(Text);

	IPrivateTextData* data = T->data;
	assert_not_null(data);

	ITextData* TextData = &data->TextData;
	assert_eq(data->type, I_TEXT_TYPE_INTEGER);
	ITextInteger* Integer = &TextData->Integer;

	return Integer->value;
}


void
ITextSetIntegerExplicit(
	IHandle Text,
	int64_t value,
	bool Changed
	)
{
	UIElement* T = ITextGetTextElement(Text);

	IPrivateTextData* data = T->data;
	assert_not_null(data);

	ITextData* TextData = &data->TextData;
	assert_eq(data->type, I_TEXT_TYPE_INTEGER);
	ITextInteger* Integer = &TextData->Integer;

	Integer->value = value;

	char Str[32];
	uint32_t len;
	IntegerToText(Str, &len, TextData);

	ITextWrite(T, Str, len, Changed);
}


void
ITextSetInteger(
	IHandle Text,
	int64_t value
	)
{
	ITextSetIntegerExplicit(Text, value, true);
}


color_argb_t
ITextGetHexColor(
	IHandle Text
	)
{
	UIElement* T = ITextGetTextElement(Text);

	IPrivateTextData* data = T->data;
	assert_not_null(data);

	ITextData* TextData = &data->TextData;
	assert_eq(data->type, I_TEXT_TYPE_HEX_COLOR);
	ITextHexColor* HexColor = &TextData->HexColor;

	return HexColor->color;
}


void
ITextSetHexColorExplicit(
	IHandle Text,
	color_argb_t color,
	bool Changed
	)
{
	UIElement* T = ITextGetTextElement(Text);

	IPrivateTextData* data = T->data;
	assert_not_null(data);

	ITextData* TextData = &data->TextData;
	assert_eq(data->type, I_TEXT_TYPE_HEX_COLOR);
	ITextHexColor* HexColor = &TextData->HexColor;

	HexColor->color = color;

	char Str[16];
	uint32_t len;
	HexColorToText(Str, &len, TextData);

	ITextWrite(T, Str, len, Changed);
}


void
ITextSetHexColor(
	IHandle value,
	color_argb_t color
	)
{
	ITextSetHexColorExplicit(value, color, true);
}





typedef struct IPrivateSliderData
{
	UICallback Callback;
	void* data;
}
IPrivateSliderData;


void
ISliderCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	assert_eq(Element->type, UI_TYPE_SLIDER);
	// UISlider* Slider = &Element->Slider;

	assert_not_null(Element->data);
	IPrivateSliderData* data = Element->data;

	UISliderCallback(Element, Event);

	data->Callback(Element, Event);
}


IHandle
ICreateSlider(
	const IElement* Element,
	const ISlider* Slider
	)
{
	assert_ge(Slider->value, 0);
	assert_lt(Slider->value, Slider->Sections);

	IPrivateSliderData* data = malloc(sizeof(IPrivateSliderData));
	assert_not_null(data);

	data->Callback = Element->Callback ? Element->Callback : UIEmptyCallback;
	data->data = Element->data;

	UIElement* T = UIGetElement();
	*T =
	(UIElement)
	{
		.x = Element->x,
		.y = Element->y,
		.w = Element->w,
		.h = Element->h,

		.MarginTop = Element->MarginTop,
		.MarginLeft = Element->MarginLeft,
		.MarginRight = Element->MarginRight,
		.MarginBottom = Element->MarginBottom,

		.BorderTop = Element->BorderTop,
		.BorderLeft = Element->BorderLeft,
		.BorderRight = Element->BorderRight,
		.BorderBottom = Element->BorderBottom,

		.Opacity = Element->Opacity,
		.BorderColor = Element->BorderColor,

		.AlignX = Element->AlignX,
		.AlignY = Element->AlignY,
		.pos = Element->pos,
		.RelativeAlignX = Element->RelativeAlignX,
		.RelativeAlignY = Element->RelativeAlignY,

		.Clickable = true,
		.InteractiveBorder = Element->InteractiveBorder,

		.Relative = Element->Relative ? Element->Relative->source : NULL,

		.Callback = ISliderCallback,
		.data = data,

		.type = UI_TYPE_SLIDER,
		.Slider =
		(UISlider)
		{
			.Axis = Slider->Axis,
			.Sections = Slider->Sections,
			.value = Slider->value,

			.color = Slider->color,
			.BgColor = Slider->BgColor,

			.type = UI_SLIDER_TYPE_NORMAL
		}
	};

	UIInitialize(T);

	return (IHandle) { .source = T, .Destination = NULL };
}





void
IColorPickerUpdate(
	color_argb_t color
	)
{
	color_argb_t OpaqueColor = color;
	OpaqueColor.a = 255;

	ColorPickerElement->ColorPicker.ColorRGB = OpaqueColor;
	float value = UIColorPickerUpdate(ColorPickerElement);
	uint32_t Brightness = nearbyintf(value * 255.0f);

	ColorPickerBrightness.Slider->color = ColorPickerElement->ColorPicker.ColorRGB;
	ColorPickerOpacity.Slider->color = OpaqueColor;
	ColorPickerRed.Slider->color = OpaqueColor;
	ColorPickerGreen.Slider->color = OpaqueColor;
	ColorPickerBlue.Slider->color = OpaqueColor;

	CurrentColorContainer->Container.white_color = color;
	ColorPickerBackground->Container.white_color = color;

	ColorPickerBrightness.Slider->value = Brightness;
	ColorPickerOpacity.Slider->value = color.a;
	ColorPickerRed.Slider->value = color.r;
	ColorPickerGreen.Slider->value = color.g;
	ColorPickerBlue.Slider->value = color.b;

	ITextSetHexColorExplicit(ColorPickerHexText, color, false);

	ITextSetIntegerExplicit(ColorPickerBrightness.value, Brightness, false);
	ITextSetIntegerExplicit(ColorPickerOpacity.value, color.a, false);
	ITextSetIntegerExplicit(ColorPickerRed.value, color.r, false);
	ITextSetIntegerExplicit(ColorPickerGreen.value, color.g, false);
	ITextSetIntegerExplicit(ColorPickerBlue.value, color.b, false);

	CurrentColorContainer->Callback(CurrentColorContainer, UI_EVENT_CHANGE);
}


void
IHexTextCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UITextCallback(Element, Event);

	if(Event != UI_EVENT_SUBMIT)
	{
		return;
	}

	color_argb_t color = ITextGetHexColor(ColorPickerHexText);
	IColorPickerUpdate(color);
}


void
IColorPickerCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UIColorPickerCallback(Element, Event);


	switch(Event)
	{

	case UI_EVENT_MOUSE_DOWN:
	case UI_EVENT_MOUSE_MOVE:
	{
		color_hsv_t ColorHSV = color_argb_to_hsv(CurrentColorContainer->Container.white_color);
		color_hsv_t PickedColor = ColorPickerElement->ColorPicker.ColorHSV;
		ColorHSV.h = PickedColor.h;
		ColorHSV.s = PickedColor.s;
		color_argb_t ColorRGB = color_hsv_to_argb(ColorHSV);
		ColorRGB.a = CurrentColorContainer->Container.white_color.a;
		IColorPickerUpdate(ColorRGB);
		break;
	}

	default: break;

	}
}


void
IBrightnessSliderRecalculate(
	void
	)
{
	color_hsv_t ColorHSV = ColorPickerElement->ColorPicker.ColorHSV;
	ColorHSV.v = ColorPickerBrightness.Slider->value / 255.0f;
	color_argb_t ColorRGB = color_hsv_to_argb(ColorHSV);
	ColorPickerElement->ColorPicker.ColorRGB = ColorRGB;
	ColorRGB.a = ColorPickerOpacity.Slider->value;
	IColorPickerUpdate(ColorRGB);
}


void
IBrightnessSliderCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UISliderCallback(Element, Event);


	switch(Event)
	{

	case UI_EVENT_MOUSE_DOWN:
	case UI_EVENT_MOUSE_MOVE:
	{
		IBrightnessSliderRecalculate();
		break;
	}

	default: break;

	}
}


void
IBrightnessTextCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UITextCallback(Element, Event);

	if(Event != UI_EVENT_SUBMIT)
	{
		return;
	}

	ColorPickerBrightness.Slider->value = ITextGetInteger(ColorPickerBrightness.value);
	IBrightnessSliderRecalculate();
}


void
IOpacitySliderRecalculate(
	void
	)
{
	color_argb_t ColorRGB = CurrentColorContainer->Container.white_color;
	ColorRGB.a = ColorPickerOpacity.Slider->value;
	IColorPickerUpdate(ColorRGB);
}


void
IOpacitySliderCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UISliderCallback(Element, Event);


	switch(Event)
	{

	case UI_EVENT_MOUSE_DOWN:
	case UI_EVENT_MOUSE_MOVE:
	{
		IOpacitySliderRecalculate();
		break;
	}

	default: break;

	}
}


void
IOpacityTextCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UITextCallback(Element, Event);

	if(Event != UI_EVENT_SUBMIT)
	{
		return;
	}

	ColorPickerOpacity.Slider->value = ITextGetInteger(ColorPickerOpacity.value);
	IOpacitySliderRecalculate();
}


void
IRedSliderRecalculate(
	void
	)
{
	color_argb_t ColorRGB = CurrentColorContainer->Container.white_color;
	ColorRGB.r = ColorPickerRed.Slider->value;
	IColorPickerUpdate(ColorRGB);
}


void
IRedSliderCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UISliderCallback(Element, Event);


	switch(Event)
	{

	case UI_EVENT_MOUSE_DOWN:
	case UI_EVENT_MOUSE_MOVE:
	{
		IRedSliderRecalculate();
		break;
	}

	default: break;

	}
}


void
IRedTextCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UITextCallback(Element, Event);

	if(Event != UI_EVENT_SUBMIT)
	{
		return;
	}

	ColorPickerRed.Slider->value = ITextGetInteger(ColorPickerRed.value);
	IRedSliderRecalculate();
}


void
IGreenSliderRecalculate(
	void
	)
{
	color_argb_t ColorRGB = CurrentColorContainer->Container.white_color;
	ColorRGB.g = ColorPickerGreen.Slider->value;
	IColorPickerUpdate(ColorRGB);
}


void
IGreenSliderCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UISliderCallback(Element, Event);


	switch(Event)
	{

	case UI_EVENT_MOUSE_DOWN:
	case UI_EVENT_MOUSE_MOVE:
	{
		IGreenSliderRecalculate();
		break;
	}

	default: break;

	}
}


void
IGreenTextCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UITextCallback(Element, Event);

	if(Event != UI_EVENT_SUBMIT)
	{
		return;
	}

	ColorPickerGreen.Slider->value = ITextGetInteger(ColorPickerGreen.value);
	IGreenSliderRecalculate();
}


void
IBlueSliderRecalculate(
	void
	)
{
	color_argb_t ColorRGB = CurrentColorContainer->Container.white_color;
	ColorRGB.b = ColorPickerBlue.Slider->value;
	IColorPickerUpdate(ColorRGB);
}


void
IBlueSliderCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UISliderCallback(Element, Event);


	switch(Event)
	{

	case UI_EVENT_MOUSE_DOWN:
	case UI_EVENT_MOUSE_MOVE:
	{
		IBlueSliderRecalculate();
		break;
	}

	default: break;

	}
}


void
IBlueTextCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	UITextCallback(Element, Event);

	if(Event != UI_EVENT_SUBMIT)
	{
		return;
	}

	ColorPickerBlue.Slider->value = ITextGetInteger(ColorPickerBlue.value);
	IBlueSliderRecalculate();
}


UICallback ColorPickerSliderCallback[] =
{
	[UI_SLIDER_TYPE_BRIGHTNESS] = IBrightnessSliderCallback,
	[UI_SLIDER_TYPE_OPACITY] = IOpacitySliderCallback,
	[UI_SLIDER_TYPE_RED] = IRedSliderCallback,
	[UI_SLIDER_TYPE_GREEN] = IGreenSliderCallback,
	[UI_SLIDER_TYPE_BLUE] = IBlueSliderCallback
};


UICallback ColorPickerTextCallback[] =
{
	[UI_SLIDER_TYPE_BRIGHTNESS] = IBrightnessTextCallback,
	[UI_SLIDER_TYPE_OPACITY] = IOpacityTextCallback,
	[UI_SLIDER_TYPE_RED] = IRedTextCallback,
	[UI_SLIDER_TYPE_GREEN] = IGreenTextCallback,
	[UI_SLIDER_TYPE_BLUE] = IBlueTextCallback
};


IColorPickerLine
IColorPickerCreateLine(
	const char* name,
	const char* ShortName,
	UISliderType SliderType,
	IHandle* ContentContainer
	)
{
	IHandle C = ICreateContainer(
		&((IElement)
		{
			.w = 410.0f,

			.MarginTop = 4.0f,
			.MarginLeft = 4.0f,
			.MarginRight = 4.0f,
			.MarginBottom = 4.0f,

			.BorderTop = 6.0f,
			.BorderLeft = 6.0f,
			.BorderRight = 6.0f,
			.BorderBottom = 6.0f,

			.Opacity = 0xFF,
			.BorderColor = (color_argb_t){ 0x20FFFFFF },

			.AlignX = UI_ALIGN_CENTER,
			.AlignY = UI_ALIGN_TOP
		}),
		&((IContainer)
		{
			.Axis = UI_AXIS_HORIZONTAL,

			.AutoH = true,

			.white_color = (color_argb_t){ 0x20FFFFFF },
			.tex = TEXTURE_RECT
		})
	);

	IHandle T = ICreateText(
		&((IElement)
		{
			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_MIDDLE
		}),
		NULL,
		&((IText)
		{
			.Str = name,
			.FontSize = 18,
			.AlignX = UI_ALIGN_CENTER,

			.Stroke = (color_argb_t){ 0xFF000000 },
			.Fill = (color_argb_t){ 0xFFFFFFFF }
		})
	);

	UIElement* s = UIGetElement();
	*s =
	(UIElement)
	{
		.w = 256.0f + SLIDER_SIZE * 2,
		.h = SLIDER_SIZE,

		.MarginRight = 4.0f,

		.Opacity = 0xFF,

		.AlignX = UI_ALIGN_RIGHT,
		.AlignY = UI_ALIGN_MIDDLE,

		.Callback = ColorPickerSliderCallback[SliderType],

		.type = UI_TYPE_SLIDER,
		.Slider =
		(UISlider)
		{
			.Axis = UI_AXIS_HORIZONTAL,
			.Sections = 256,
			.value = 0,

			.type = SliderType
		}
	};

	UIInitialize(s);

	IHandle v = ICreateText(
		&((IElement)
		{
			.w = 30.0f,

			.BorderBottom = 2.0f,

			.Opacity = 0xFF,
			.BorderColor = (color_argb_t){ 0xFFFFFFFF },

			.AlignX = UI_ALIGN_RIGHT,
			.AlignY = UI_ALIGN_MIDDLE,

			.Callback = ColorPickerTextCallback[SliderType]
		}),
		&((IContainer)
		{
			.AutoH = true
		}),
		&((IText)
		{
			.Placeholder = ShortName,
			.max_len = 3,

			.FontSize = 18,

			.Selectable = true,
			.Editable = true,

			.Stroke = (color_argb_t){ 0xFF000000 },
			.InverseStroke = (color_argb_t){ 0xFFFFFFFF },
			.Fill = (color_argb_t){ 0xFFFFFFFF },
			.InverseFill = (color_argb_t){ 0xFF000000 },
			.Background = (color_argb_t){ 0xA0000000 },

			.type = I_TEXT_TYPE_INTEGER,
			.data =
			(ITextData)
			{
				.Integer =
				(ITextInteger)
				{
					.min = 0,
					.max = 255
				}
			}
		})
	);

	IAddElementLast(&T, &C);
	IAddElementLast(&v, &C);
	UIAddElementLast(s, C.Destination);

	IAddElementLast(&C, ContentContainer);

	return (IColorPickerLine) { .Slider = &s->Slider, .value = v };
}


void
IColorPickerCreate(
	void
	)
{
	IHandle ParentContainer = ICreateContainer(
		&((IElement)
		{
			.MarginLeft = 10.0f,

			.BorderTop = 6.0f,
			.BorderLeft = 6.0f,
			.BorderRight = 6.0f,
			.BorderBottom = 6.0f,

			.Opacity = 0xFF,
			.BorderColor = (color_argb_t){ 0xFF000000 }
		}),
		&((IContainer)
		{
			.AutoW = true,
			.AutoH = true,

			.white_color = (color_argb_t){ 0xFFFFFFFF },
			.tex = TEXTURE_RECT128_T
		})
	);

	IHandle ColorContainer = ICreateContainer(
		&((IElement)
		{
			.Opacity = 0xFF
		}),
		&((IContainer)
		{
			.AutoW = true,
			.AutoH = true,

			.tex = TEXTURE_RECT
		})
	);

	IHandle ContentContainer = ICreateContainer(
		&((IElement)
		{
			.BorderTop = 6.0f,
			.BorderLeft = 6.0f,
			.BorderRight = 6.0f,
			.BorderBottom = 6.0f,

			.MarginTop = 64.0f,
			.MarginLeft = 64.0f,
			.MarginRight = 64.0f,
			.MarginBottom = 64.0f,

			.Opacity = 0xFF,
			.BorderColor = (color_argb_t){ 0xA0000000 },

			.AlignX = UI_ALIGN_CENTER,
			.AlignY = UI_ALIGN_MIDDLE
		}),
		&((IContainer)
		{
			.Axis = UI_AXIS_VERTICAL,

			.AutoW = true,
			.AutoH = true,

			.white_color = (color_argb_t){ 0xA0000000 },
			.tex = TEXTURE_RECT
		})
	);

	UIElement* ColorPicker = UIGetElement();
	*ColorPicker =
	(UIElement)
	{
		.w = 420.0f,
		.h = 420.0f,

		.Opacity = 0xFF,

		.AlignX = UI_ALIGN_CENTER,
		.AlignY = UI_ALIGN_TOP,

		.Callback = IColorPickerCallback,

		.type = UI_TYPE_COLOR_PICKER
	};

	UIInitialize(ColorPicker);

	IHandle HexContainer = ICreateContainer(
		&((IElement)
		{
			// .w = 100.0f,

			.MarginTop = 4.0f,
			.MarginLeft = 4.0f,
			.MarginRight = 4.0f,
			.MarginBottom = 4.0f,

			.BorderTop = 6.0f,
			.BorderLeft = 6.0f,
			.BorderRight = 6.0f,
			.BorderBottom = 6.0f,

			.Opacity = 0xFF,
			.BorderColor = (color_argb_t){ 0x20FFFFFF },

			.AlignX = UI_ALIGN_CENTER,
			.AlignY = UI_ALIGN_TOP
		}),
		&((IContainer)
		{
			.Axis = UI_AXIS_VERTICAL,

			.AutoW = true,
			.AutoH = true,

			.white_color = (color_argb_t){ 0x20FFFFFF },
			.tex = TEXTURE_RECT
		})
	);

	IHandle HexText = ICreateText(
		&((IElement)
		{
			.w = 108.0f,

			.BorderBottom = 2.0f,

			.Opacity = 0xFF,
			.BorderColor = (color_argb_t){ 0xFFFFFFFF },

			.AlignX = UI_ALIGN_CENTER,
			.AlignY = UI_ALIGN_TOP,

			.Callback = IHexTextCallback
		}),
		&((IContainer)
		{
			.AutoH = true
		}),
		&((IText)
		{
			.Placeholder = "#RRGGBBAA",
			.max_len = 9,

			.FontSize = 18,

			.Selectable = true,
			.Editable = true,

			.Stroke = (color_argb_t){ 0xFF000000 },
			.InverseStroke = (color_argb_t){ 0xFFFFFFFF },
			.Fill = (color_argb_t){ 0xFFFFFFFF },
			.InverseFill = (color_argb_t){ 0xFF000000 },
			.Background = (color_argb_t){ 0xA0000000 },

			.type = I_TEXT_TYPE_HEX_COLOR
		})
	);

	IAddElementLast(&ColorContainer, &ParentContainer);
	IAddElementLast(&ContentContainer, &ColorContainer);
	UIAddElementLast(ColorPicker, ContentContainer.Destination);
	IAddElementLast(&HexContainer, &ContentContainer);
	IAddElementLast(&HexText, &HexContainer);

	ColorPickerContainer	= ParentContainer.source;
	ColorPickerBackground	= ColorContainer.source;
	ColorPickerElement		= ColorPicker;
	ColorPickerHexText		= HexText;

	ColorPickerBrightness	= IColorPickerCreateLine("Brightness", "v", UI_SLIDER_TYPE_BRIGHTNESS, &ContentContainer);
	ColorPickerOpacity		= IColorPickerCreateLine("Opacity", "a", UI_SLIDER_TYPE_OPACITY, &ContentContainer);
	ColorPickerRed			= IColorPickerCreateLine("Red", "r", UI_SLIDER_TYPE_RED, &ContentContainer);
	ColorPickerGreen		= IColorPickerCreateLine("Green", "g", UI_SLIDER_TYPE_GREEN, &ContentContainer);
	ColorPickerBlue			= IColorPickerCreateLine("Blue", "b", UI_SLIDER_TYPE_BLUE, &ContentContainer);
}


void
IColorContainerCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	if(Event != UI_EVENT_MOUSE_UP)
	{
		return;
	}

	if(!SameElement)
	{
		return;
	}

	if(UIIsLinked(ColorPickerContainer))
	{
		UIUnlinkElement(ColorPickerContainer);
	}

	if(CurrentColorContainer != Element)
	{
		CurrentColorContainer = Element;
		IColorPickerUpdate(Element->Container.white_color);
		UIAddElementAfter(ColorPickerContainer, Element->Parent);
	}
	else
	{
		CurrentColorContainer = NULL;
	}
}


IHandle
ICreateColorPicker(
	const IElement* Element,
	const IColorPicker* ColorPicker
	)
{
	if(ColorPickerElement == NULL)
	{
		IColorPickerCreate();
	}

	IHandle ParentContainer = ICreateContainer(
		&((IElement)
		{
			.MarginTop = Element->MarginTop,
			.MarginLeft = Element->MarginLeft,
			.MarginRight = Element->MarginRight,
			.MarginBottom = Element->MarginBottom,

			.Opacity = Element->Opacity,

			.AlignX = Element->AlignX,
			.AlignY = Element->AlignY,
			.pos = Element->pos,
			.RelativeAlignX = Element->RelativeAlignX,
			.RelativeAlignY = Element->RelativeAlignY
		}),
		&((IContainer)
		{
			.Axis = UI_AXIS_HORIZONTAL,

			.AutoW = true,
			.AutoH = true
		})
	);

	IHandle TransparencyContainer = ICreateContainer(
		&((IElement)
		{
			.BorderTop = 6.0f,
			.BorderLeft = 6.0f,
			.BorderRight = 6.0f,
			.BorderBottom = 6.0f,

			.Opacity = Element->Opacity,
			.BorderColor = (color_argb_t){ 0xFF000000 },

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_MIDDLE
		}),
		&((IContainer)
		{
			.AutoW = true,
			.AutoH = true,

			.white_color = (color_argb_t){ 0xFFFFFFFF },
			.tex = TEXTURE_RECT8_T
		})
	);

	IHandle ColorContainer = ICreateContainer(
		&((IElement)
		{
			.w = 32.0f,
			.h = 32.0f,

			.Opacity = 0xFF,

			.Callback = IColorContainerCallback
		}),
		&((IContainer)
		{
			.Clickable = true,

			.white_color = ColorPicker->color,
			.tex = TEXTURE_RECT
		})
	);

	IAddElementLast(&TransparencyContainer, &ParentContainer);
	IAddElementLast(&ColorContainer, &TransparencyContainer);

	return (IHandle){ .source = ParentContainer.source, .Destination = NULL };
}


void
IDropdownOptionCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	assert_eq(Element->type, UI_TYPE_CONTAINER);
	UIContainer* Container = &Element->Container;


	switch(Event)
	{

	case UI_EVENT_MOUSE_IN:
	{
		Container->white_color = (color_argb_t){ 0xFF717171 };
		break;
	}

	case UI_EVENT_MOUSE_OUT:
	{
		Container->white_color = (color_argb_t){ 0xFFAAAAAA };
		break;
	}

	case UI_EVENT_MOUSE_UP:
	{
		if(!SameElement)
		{
			break;
		}

		UIElement* Chosen = CurrentDropdown->Container.head;
		assert_eq(Chosen->type, UI_TYPE_TEXT);

		UIText* OwnText = &Container->head->Text;
		UIText* Text = &Chosen->Text;

		Text->Str = realloc(Text->Str, OwnText->len + 1);
		assert_not_null(Text->Str);

		memcpy(Text->Str, OwnText->Str, OwnText->len + 1);
		Text->len = OwnText->len;

		UIUpdateElement(Chosen);

		break;
	}

	default: break;

	}
}


IHandle
ICreateDropdownOption(
	const IElement* Element,
	const IDropdown* Dropdown,
	const IDropdownOption* Option
	)
{
	IHandle Container = ICreateContainer(
		&((IElement)
		{
			.w = Element->w,

			.BorderTop = 2.0f,

			.Opacity = 0xFF,
			.BorderColor = (color_argb_t){ 0xFF000000 },

			.Callback = IDropdownOptionCallback
		}),
		&((IContainer)
		{
			.AutoH = true,

			.white_color = Dropdown->BgColor,
			.tex = TEXTURE_RECT
		})
	);

	IHandle Text = ICreateText(
		&((IElement)
		{
			.w = Element->w - 4.0f,

			.MarginTop = 4.0f,
			.MarginLeft = 4.0f,
			.MarginBottom = 4.0f,

			.Opacity = 0xFF,

			.AlignY = UI_ALIGN_MIDDLE
		}),
		NULL,
		&((IText)
		{
			.Str = Option->Text,

			.FontSize = Dropdown->FontSize,
			.AlignX = UI_ALIGN_LEFT,

			.Stroke = Dropdown->Stroke,
			.Fill = Dropdown->Fill
		})
	);

	IAddElementLast(&Text, &Container);

	return (IHandle){ .source = Container.source, .Destination = NULL };
}


void
IDropdownCallback(
	UIElement* Element,
	UIEvent Event
	)
{
	assert_eq(Element->type, UI_TYPE_CONTAINER);
	UIContainer* Container = &Element->Container;


	switch(Event)
	{

	case UI_EVENT_MOUSE_IN:
	{
		Container->white_color = (color_argb_t){ 0xFF717171 };
		break;
	}

	case UI_EVENT_MOUSE_OUT:
	{
		Container->white_color = (color_argb_t){ 0xFFAAAAAA };
		break;
	}

	case UI_EVENT_MOUSE_UP:
	{
		if(!SameElement)
		{
			break;
		}

		UIElement* Dropdown = CurrentDropdown;
		if(CurrentDropdown)
		{
			UIUnlinkElement(CurrentDropdown->data);
			CurrentDropdown = NULL;

			Dropdown->Container.tail->tex.tex = TEXTURE_TRIANGLE_DOWN;
		}

		if(Dropdown != Element)
		{
			CurrentDropdown = Element;
			UIAddElementAfter(Element->data, Element);

			CurrentDropdown->Container.tail->tex.tex = TEXTURE_TRIANGLE_UP;
		}

		break;
	}

	default: break;

	}
}


IHandle
ICreateDropdown(
	const IElement* Element,
	const IDropdown* Dropdown
	)
{
	const IDropdownOption* Option = Dropdown->Options + Dropdown->Chosen;

	IHandle ParentContainer = ICreateContainer(
		&((IElement)
		{
			.MarginTop = Element->MarginTop,
			.MarginLeft = Element->MarginLeft,
			.MarginRight = Element->MarginRight,
			.MarginBottom = Element->MarginBottom,

			.Opacity = Element->Opacity,

			.AlignX = Element->AlignX,
			.AlignY = Element->AlignY,
			.pos = Element->pos,
			.RelativeAlignX = Element->RelativeAlignX,
			.RelativeAlignY = Element->RelativeAlignY,

			.Relative = Element->Relative
		}),
		&((IContainer)
		{
			.Axis = UI_AXIS_VERTICAL,

			.AutoW = true,
			.AutoH = true
		})
	);

	IHandle Container = ICreateContainer(
		&((IElement)
		{
			.w = Element->w,

			.Opacity = 0xFF,

			.Callback = IDropdownCallback
		}),
		&((IContainer)
		{
			.Axis = UI_AXIS_HORIZONTAL,

			.AutoH = true,

			.white_color = Dropdown->BgColor,
			.tex = TEXTURE_RECT
		})
	);

	IHandle Text = ICreateText(
		&((IElement)
		{
			.w = Element->w - 32.0f,

			.MarginTop = 4.0f,
			.MarginLeft = 4.0f,
			.MarginBottom = 4.0f,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_MIDDLE
		}),
		NULL,
		&((IText)
		{
			.Str = Option->Text,

			.FontSize = Dropdown->FontSize,
			.AlignX = UI_ALIGN_LEFT,

			.Stroke = Dropdown->Stroke,
			.Fill = Dropdown->Fill
		})
	);

	IHandle tex = ICreateTexture(
		&((IElement)
		{
			.w = 16.0f,
			.h = 16.0f,

			.MarginRight = 6.0f,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_RIGHT,
			.AlignY = UI_ALIGN_MIDDLE
		}),
		&((ITexture)
		{
			.white_color = (color_argb_t){ 0xFF000000 },
			.tex = TEXTURE_TRIANGLE_DOWN
		})
	);

	IAddElementLast(&Text, &Container);
	IAddElementLast(&tex, &Container);
	IAddElementLast(&Container, &ParentContainer);

	IHandle DummyContainer = ICreateContainer(
		&((IElement)
		{
			.Opacity = 0xFF
		}),
		&((IContainer)
		{
			.AutoW = true,
			.AutoH = true
		})
	);

	for(uint32_t i = 0; i < Dropdown->count; ++i)
	{
		IHandle handle = ICreateDropdownOption(Element, Dropdown, Dropdown->Options + i);
		IAddElementLast(&handle, &DummyContainer);
	}

	Container.source->data = DummyContainer.source;

	return (IHandle){ .source = ParentContainer.source, .Destination = NULL };
}


IHandle
ICreateTexture(
	const IElement* Element,
	const ITexture* tex
	)
{
	UITexture Tex;

	if(tex->UseExplicit)
	{
		Tex =
		(UITexture)
		{
			.SW = tex->SW,
			.SH = tex->SH,
			.OX = tex->OX,
			.OY = tex->OY,

			.white_color = tex->white_color,
			.black_color = tex->black_color,
			.tex = tex->tex,
			.angle = tex->angle
		};
	}
	else
	{
		Tex =
		(UITexture)
		{
			.SW = 1.0f,
			.SH = 1.0f,
			.OX = 0.5f,
			.OY = 0.5f,

			.white_color = tex->white_color,
			.black_color = tex->black_color,
			.tex = tex->tex,
			.angle = tex->angle
		};
	}

	UIElement* T = UIGetElement();
	*T =
	(UIElement)
	{
		.x = Element->x,
		.y = Element->y,
		.w = Element->w,
		.h = Element->h,

		.MarginTop = Element->MarginTop,
		.MarginLeft = Element->MarginLeft,
		.MarginRight = Element->MarginRight,
		.MarginBottom = Element->MarginBottom,

		.BorderTop = Element->BorderTop,
		.BorderLeft = Element->BorderLeft,
		.BorderRight = Element->BorderRight,
		.BorderBottom = Element->BorderBottom,

		.Opacity = Element->Opacity,
		.BorderColor = Element->BorderColor,

		.AlignX = Element->AlignX,
		.AlignY = Element->AlignY,
		.pos = Element->pos,
		.RelativeAlignX = Element->RelativeAlignX,
		.RelativeAlignY = Element->RelativeAlignY,

		.InteractiveBorder = Element->InteractiveBorder,

		.Relative = Element->Relative ? Element->Relative->source : NULL,

		.Callback = Element->Callback,
		.data = Element->data,

		.type = UI_TYPE_TEXTURE,
		.tex = Tex
	};

	UIInitialize(T);

	return (IHandle){ .source = T, .Destination = NULL };
}
