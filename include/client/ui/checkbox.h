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


typedef struct UICheckbox
{
	bool Checked;

	color_argb_t CheckYes;
	color_argb_t CheckYesBackground;

	color_argb_t CheckNo;
	color_argb_t CheckNoBackground;
}
UICheckbox;


typedef struct UICheckboxInfo
{
	bool Checked;

	color_argb_t CheckYes;
	color_argb_t CheckYesBackground;

	color_argb_t CheckNo;
	color_argb_t CheckNoBackground;
}
UICheckboxInfo;


extern bool
UIElementIsCheckbox(
	UIElement* Element
	);


extern UIElement*
UIAllocCheckbox(
	UIElementInfo ElementInfo,
	UICheckboxInfo info
	);


extern bool
UICheckboxGetChecked(
	UIElement* Element
	);


extern void
UICheckboxSetChecked(
	UIElement* Element,
	bool Checked
	);
