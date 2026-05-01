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

#include <shared/color.h>
#include <shared/debug.h>
#include <shared/macro.h>
#include <shared/extent.h>

#include <math.h>


color_hsv_t
color_argb_to_hsv(
	color_argb_t argb
	)
{
	float r = argb.r / 255.0f;
	float g = argb.g / 255.0f;
	float b = argb.b / 255.0f;

	float max = MACRO_MAX(MACRO_MAX(r, g), b);
	float min = MACRO_MIN(MACRO_MIN(r, g), b);
	float delta = max - min;

	color_hsv_t hsv;

	if(delta == 0.0f)
	{
		hsv.h = 0.0f;
	}
	else if(max == r)
	{
		hsv.h = (g - b) / delta;

		if(hsv.h < 0.0f)
		{
			hsv.h += 6.0f;
		}
	}
	else if(max == g)
	{
		hsv.h = (b - r) / delta + 2;
	}
	else
	{
		hsv.h = (r - g) / delta + 4;
	}

	hsv.h /= 6.0f;
	assert_le(hsv.h, 1.0f);

	if(max == 0.0f)
	{
		hsv.s = 0.0f;
	}
	else
	{
		hsv.s = delta / max;
	}

	assert_le(hsv.s, 1.0f);

	hsv.v = max;

	return hsv;
}


color_argb_t
color_hsv_to_argb(
	color_hsv_t hsv
	)
{
	float    v			= hsv.v * 255.0f;
	float    s			= hsv.s * v;
	uint32_t section	= hsv.h * 6.0f;
	float    remainder	= hsv.h * 6.0f - section;
	uint8_t	 zero		= nearbyintf(v - s                     );
	uint8_t	 dropping	= nearbyintf(v - s *         remainder );
	uint8_t	 rising		= nearbyintf(v - s * (1.0f - remainder));

	switch(section)
	{

	case 0: return (color_argb_t){ .b = zero	, .g = rising	, .r = v		, .a = 0xFF };
	case 1: return (color_argb_t){ .b = zero	, .g = v		, .r = dropping	, .a = 0xFF };
	case 2: return (color_argb_t){ .b = rising	, .g = v		, .r = zero		, .a = 0xFF };
	case 3: return (color_argb_t){ .b = v		, .g = dropping	, .r = zero		, .a = 0xFF };
	case 4: return (color_argb_t){ .b = v		, .g = zero		, .r = rising	, .a = 0xFF };
	case 5: return (color_argb_t){ .b = dropping, .g = zero		, .r = v		, .a = 0xFF };
	case 6: return (color_argb_t){ .b = zero	, .g = zero		, .r = v		, .a = 0xFF };

	default: assert_unreachable();

	}
}


pair_t
color_hsv_to_pos(
	color_hsv_t hsv
	)
{
	float distance = hsv.s * 1.0f * 0.5f;
	float angle = hsv.h * 2.0f * M_PI;

	pair_t pos =
	{
		.x = -distance * cosf(angle),
		.y = -distance * sinf(angle)
	};

	return pos;
}


color_hsv_t
color_pos_to_hsv(
	pair_t pos
	)
{
	float h = (atan2f(pos.y, pos.x) + M_PI) / (M_PI * 2.0f);
	assert_le(h, 1.0f);

	float s = sqrtf(pos.x * pos.x + pos.y * pos.y) * 2.0f;
	s = MACRO_MIN(s, 1.0f);
	assert_le(s, 1.0f);

	return (color_hsv_t){ .h = h, .s = s, .v = 1.0f };
}


uint8_t
color_a_mul_a(
	uint8_t a_1,
	uint8_t a_2
	)
{
	return ((uint32_t) a_1 * (uint32_t) a_2 + 255) >> 8;
}


color_argb_t
color_argb_mul_a(
	color_argb_t color,
	uint8_t a
	)
{
	color.a = color_a_mul_a(a, color.a);
	return color;
}
