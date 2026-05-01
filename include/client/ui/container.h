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

#include <client/ui/base.h>
#include <client/tex/base.h>


typedef struct UIContainer
{
	UIAxis Axis;

	bool AutoW;
	bool AutoH;

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


typedef struct UIContainerInfo
{
	UIAxis Axis;

	bool AutoW;
	bool AutoH;

	color_argb_t white_color;
	color_argb_t black_color;
	TexInfo tex;
}
UIContainerInfo;


extern bool
UIElementIsContainer(
	UIElement* Element
	);


extern UIElement*
UIAllocContainer(
	UIElementInfo ElementInfo,
	UIContainerInfo info
	);


extern void
UIInsertFirst(
	UIElement* Element,
	UIElement* Parent
	);


extern void
UIInsertLast(
	UIElement* Element,
	UIElement* Parent
	);


extern void
UIInsertBefore(
	UIElement* Element,
	UIElement* Before
	);


extern void
UIInsertAfter(
	UIElement* Element,
	UIElement* After
	);


extern bool
UIIsLinked(
	UIElement* Element
	);


extern void
UIUnlink(
	UIElement* Element
	);
