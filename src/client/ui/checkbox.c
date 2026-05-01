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
#include <shared/alloc/base.h>
#include <client/ui/checkbox.h>
#include <client/ui/container.h>


void
UICheckboxBubbleDown(
	UIElement* Element,
	UIBubbleCallback Callback,
	void* data
	)
{
}


void
UICheckboxPropagateSize(
	UIElement* Element,
	UIElement* Child,
	pair_t delta
	)
{
}


void
UICheckboxPreClip(
	UIElement* Element,
	UIElement* Scrollable
	)
{

}


void
UICheckboxPostClip(
	UIElement* Element,
	rect_extent_t Clip,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UICheckbox* Checkbox = Element->TypeData;

	Element->BorderColor = Checkbox->Checked ? Checkbox->CheckYesBackground : Checkbox->CheckNoBackground;
}


bool
UICheckboxMouseOver(
	UIElement* Element
	)
{
	return UIMouseOverElement(Element);
}


void
UICheckboxDrawDetail(
	UIElement* Element,
	rect_extent_t Clip,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	assert_eq(Element->Extent.w, Element->Extent.h);

	UICheckbox* Checkbox = Element->TypeData;

	UIClipTexture(Element->EndExtent.x, Element->EndExtent.y, Element->Extent.w, Element->Extent.h,
		.white_color = color_argb_mul_a(Checkbox->Checked ? Checkbox->CheckYesBackground : Checkbox->CheckNoBackground, Opacity),
		.white_depth = depth,
		.tex = TEXTURE_RECT
		);

	float size = (Element->Extent.w * 0.66666f + Element->BorderRadius * 1.33333f) * 1.1f;

	UIClipTexture(Element->EndExtent.x, Element->EndExtent.y, size, size,
		.white_color = color_argb_mul_a(Checkbox->Checked ? Checkbox->CheckYes : Checkbox->CheckNo, Opacity),
		.white_depth = depth + DRAW_DEPTH_JIFFIE,
		.tex = Checkbox->Checked ? TEXTURE_CHECK_YES : TEXTURE_CHECK_NO
		);
}


void
UICheckboxDrawChildren(
	UIElement* Element,
	rect_extent_t Clip,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
}


UIVirtualTable CheckboxVirtualTable =
{
	.BubbleDown = UICheckboxBubbleDown,
	.PropagateSize = UICheckboxPropagateSize,
	.PreClip = UICheckboxPreClip,
	.PostClip = UICheckboxPostClip,
	.MouseOver = UICheckboxMouseOver,
	.DrawDetail = UICheckboxDrawDetail,
	.DrawChildren = UICheckboxDrawChildren
};


bool
UIElementIsCheckbox(
	UIElement* Element
	)
{
	return Element->VirtualTable == &CheckboxVirtualTable;
}


UICheckbox*
UIAllocCheckboxData(
	void
	)
{
	void* Checkbox = alloc_malloc(sizeof(UICheckbox));
	assert_not_null(Checkbox);

	return Checkbox;
}


void
UIFreeCheckboxData(
	UICheckbox* Checkbox,
	void* _
	)
{
	assert_not_null(Checkbox);
	alloc_free(sizeof(UICheckbox), Checkbox);
}


void
UICheckboxMouseUpCallback(
	UICheckbox* Checkbox,
	UIMouseUpData* data
	)
{
	if(!UISameElement)
	{
		return;
	}

	Checkbox->Checked = !Checkbox->Checked;
}


UIElement*
UIAllocCheckbox(
	UIElementInfo ElementInfo,
	UICheckboxInfo info
	)
{
	ElementInfo.Clickable = true;
	ElementInfo.InteractiveBorder = true;
	ElementInfo.ClipToBorder = true;

	UIElement* Element = UIAllocElement(ElementInfo);
	assert_not_null(Element);

	UICheckbox* Checkbox = UIAllocCheckboxData();
	*Checkbox =
	(UICheckbox)
	{
		.Checked = info.Checked,

		.CheckYes = info.CheckYes,
		.CheckYesBackground = info.CheckYesBackground,

		.CheckNo = info.CheckNo,
		.CheckNoBackground = info.CheckNoBackground
	};

	Element->VirtualTable = &CheckboxVirtualTable;
	Element->TypeData = Checkbox;

	event_target_add(&Element->MouseUpTarget, (void*) UICheckboxMouseUpCallback, Checkbox);
	event_target_add(&Element->FreeTarget, (void*) UIFreeCheckboxData, Checkbox);

	return Element;
}


bool
UICheckboxGetChecked(
	UIElement* Element
	)
{
	assert_true(UIElementIsCheckbox(Element));
	UICheckbox* Checkbox = Element->TypeData;

	return Checkbox->Checked;
}


void
UICheckboxSetChecked(
	UIElement* Element,
	bool Checked
	)
{
	assert_true(UIElementIsCheckbox(Element));
	UICheckbox* Checkbox = Element->TypeData;

	Checkbox->Checked = Checked;
}
