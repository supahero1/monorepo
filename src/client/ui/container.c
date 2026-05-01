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
#include <client/ui/container.h>


void
UIContainerBubbleDown(
	UIElement* Element,
	UIBubbleCallback Callback,
	void* data
	)
{
	UIContainer* Container = Element->TypeData;
	UIElement* Child = Container->head;

	while(Child)
	{
		/* Child might be free'd during the bubble */
		UIElement* next = Child->next;
		Child->VirtualTable->BubbleDown(Child, Callback, data);
		Child = next;
	}

	Callback(Element, data);
}


void
UIContainerPropagateSize(
	UIElement* Element,
	UIElement* Child,
	pair_t delta
	)
{
	assert_true(UIElementIsContainer(Element));
	UIContainer* Container = Element->TypeData;
	assert_not_null(Container->head);
	assert_not_null(Container->tail);

	bool Pass = false;

	pair_t old_size = Element->Extent.size;

	if(Container->AutoW)
	{
		if(Container->Axis == UI_AXIS_HORIZONTAL)
		{
			if(!Child->Inline)
			{
				Container->ContentW += delta.w;
			}
			else /* Inline */
			{
				if(delta.w >= 0.0f)
				{
					Container->MaxInlineW = MACRO_MAX(Container->MaxInlineW, Child->EndExtent.w);
				}
				else
				{
					float TotalW = 0.0f;
					UIElement* Child = Container->head;

					while(Child)
					{
						if(Child->Inline)
						{
							TotalW = MACRO_MAX(TotalW, Child->EndExtent.w);
						}
						Child = Child->next;
					}

					Container->MaxInlineW = TotalW;
				}
			}
		}
		else /* UI_AXIS_VERTICAL */
		{
			if(!Child->Inline)
			{
				if(delta.w >= 0.0f)
				{
					Container->ContentW = MACRO_MAX(Container->ContentW, Child->EndExtent.w);
				}
				else
				{
					float TotalW = 0.0f;
					UIElement* Child = Container->head;

					while(Child)
					{
						if(!Child->Inline)
						{
							TotalW = MACRO_MAX(TotalW, Child->EndExtent.w);
						}
						Child = Child->next;
					}

					Container->ContentW = TotalW;
				}
			}
			else /* Inline */
			{
				if(delta.w >= 0.0f)
				{
					Container->MaxInlineW = MACRO_MAX(Container->MaxInlineW, Child->EndExtent.w);
				}
				else
				{
					float TotalW = 0.0f;
					UIElement* Child = Container->head;

					while(Child)
					{
						if(Child->Inline)
						{
							TotalW = MACRO_MAX(TotalW, Child->EndExtent.w);
						}
						Child = Child->next;
					}

					Container->MaxInlineW = TotalW;
				}
			}
		}

		Element->Extent.w = MACRO_MAX(Container->ContentW, Container->MaxInlineW);

		delta.w = Element->Extent.w - old_size.w;
		if(delta.w)
		{
			Pass = true;
			UIUpdateWidth(Element);
		}
	}
	else
	{
		delta.w = 0.0f;
	}


	if(Container->AutoH)
	{
		/* Analogy to x above */

		if(Container->Axis == UI_AXIS_VERTICAL)
		{
			if(!Child->Inline)
			{
				Container->ContentH += delta.h;
			}
			else /* Inline */
			{
				if(delta.h >= 0.0f)
				{
					Container->MaxInlineH = MACRO_MAX(Container->MaxInlineH, Child->EndExtent.h);
				}
				else
				{
					float TotalH = 0.0f;
					UIElement* Child = Container->head;

					while(Child)
					{
						if(Child->Inline)
						{
							TotalH = MACRO_MAX(TotalH, Child->EndExtent.h);
						}
						Child = Child->next;
					}

					Container->MaxInlineH = TotalH;
				}
			}
		}
		else /* UI_AXIS_HORIZONTAL */
		{
			if(!Child->Inline)
			{
				if(delta.h >= 0.0f)
				{
					Container->ContentH = MACRO_MAX(Container->ContentH, Child->EndExtent.h);
				}
				else
				{
					float TotalH = 0.0f;
					UIElement* Child = Container->head;

					while(Child)
					{
						if(!Child->Inline)
						{
							TotalH = MACRO_MAX(TotalH, Child->EndExtent.h);
						}
						Child = Child->next;
					}

					Container->ContentH = TotalH;
				}
			}
			else /* Inline */
			{
				if(delta.h >= 0.0f)
				{
					Container->MaxInlineH = MACRO_MAX(Container->MaxInlineH, Child->EndExtent.h);
				}
				else
				{
					float TotalH = 0.0f;
					UIElement* Child = Container->head;

					while(Child)
					{
						if(Child->Inline)
						{
							TotalH = MACRO_MAX(TotalH, Child->EndExtent.h);
						}
						Child = Child->next;
					}

					Container->MaxInlineH = TotalH;
				}
			}
		}

		Element->Extent.h = MACRO_MAX(Container->ContentH, Container->MaxInlineH);

		delta.h = Element->Extent.h - old_size.h;
		if(delta.h)
		{
			Pass = true;
			UIUpdateHeight(Element);
		}
	}
	else
	{
		delta.h = 0.0f;
	}


	if(!Pass)
	{
		return;
	}

	UIResizeData event_data =
	(UIResizeData)
	{
		.old_size = old_size,
		.new_size = Element->Extent.size
	};

	event_target_fire(&Element->ResizeTarget, &event_data);


	if(Element->Parent)
	{
		Element->VirtualTable->PropagateSize(
			Element->Parent,
			Element,
			delta
			);
	}
}


void
UIContainerPreClip(
	UIElement* Element,
	UIElement* Scrollable
	)
{
	UIContainer* Container = Element->TypeData;

	if(Element->Scrollable)
	{
		UIElement* Parent = Element->Parent;

		if(Container->Axis == UI_AXIS_VERTICAL)
		{
			float ClampedGoalOffsetY = MACRO_CLAMP(Container->GoalOffsetY, Parent->Extent.h - Element->Extent.h, 0);
			Container->GoalOffsetY = glm_lerpc(Container->GoalOffsetY, ClampedGoalOffsetY, 0.5f * DeltaTime);
			Container->offset_y     = glm_lerpc(Container->offset_y, Container->GoalOffsetY, 0.2f * DeltaTime);
		}
		else
		{
			float ClampedGoalOffsetX = MACRO_CLAMP(Container->GoalOffsetX, Parent->Extent.w - Element->Extent.w, 0);
			Container->GoalOffsetX = glm_lerpc(Container->GoalOffsetX, ClampedGoalOffsetX, 0.5f * DeltaTime);
			Container->OffsetX     = glm_lerpc(Container->OffsetX, Container->GoalOffsetX, 0.2f * DeltaTime);
		}

		Element->EndExtent.x += Container->OffsetX;
		Element->EndExtent.y += Container->offset_y;
	}
}


void
UIContainerPostClip(
	UIElement* Element,
	rect_extent_t Clip,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{

}


bool
UIContainerMouseOver(
	UIElement* Element
	)
{
	return UIMouseOverElement(Element);
}


void
UIContainerDrawDetail(
	UIElement* Element,
	rect_extent_t Clip,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UIContainer* Container = Element->TypeData;

	UIClipTexture(Element->EndExtent.x, Element->EndExtent.y,
		Element->Extent.w, Element->Extent.h,
		.white_color = color_argb_mul_a(Container->white_color, Opacity),
		.white_depth = depth,
		.black_color = color_argb_mul_a(Container->black_color, Opacity),
		.black_depth = depth,
		.tex = Container->tex
		);
}


void
UIInheritPosition(
	UIElement* Element,
	rect_extent_t Clip
	)
{
	if(!Element->Parent)
	{
		Element->EndExtent.Pos = Element->Extent.Pos;
		return;
	}

	UIElement* Parent = Element->Parent;
	assert_true(UIElementIsContainer(Parent));
	UIContainer* Container = Parent->TypeData;


	switch(Element->pos)
	{

	case UI_POSITION_INHERIT:
	{
		float OffsetMinX;
		float OffsetMaxX;
		float OffsetMinY;
		float OffsetMaxY;

		if(Container->Axis == UI_AXIS_VERTICAL)
		{
			OffsetMinX = 0.0f;
			OffsetMaxX = 0.0f;
			OffsetMinY = Container->OffsetMin;
			OffsetMaxY = Container->OffsetMax;
		}
		else
		{
			OffsetMinX = Container->OffsetMin;
			OffsetMaxX = Container->OffsetMax;
			OffsetMinY = 0.0f;
			OffsetMaxY = 0.0f;
		}


		switch(Element->AlignX)
		{

		case UI_ALIGN_LEFT:
		{
			Element->EndExtent.x = Parent->EndExtent.x - Parent->Extent.w
				+ Element->Extent.w + Element->DrawMargin.left * 2.0f + OffsetMinX;
			OffsetMinX += Element->EndExtent.w * 2.0f;
			break;
		}

		case UI_ALIGN_CENTER:
		{
			Element->EndExtent.x = Parent->EndExtent.x;
			break;
		}

		case UI_ALIGN_RIGHT:
		{
			Element->EndExtent.x = Parent->EndExtent.x + Parent->Extent.w
				-(Element->Extent.w + Element->DrawMargin.right * 2.0f + OffsetMaxX);
			OffsetMaxX += Element->EndExtent.w * 2.0f;
			break;
		}

		}


		switch(Element->AlignY)
		{

		case UI_ALIGN_TOP:
		{
			Element->EndExtent.y = Parent->EndExtent.y - Parent->Extent.h
				+ Element->Extent.h + Element->DrawMargin.top * 2.0f + OffsetMinY;
			OffsetMinY += Element->EndExtent.h * 2.0f;
			break;
		}

		case UI_ALIGN_MIDDLE:
		{
			Element->EndExtent.y = Parent->EndExtent.y;
			break;
		}

		case UI_ALIGN_BOTTOM:
		{
			Element->EndExtent.y = Parent->EndExtent.y + Parent->Extent.h
				-(Element->Extent.h + Element->DrawMargin.bottom * 2.0f + OffsetMaxY);
			OffsetMaxY += Element->EndExtent.h * 2.0f;
			break;
		}

		}


		if(!Element->Inline)
		{
			if(Container->Axis == UI_AXIS_VERTICAL)
			{
				Container->OffsetMin = OffsetMinY;
				Container->OffsetMax = OffsetMaxY;
			}
			else
			{
				Container->OffsetMin = OffsetMinX;
				Container->OffsetMax = OffsetMaxX;
			}
		}

		break;
	}

	case UI_POSITION_ABSOLUTE:
	{
		Element->EndExtent.x = Parent->EndExtent.x + Element->Extent.x;
		Element->EndExtent.y = Parent->EndExtent.y + Element->Extent.y;
		break;
	}

	case UI_POSITION_RELATIVE:
	{
		assert_not_null(Element->Relative);
		UIElement* Relative = Element->Relative;


		Element->EndExtent.x = Relative->EndExtent.x;

		switch(Element->AlignX)
		{

		case UI_ALIGN_LEFT:
		{
			Element->EndExtent.x -= Element->Extent.w + Element->DrawMargin.right * 2.0f;
			break;
		}

		case UI_ALIGN_RIGHT:
		{
			Element->EndExtent.x += Element->Extent.w + Element->DrawMargin.left * 2.0f;
			break;
		}

		default: break;

		}

		switch(Element->RelativeAlignX)
		{

		case UI_ALIGN_LEFT:
		{
			Element->EndExtent.x -= Relative->Extent.w;
			break;
		}

		case UI_ALIGN_RIGHT:
		{
			Element->EndExtent.x += Relative->Extent.w;
			break;
		}

		default: break;

		}


		Element->EndExtent.y = Relative->EndExtent.y;

		switch(Element->AlignY)
		{

		case UI_ALIGN_TOP:
		{
			Element->EndExtent.y -= Element->Extent.h + Element->DrawMargin.bottom * 2.0f;
			break;
		}

		case UI_ALIGN_BOTTOM:
		{
			Element->EndExtent.y += Element->Extent.h + Element->DrawMargin.top * 2.0f;
			break;
		}

		default: break;

		}

		switch(Element->RelativeAlignY)
		{

		case UI_ALIGN_TOP:
		{
			Element->EndExtent.y -= Relative->Extent.h;
			break;
		}

		case UI_ALIGN_BOTTOM:
		{
			Element->EndExtent.y += Relative->Extent.h;
			break;
		}

		default: break;

		}


		break;
	}

	default: assert_unreachable();

	}
}


void
UIContainerDrawChildren(
	UIElement* Element,
	rect_extent_t Clip,
	uint8_t Opacity,
	UIElement* Scrollable
	)
{
	UIContainer* Container = Element->TypeData;

	Container->OffsetMin = 0.0f;
	Container->OffsetMax = 0.0f;

	if(Element->Scrollable)
	{
		Scrollable = Element;
	}

	UIElement* Child = Container->head;
	while(Child)
	{
		uint8_t ChildOpacity = color_a_mul_a(Opacity, Child->Opacity);
		if(ChildOpacity)
		{
			depth += DRAW_DEPTH_LEAP;

			UIDrawElement(
				Child,
				Clip,
				ChildOpacity,
				Scrollable
				);
		}

		Child = Child->next;
	}
}


UIVirtualTable ContainerVirtualTable =
{
	.BubbleDown = UIContainerBubbleDown,
	.PropagateSize = UIContainerPropagateSize,
	.PreClip = UIContainerPreClip,
	.PostClip = UIContainerPostClip,
	.MouseOver = UIContainerMouseOver,
	.DrawDetail = UIContainerDrawDetail,
	.DrawChildren = UIContainerDrawChildren,
};


bool
UIElementIsContainer(
	UIElement* Element
	)
{
	return Element->VirtualTable == &ContainerVirtualTable;
}


UIContainer*
UIAllocContainerData(
	void
	)
{
	void* Container = alloc_malloc(sizeof(UIContainer));
	assert_not_null(Container);

	return Container;
}


void
UIFreeContainerData(
	UIContainer* Container,
	void* _
	)
{
	assert_not_null(Container);
	alloc_free(sizeof(UIContainer), Container);
}


UIElement*
UIAllocContainer(
	UIElementInfo ElementInfo,
	UIContainerInfo info
	)
{
	UIElement* Element = UIAllocElement(ElementInfo);
	assert_not_null(Element);

	UIContainer* Container = UIAllocContainerData();
	*Container =
	(UIContainer)
	{
		.Axis = info.Axis,

		.AutoW = info.AutoW,
		.AutoH = info.AutoH,

		.white_color = info.white_color,
		.black_color = info.black_color,
		.tex = info.tex
	};

	Element->VirtualTable = &ContainerVirtualTable;
	Element->TypeData = Container;

	event_target_add(&Element->FreeTarget, (void*) UIFreeContainerData, Container);

	return Element;
}


void
UIPropagateInsertion(
	UIElement* Element,
	UIElement* Parent
	)
{
	Parent->VirtualTable->PropagateSize(
		Parent,
		Element,
		Element->EndExtent.size
		);
}


void
UIInsertFirst(
	UIElement* Element,
	UIElement* Parent
	)
{
	assert_true(UIElementIsContainer(Parent));
	UIContainer* Container = Parent->TypeData;

	Element->Parent = Parent;
	Element->prev = NULL;
	Element->next = Container->head;

	if(Container->head)
	{
		Container->head->prev = Element;
	}
	else
	{
		Container->tail = Element;
	}

	Container->head = Element;

	UIPropagateInsertion(Element, Parent);
}


void
UIInsertLast(
	UIElement* Element,
	UIElement* Parent
	)
{
	assert_true(UIElementIsContainer(Parent));
	UIContainer* Container = Parent->TypeData;

	Element->Parent = Parent;
	Element->prev = Container->tail;
	Element->next = NULL;

	if(Container->tail)
	{
		Container->tail->next = Element;
	}
	else
	{
		Container->head = Element;
	}

	Container->tail = Element;

	UIPropagateInsertion(Element, Parent);
}


void
UIInsertBefore(
	UIElement* Element,
	UIElement* Before
	)
{
	UIElement* Parent = Before->Parent;
	assert_true(UIElementIsContainer(Parent));
	UIContainer* Container = Parent->TypeData;

	Element->Parent = Parent;
	Element->prev = Before->prev;
	Element->next = Before;

	if(Before->prev)
	{
		Before->prev->next = Element;
	}
	else
	{
		Container->head = Element;
	}

	Before->prev = Element;

	UIPropagateInsertion(Element, Element->Parent);
}


void
UIInsertAfter(
	UIElement* Element,
	UIElement* After
	)
{
	UIElement* Parent = After->Parent;
	assert_true(UIElementIsContainer(Parent));
	UIContainer* Container = Parent->TypeData;

	Element->Parent = Parent;
	Element->prev = After;
	Element->next = After->next;

	if(After->next)
	{
		After->next->prev = Element;
	}
	else
	{
		Container->tail = Element;
	}

	After->next = Element;

	UIPropagateInsertion(Element, Element->Parent);
}


bool
UIIsLinked(
	UIElement* Element
	)
{
	return !!Element->Parent;
}


void
UIUnlink(
	UIElement* Element
	)
{
	if(Element == UIGetRootElement())
	{
		UISetRootElement(NULL);
	}

	UIElement* Parent = Element->Parent;
	assert_true(UIElementIsContainer(Parent));
	UIContainer* Container = Parent->TypeData;

	if(Element->prev)
	{
		Element->prev->next = Element->next;
	}
	else
	{
		Container->head = Element->next;
	}

	if(Element->next)
	{
		Element->next->prev = Element->prev;
	}
	else
	{
		Container->tail = Element->prev;
	}

	Element->Parent = NULL;
	Element->prev = NULL;
	Element->next = NULL;

	Parent->VirtualTable->PropagateSize(
		Parent,
		Element,
		(pair_t)
		{
			.w = -Element->EndExtent.w,
			.h = -Element->EndExtent.h
		}
		);
}
