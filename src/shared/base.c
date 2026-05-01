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

#include <shared/base.h>


const float ShapeRadius[] =
{
	[SHAPE_SQUARE] = 50.0,
	[SHAPE_TRIANGLE] = 55.0,
	[SHAPE_PENTAGON] = 70.0
};


const float ShapeHitbox[] =
{
	[SHAPE_SQUARE] = 35.0,
	[SHAPE_TRIANGLE] = 25.0,
	[SHAPE_PENTAGON] = 55.0
};


typedef enum ShapeMaxHPValues
{
	SQUARE_MAX_HP = 10,
	TRIANGLE_MAX_HP = 20,
	PENTAGON_MAX_HP = 100
}
ShapeMaxHPValues;


const uint32_t ShapeMaxHP[] =
{
	[SHAPE_SQUARE] = SQUARE_MAX_HP,
	[SHAPE_TRIANGLE] = TRIANGLE_MAX_HP,
	[SHAPE_PENTAGON] = PENTAGON_MAX_HP
};


#define SHAPE_HP_BITS(ShapeName)	\
[ SHAPE_##ShapeName ] = MACRO_GET_BITS( ShapeName##_MAX_HP )

const uint32_t ShapeHPBits[] =
{
	SHAPE_HP_BITS(SQUARE),
	SHAPE_HP_BITS(TRIANGLE),
	SHAPE_HP_BITS(PENTAGON)
};

#undef SHAPE_HP_BITS


const uint32_t TypeToSubtypeBits[] =
{
	[ENTITY_TYPE_TANK] = TANK__BITS,
	[ENTITY_TYPE_SHAPE] = SHAPE__BITS
};
