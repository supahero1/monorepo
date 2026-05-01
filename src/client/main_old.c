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

// #include <socket.h>

#include <shared/base.h>
#include <shared/rand.h>
#include <shared/time.h>
#include <shared/debug.h>
#include <shared/base64.h>
#include <shared/threads.h>
#include <shared/alloc/base.h>
#include <shared/bit_buffer.h>
#include <client/window/base.h>

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <string.h>


/*
#define INTERPOLATED(value) value[2]

typedef struct GameEntity
{
	EntityType type;
	uint32_t Subtype;

	INTERPOLATED(float x);
	INTERPOLATED(float y);
	union
	{
		INTERPOLATED(float w);
		INTERPOLATED(float r);
	};
	INTERPOLATED(float h);

	INTERPOLATED(float angle);

	uint32_t MaxHP;
	uint32_t HPBits;
	INTERPOLATED(uint32_t HP);
	INTERPOLATED(float OpacityHP);

	float angleDir;

	INTERPOLATED(float Damageness);
}
GameEntity;

typedef struct GameState
{
	uint64_t Duration;

	GameEntity* Entities;
	uint16_t EntityCount;

	float INTERPOLATED(FoV);
	float INTERPOLATED(CameraX);
	float INTERPOLATED(CameraY);
}
GameState;

#undef INTERPOLATED


GameState States[GAME_CONST_BUFFERED_STATES];
sync_mtx_t StateMutex;
uintptr_t CurrentStateIdx;
uint64_t CompletedStates = -1;
uint64_t CompletedTime;


float
LerpF(
	float old,
	float New,
	float By
	)
{
	return old + (New - old) * By;
}


float
ShortestAngleDifference(
	float A,
	float b
	)
{
	float Diff = b - A;

	if(fabsf(Diff) > M_PI)
	{
		if(Diff > 0)
		{
			Diff -= M_PI * 2.0;
		}
		else
		{
			Diff += M_PI * 2.0;
		}
	}

	return Diff;
}


#define LERP(value) LerpF(value[0], value[1], Scale)

#define OLD 0
#define NEW 1

#define NEW_VALUE(To, value) new_state->To[NEW] = (value)
#define NEW_ENTITY_VALUE(To, value) NewEntity->To[NEW] = (value)
#define COPY_OVER(To) new_state->To[OLD] = old_state->To[NEW]
#define SHIFT_OVER(To) NewEntity->To[OLD] = NewEntity->To[NEW]

void
OnPacket(
	bit_buffer_t* buffer,
	ServerOpCode OpCode
	)
{
	switch(OpCode)
	{

	case SERVER_OPCODE_UPDATE:
	{
		sync_mtx_lock(&StateMutex);

		++CompletedStates;

		GameState* old_state = States + CurrentStateIdx;
		CurrentStateIdx = (CurrentStateIdx + 1) % MACRO_ARRAY_LEN(States);
		GameState* new_state = States + CurrentStateIdx;

		if(!CompletedStates)
		{
			CompletedTime = WindowGetTime();
		}
		else
		{
			CompletedTime += new_state->Duration;
		}

		old_state->Duration = bit_buffer_get_bits(buffer, FIELD_SIZE_TICK_DURATION) * 10000;
		new_state->Duration = old_state->Duration;

		COPY_OVER(FoV);
		NEW_VALUE(FoV, bit_buffer_get_fixed_point(buffer, FIXED_POINT(FOV)));

		COPY_OVER(CameraX);
		NEW_VALUE(CameraX, bit_buffer_get_signed_fixed_point(buffer, FIXED_POINT(POS)));

		COPY_OVER(CameraY);
		NEW_VALUE(CameraY, bit_buffer_get_signed_fixed_point(buffer, FIXED_POINT(POS)));

		uint16_t count = bit_buffer_get_bits(buffer, GAME_CONST_MAX_ENTITIES__BITS);

		GameEntity* NewEntities = malloc(sizeof(*NewEntities) * count);
		assert_not_null(NewEntities);

		GameEntity* OldEntity = old_state->Entities;
		GameEntity* NewEntity = NewEntities;

		for(uint16_t i = 0; i < count; ++i)
		{
			uint8_t OldSet = bit_buffer_get_bits(buffer, 1);
			uint8_t NewSet = bit_buffer_get_bits(buffer, 1);

			if(!OldSet)
			{
				/* Creation *\/

				NewEntity->type = bit_buffer_get_bits(buffer, ENTITY_TYPE__BITS);
				NewEntity->Subtype = bit_buffer_get_bits(buffer, TypeToSubtypeBits[NewEntity->type]);

				NEW_ENTITY_VALUE(x, bit_buffer_get_signed_fixed_point(buffer, FIXED_POINT(SCREEN_POS)));
				SHIFT_OVER(x);

				NEW_ENTITY_VALUE(y, bit_buffer_get_signed_fixed_point(buffer, FIXED_POINT(SCREEN_POS)));
				SHIFT_OVER(y);


				switch(NewEntity->type)
				{

				case ENTITY_TYPE_TANK:
				{
					NEW_ENTITY_VALUE(r, bit_buffer_get_fixed_point(buffer, FIXED_POINT(RADIUS)));
					SHIFT_OVER(r);

					NewEntity->MaxHP = 1000;
					NewEntity->HPBits = 10;

					break;
				}

				case ENTITY_TYPE_SHAPE:
				{
					NEW_ENTITY_VALUE(r, ShapeRadius[NewEntity->Subtype]);
					SHIFT_OVER(r);

					NEW_ENTITY_VALUE(angle, rand_angle());
					SHIFT_OVER(angle);

					NewEntity->angleDir = rand_bool() ? 1 : -1;

					NewEntity->MaxHP = ShapeMaxHP[NewEntity->Subtype];
					NewEntity->HPBits = ShapeHPBits[NewEntity->Subtype];

					break;
				}

				default: __builtin_unreachable();

				}


				switch(NewEntity->type)
				{

				case ENTITY_TYPE_TANK:
				case ENTITY_TYPE_SHAPE:
				{
					uintptr_t WroteHP = bit_buffer_get_bits(buffer, 1);

					if(bit_buffer_get_bits(buffer, 1))
					{
						NEW_ENTITY_VALUE(Damageness, 0.1666f);
					}
					else
					{
						NEW_ENTITY_VALUE(Damageness, 0.0f);
					}

					SHIFT_OVER(Damageness);

					if(WroteHP)
					{
						NEW_ENTITY_VALUE(HP, bit_buffer_get_bits(buffer, NewEntity->HPBits));
						NEW_ENTITY_VALUE(OpacityHP, 1.0f);
					}
					else
					{
						NEW_ENTITY_VALUE(HP, NewEntity->MaxHP);
						NEW_ENTITY_VALUE(OpacityHP, 0.0f);
					}

					SHIFT_OVER(HP);
					SHIFT_OVER(OpacityHP);

					break;
				}

				default: __builtin_unreachable();

				}


				++NewEntity;
			}
			else if(NewSet)
			{
				/* Update *\/

				*NewEntity = *OldEntity;

				SHIFT_OVER(x);
				SHIFT_OVER(y);
				SHIFT_OVER(w);
				SHIFT_OVER(h);
				SHIFT_OVER(angle);
				SHIFT_OVER(HP);
				SHIFT_OVER(OpacityHP);
				SHIFT_OVER(Damageness);


				switch(NewEntity->type)
				{

				case ENTITY_TYPE_TANK:
				case ENTITY_TYPE_SHAPE:
				{
					NEW_ENTITY_VALUE(x, bit_buffer_get_signed_fixed_point(buffer, FIXED_POINT(SCREEN_POS)));
					NEW_ENTITY_VALUE(y, bit_buffer_get_signed_fixed_point(buffer, FIXED_POINT(SCREEN_POS)));

					break;
				}

				default: __builtin_unreachable();

				}


				switch(NewEntity->type)
				{

				case ENTITY_TYPE_TANK:
				case ENTITY_TYPE_SHAPE:
				{
					uintptr_t WroteHP = bit_buffer_get_bits(buffer, 1);

					if(bit_buffer_get_bits(buffer, 1))
					{
						NEW_ENTITY_VALUE(Damageness, MACRO_MIN(0.5f, NewEntity->Damageness[OLD] + 0.1666f));
					}
					else
					{
						NEW_ENTITY_VALUE(Damageness, MACRO_MAX(0.0f, NewEntity->Damageness[OLD] - 0.1666f));
					}

					if(WroteHP)
					{
						NEW_ENTITY_VALUE(HP, bit_buffer_get_bits(buffer, NewEntity->HPBits));
					}

					break;
				}

				default: __builtin_unreachable();

				}


				switch(NewEntity->type)
				{

				case ENTITY_TYPE_TANK:
				{
					break;
				}

				case ENTITY_TYPE_SHAPE:
				{
					float angle = 0.001f * NewEntity->angleDir * (250.0f / NewEntity->r[NEW]);
					NEW_ENTITY_VALUE(angle, NewEntity->angle[OLD] + angle);

					break;
				}

				default: __builtin_unreachable();

				}


				if(NewEntity->HP[NEW] != NewEntity->MaxHP)
				{
					NEW_ENTITY_VALUE(OpacityHP, MACRO_MIN(1.0f, NewEntity->OpacityHP[OLD] + 0.15f));
				}
				else
				{
					NEW_ENTITY_VALUE(OpacityHP, MACRO_MAX(0.0f, NewEntity->OpacityHP[OLD] - 0.15f));
				}

				++OldEntity;
				++NewEntity;
			}
			else
			{
				/* Removal *\/

				++OldEntity;
			}
		}

		assert_eq(BitBufferConsumed(buffer), buffer->len);

		free(new_state->Entities);
		new_state->Entities = NewEntities;
		new_state->EntityCount = NewEntity - NewEntities;

		sync_mtx_unlock(&StateMutex);

		break;
	}

	default: __builtin_unreachable();

	}
}

#undef SHIFT_OVER
#undef COPY_OVER
#undef NEW_ENTITY_VALUE
#undef NEW_VALUE


void
SocketOnOpen(
	void
	)
{
	puts("socket open");
}


uint32_t
SocketOnData(
	uint8_t* data,
	uint32_t len
	)
{
	bit_buffer_t buffer = {0};
	buffer.buffer = data;
	buffer.at = buffer.buffer;
	buffer.len = len;

	if(bit_buffer_available_bits(&buffer) < SERVER_OPCODE__BITS)
	{
		return 0;
	}

	ServerOpCode OpCode = bit_buffer_get_bits(&buffer, SERVER_OPCODE__BITS);

	switch(OpCode)
	{

	case SERVER_OPCODE_UPDATE:
	{
		if(bit_buffer_available_bits(&buffer) < GAME_CONST_SERVER_PACKET_SIZE__BITS)
		{
			return 0;
		}

		uintptr_t size = bit_buffer_get_bits(&buffer, GAME_CONST_SERVER_PACKET_SIZE__BITS);

		if(size <= len)
		{
			buffer.len = size;

			OnPacket(&buffer, OpCode);

			return size;
		}

		return 0;
	}

	default: __builtin_unreachable();

	}
}


void
SocketOnClose(
	void
	)
{
	puts("socket close");
}

*//*
VkVertexInstanceInput* DrawInput;
uint32_t DrawCount;


void
DrawBar(
	float x,
	float y,
	float w,
	float Scale,
	float Opacity,
	DrawDepth depth
	)
{
	float BlackBarHalfHeight = w * 0.125f;
	float BlackBarHalfWidth = w - BlackBarHalfHeight;
	float BlackBarHeight = BlackBarHalfHeight * 2.0f;
	float BlackBarLeft = x - BlackBarHalfWidth;
	float BlackBarRight = x + BlackBarHalfWidth;
	float BlackBarMiddle = (BlackBarLeft + BlackBarRight) * 0.5f;

	float Y_Pos = y + BlackBarHalfHeight;

	float HPBarScale = 0.75f;
	float HPBarPadding = BlackBarHalfHeight * HPBarScale;
	float HPBarHalfHeight = BlackBarHalfHeight - HPBarPadding;
	float HPBarHalfWidth = BlackBarHalfWidth - HPBarPadding * (1.0f - HPBarScale);
	float HPBarHeight = HPBarHalfHeight * 2.0f;
	float HPBarLeft = x - HPBarHalfWidth;
	float HPBarRight = HPBarLeft + HPBarHalfWidth * 2.0f * Scale;
	float HPBarMiddle = (HPBarLeft + HPBarRight) * 0.5f;

	float r = MACRO_MIN(1.000f, (1.000f - Scale) * 2.0f);
	float g = MACRO_MIN(1.000f, Scale * 2.0f);
	float b = 0.000f;


	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.pos = { HPBarMiddle, Y_Pos, DRAW_DEPTH(depth + 4) },
		.color = { r, g, b, Opacity },
		.size = { HPBarRight - HPBarLeft, HPBarHeight },
		.angle = 0.0f,
		.tex_scale = { 1.0f, 1.0f },
		.tex_offset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_RECT,
		.TexIndex = TEXTURE_RECT
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.pos = { HPBarLeft, Y_Pos, DRAW_DEPTH(depth + 3) },
		.color = { r, g, b, Opacity },
		.size = { HPBarHeight, HPBarHeight },
		.angle = 0.0f,
		.tex_scale = { 1.0f, 1.0f },
		.tex_offset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_CIRCLE,
		.TexIndex = TEXTURE_CIRCLE
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.pos = { HPBarRight, Y_Pos, DRAW_DEPTH(depth + 3) },
		.color = { r, g, b, Opacity },
		.size = { HPBarHeight, HPBarHeight },
		.angle = 0.0f,
		.tex_scale = { 1.0f, 1.0f },
		.tex_offset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_CIRCLE,
		.TexIndex = TEXTURE_CIRCLE
	};


	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.pos = { BlackBarMiddle, Y_Pos, DRAW_DEPTH(depth + 2) },
		.color = { 0.000f, 0.000f, 0.000f, Opacity },
		.size = { BlackBarRight - BlackBarLeft, BlackBarHeight },
		.angle = 0.0f,
		.tex_scale = { 1.0f, 1.0f },
		.tex_offset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_RECT,
		.TexIndex = TEXTURE_RECT
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.pos = { BlackBarLeft, Y_Pos, DRAW_DEPTH(depth + 1) },
		.color = { 0.000f, 0.000f, 0.000f, Opacity },
		.size = { BlackBarHeight, BlackBarHeight },
		.angle = 0.0f,
		.tex_scale = { 1.0f, 1.0f },
		.tex_offset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_CIRCLE,
		.TexIndex = TEXTURE_CIRCLE
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.pos = { BlackBarRight, Y_Pos, DRAW_DEPTH(depth + 1) },
		.color = { 0.000f, 0.000f, 0.000f, Opacity },
		.size = { BlackBarHeight, BlackBarHeight },
		.angle = 0.0f,
		.tex_scale = { 1.0f, 1.0f },
		.tex_offset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_CIRCLE,
		.TexIndex = TEXTURE_CIRCLE
	};
}


float
TextWidth(
	const char* Text,
	uint32_t FontSize
	)
{
	const char* Char = Text;
	uint32_t width = 0;

	while(*Char)
	{
		width += FontData[*(Char++) - ' '].Stride;
	}

	float Scale = (float) FontSize / FONT_SIZE;
	return width * Scale / 64.0f;
}


typedef enum TextAlign
{
	TEXT_ALIGN_LEFT,
	TEXT_ALIGN_CENTER,
	TEXT_ALIGN_RIGHT
}
TextAlign;

typedef enum TextBaseline
{
	TEXT_BASELINE_BOTTOM,
	TEXT_BASELINE_MIDDLE,
	TEXT_BASELINE_TOP
}
TextBaseline;

void
DrawAText(
	const char* Text,
	float x,
	float y,
	float Opacity,
	uint32_t FontSize,
	TextAlign Align,
	TextBaseline Baseline,
	DrawDepth depth
	)
{
	x -= TextWidth(Text, FontSize) * Align * 0.5f;
	y += FontSize * Baseline * 0.3333f;

	float Scale = (float) FontSize / FONT_SIZE;


	const char* Char = Text;
	float CurrentDepth = DRAW_DEPTH(depth - 1);
	float CurrentX = x;

	while(*Char)
	{
		const FontDrawData* data = FontData + (*(Char++) - ' ');

		float top = data->top  * Scale;
		float left = data->left * Scale;
		float Stride = data->Stride * Scale / 64.0f;
		float size = data->size * Scale;

		DrawInput[DrawCount++] =
		(VkVertexInstanceInput)
		{
			.pos = { CurrentX + size / 2 + left, y + size / 2 - top, CurrentDepth },
			.color = { 1.000f, 1.000f, 1.000f, Opacity },
			.size = { size, size },
			.angle = 0.0f,
			.tex_scale = { 1.0f, 1.0f },
			.tex_offset = { 0.5f, 0.5f },
			.TexRes = data->BgTexRes,
			.TexIndex = data->BgTexIndex
		};

		CurrentX += Stride;
	}


	Char = Text;
	CurrentDepth = DRAW_DEPTH(depth);
	CurrentX = x;

	while(*Char)
	{
		const FontDrawData* data = FontData + (*(Char++) - ' ');

		float top = data->top  * Scale;
		float left = data->left * Scale;
		float Stride = data->Stride * Scale / 64.0f;
		float size = data->size * Scale;

		DrawInput[DrawCount++] =
		(VkVertexInstanceInput)
		{
			.pos = { CurrentX + size / 2 + left, y + size / 2 - top, CurrentDepth },
			.color = { 1.000f, 1.000f, 1.000f, Opacity },
			.size = { size, size },
			.angle = 0.0f,
			.tex_scale = { 1.0f, 1.0f },
			.tex_offset = { 0.5f, 0.5f },
			.TexRes = data->TexRes,
			.TexIndex = data->TexIndex
		};

		CurrentX += Stride;
	}
}


window_draw_event_data_t
VulkanGetDrawData___(
	void
	)
{
	DrawCount = 0;

	if(CompletedStates == -1 || CompletedStates < MACRO_ARRAY_LEN(States))
	{
		return
		(window_draw_event_data_t)
		{
			.Input = DrawInput,
			.count = DrawCount
		};
	}

	sync_mtx_lock(&StateMutex);

	uintptr_t OldestStateIdx = (CurrentStateIdx + 1) % MACRO_ARRAY_LEN(States);
	GameState* OldestState = States + OldestStateIdx;

	uint64_t TotalTime = 0;

	GameState* state = States;
	GameState* StateEnd = state + MACRO_ARRAY_LEN(States);

	do
	{
		TotalTime += state->Duration;
	}
	while(++state != StateEnd);

	static uint64_t DrawAt = 0;
	static uint64_t LastDrawAt = 0;

	uint64_t Now = GetTime();

	if(!LastDrawAt)
	{
		LastDrawAt = Now;
	}

	DrawAt += Now - LastDrawAt;
	LastDrawAt = Now;

	DrawAt = MACRO_MAX(MACRO_MIN(DrawAt, CompletedTime + TotalTime), CompletedTime);
	uint64_t StepAt = DrawAt - CompletedTime;

	uintptr_t StateIdx = OldestStateIdx;
	state = OldestState;

	while(StepAt > state->Duration)
	{
		StepAt -= state->Duration;
		StateIdx = (StateIdx + 1) % MACRO_ARRAY_LEN(States);
		state = States + StateIdx;
	}

	float Scale = (float)((double) StepAt / (double) state->Duration);

	float FoV = LERP(state->FoV);
	float CameraX = LERP(state->CameraX);
	float CameraY = LERP(state->CameraY);

	float TileSize = GAME_CONST_TILE_SIZE;
	float ScaledTileSize = TileSize * FoV;
	float XMod = fmodf(CameraX, TileSize) / TileSize;
	float YMod = fmodf(CameraY, TileSize) / TileSize;

	float ArenaMinX = MACRO_MAX(-960.0f, (-GAME_CONST_HALF_ARENA_SIZE - CameraX) * FoV);
	float ArenaMinY = MACRO_MAX(-540.0f, (-GAME_CONST_HALF_ARENA_SIZE - CameraY) * FoV);
	float ArenaMaxX = MACRO_MIN(960.0f, (GAME_CONST_HALF_ARENA_SIZE - CameraX) * FoV);
	float ArenaMaxY = MACRO_MIN(540.0f, (GAME_CONST_HALF_ARENA_SIZE - CameraY) * FoV);

	if(ArenaMinX != ArenaMaxX && ArenaMinY != ArenaMaxY)
	{
		float ArenaX = (ArenaMinX + ArenaMaxX) * 0.5f;
		float ArenaY = (ArenaMinY + ArenaMaxY) * 0.5f;
		float ArenaWidth = ArenaMaxX - ArenaMinX;
		float ArenaHeight = ArenaMaxY - ArenaMinY;
		float ScaledXMod = XMod + fmodf(ArenaX, ScaledTileSize) / ScaledTileSize;
		float ScaledYMod = YMod + fmodf(ArenaY, ScaledTileSize) / ScaledTileSize;

		DrawInput[DrawCount++] =
		(VkVertexInstanceInput)
		{
			.pos = { ArenaX, ArenaY, DRAW_DEPTH(DRAW_DEPTH_BACKGROUND) },
			.color = { 1.0f, 1.0f, 1.0f, 1.0f },
			.size = { ArenaWidth, ArenaHeight },
			.angle = 0,
			.tex_scale = { ArenaWidth / ScaledTileSize, ArenaHeight / ScaledTileSize },
			.tex_offset = { ScaledXMod, ScaledYMod },
			.TexRes = TEXTURE_RESOLUTION_BG_LIGHT,
			.TexIndex = TEXTURE_BG_LIGHT
		};
	}

	float LeftBorderMinX = -960.0f;
	float LeftBorderMinY = -540.0f;
	float LeftBorderMaxX = ArenaMinX;
	float LeftBorderMaxY = 540.0f;

	if(LeftBorderMinX != LeftBorderMaxX && LeftBorderMinY != LeftBorderMaxY)
	{
		float BorderX = (LeftBorderMinX + LeftBorderMaxX) * 0.5f;
		float BorderY = (LeftBorderMinY + LeftBorderMaxY) * 0.5f;
		float BorderWidth = LeftBorderMaxX - LeftBorderMinX;
		float BorderHeight = LeftBorderMaxY - LeftBorderMinY;
		float ScaledXMod = XMod + fmodf(BorderX, ScaledTileSize) / ScaledTileSize;
		float ScaledYMod = YMod + fmodf(BorderY, ScaledTileSize) / ScaledTileSize;

		DrawInput[DrawCount++] =
		(VkVertexInstanceInput)
		{
			.pos = { BorderX, BorderY, DRAW_DEPTH(DRAW_DEPTH_BACKGROUND) },
			.color = { 1.0f, 1.0f, 1.0f, 1.0f },
			.size = { BorderWidth, BorderHeight },
			.angle = 0,
			.tex_scale = { BorderWidth / ScaledTileSize, BorderHeight / ScaledTileSize },
			.tex_offset = { ScaledXMod, ScaledYMod },
			.TexRes = TEXTURE_RESOLUTION_BG_DARK,
			.TexIndex = TEXTURE_BG_DARK
		};
	}

	float RightBorderMinX = ArenaMaxX;
	float RightBorderMinY = -540.0f;
	float RightBorderMaxX = 960.0f;
	float RightBorderMaxY = 540.0f;

	if(RightBorderMinX != RightBorderMaxX && RightBorderMinY != RightBorderMaxY)
	{
		float BorderX = (RightBorderMinX + RightBorderMaxX) * 0.5f;
		float BorderY = (RightBorderMinY + RightBorderMaxY) * 0.5f;
		float BorderWidth = RightBorderMaxX - RightBorderMinX;
		float BorderHeight = RightBorderMaxY - RightBorderMinY;
		float ScaledXMod = XMod + fmodf(BorderX, ScaledTileSize) / ScaledTileSize;
		float ScaledYMod = YMod + fmodf(BorderY, ScaledTileSize) / ScaledTileSize;

		DrawInput[DrawCount++] =
		(VkVertexInstanceInput)
		{
			.pos = { BorderX, BorderY, DRAW_DEPTH(DRAW_DEPTH_BACKGROUND) },
			.color = { 1.0f, 1.0f, 1.0f, 1.0f },
			.size = { BorderWidth, BorderHeight },
			.angle = 0,
			.tex_scale = { BorderWidth / ScaledTileSize, BorderHeight / ScaledTileSize },
			.tex_offset = { ScaledXMod, ScaledYMod },
			.TexRes = TEXTURE_RESOLUTION_BG_DARK,
			.TexIndex = TEXTURE_BG_DARK
		};
	}

	float BottomBorderMinX = -960.0f;
	float BottomBorderMinY = ArenaMaxY;
	float BottomBorderMaxX = 960.0f;
	float BottomBorderMaxY = 540.0f;

	if(BottomBorderMinX != BottomBorderMaxX && BottomBorderMinY != BottomBorderMaxY)
	{
		float BorderX = (BottomBorderMinX + BottomBorderMaxX) * 0.5f;
		float BorderY = (BottomBorderMinY + BottomBorderMaxY) * 0.5f;
		float BorderWidth = BottomBorderMaxX - BottomBorderMinX;
		float BorderHeight = BottomBorderMaxY - BottomBorderMinY;
		float ScaledXMod = XMod + fmodf(BorderX, ScaledTileSize) / ScaledTileSize;
		float ScaledYMod = YMod + fmodf(BorderY, ScaledTileSize) / ScaledTileSize;

		DrawInput[DrawCount++] =
		(VkVertexInstanceInput)
		{
			.pos = { BorderX, BorderY, DRAW_DEPTH(DRAW_DEPTH_BACKGROUND) },
			.color = { 1.0f, 1.0f, 1.0f, 1.0f },
			.size = { BorderWidth, BorderHeight },
			.angle = 0,
			.tex_scale = { BorderWidth / ScaledTileSize, BorderHeight / ScaledTileSize },
			.tex_offset = { ScaledXMod, ScaledYMod },
			.TexRes = TEXTURE_RESOLUTION_BG_DARK,
			.TexIndex = TEXTURE_BG_DARK
		};
	}

	float TopBorderMinX = -960.0f;
	float TopBorderMinY = -540.0f;
	float TopBorderMaxX = 960.0f;
	float TopBorderMaxY = ArenaMinY;

	if(TopBorderMinX != TopBorderMaxX && TopBorderMinY != TopBorderMaxY)
	{
		float BorderX = (TopBorderMinX + TopBorderMaxX) * 0.5f;
		float BorderY = (TopBorderMinY + TopBorderMaxY) * 0.5f;
		float BorderWidth = TopBorderMaxX - TopBorderMinX;
		float BorderHeight = TopBorderMaxY - TopBorderMinY;
		float ScaledXMod = XMod + fmodf(BorderX, ScaledTileSize) / ScaledTileSize;
		float ScaledYMod = YMod + fmodf(BorderY, ScaledTileSize) / ScaledTileSize;

		DrawInput[DrawCount++] =
		(VkVertexInstanceInput)
		{
			.pos = { BorderX, BorderY, DRAW_DEPTH(DRAW_DEPTH_BACKGROUND) },
			.color = { 1.0f, 1.0f, 1.0f, 1.0f },
			.size = { BorderWidth, BorderHeight },
			.angle = 0,
			.tex_scale = { BorderWidth / ScaledTileSize, BorderHeight / ScaledTileSize },
			.tex_offset = { ScaledXMod, ScaledYMod },
			.TexRes = TEXTURE_RESOLUTION_BG_DARK,
			.TexIndex = TEXTURE_BG_DARK
		};
	}

	GameEntity* Entity = state->Entities;
	GameEntity* EntityEnd = Entity + state->EntityCount;

	typedef struct EntityDrawData
	{
		float x;
		float y;
		float w;
		float h;

		DrawDepth depth;

		float angle;

		float HPScale;
		float OpacityHP;

		float Damageness;

		uint32_t TexRes;
		uint32_t TexIndex;

		float r;
		float g;
		float b;
		float A;
	}
	EntityDrawData;

	EntityDrawData EntityData[state->EntityCount];
	EntityDrawData* data = EntityData;

	while(Entity != EntityEnd)
	{
		data->x = LERP(Entity->x);
		data->y = LERP(Entity->y);
		data->w = LERP(Entity->w);

		switch(Entity->type)
		{

		case ENTITY_TYPE_TANK:
		case ENTITY_TYPE_SHAPE:
		{
			data->h = data->w;

			break;
		}

		default: __builtin_unreachable();

		}


		data->w *= FoV;
		data->h *= FoV;

		data->angle =
			Entity->angle[OLD] + ShortestAngleDifference(Entity->angle[OLD], Entity->angle[NEW]) * Scale;

		data->HPScale = LERP(Entity->HP) / Entity->MaxHP;
		data->OpacityHP = LERP(Entity->OpacityHP);

		data->Damageness = LERP(Entity->Damageness);

		data->A = 1.000f;


		switch(Entity->type)
		{

		case ENTITY_TYPE_TANK:
		{
			data->depth = DRAW_DEPTH_TANK;

			data->r = 0.000f;
			data->g = 0.450f;
			data->b = 0.750f;

			data->TexRes = TEXTURE_RESOLUTION_TANK;
			data->TexIndex = TEXTURE_TANK;

			DrawAText("23.5k",
				data->x,
				data->y - data->h - 4,
				1.0f,
				16,
				TEXT_ALIGN_CENTER,
				TEXT_BASELINE_BOTTOM,
				DRAW_DEPTH_TEST1);

			DrawAText("Shadam",
				data->x,
				data->y - data->h - 20,
				1.0f,
				32,
				TEXT_ALIGN_CENTER,
				TEXT_BASELINE_BOTTOM,
				DRAW_DEPTH_TEST1);

			break;
		}

		case ENTITY_TYPE_SHAPE:
		{
			data->depth = DRAW_DEPTH_SHAPE;

			switch(Entity->Subtype)
			{

			case SHAPE_SQUARE:
			{
				data->TexRes = TEXTURE_RESOLUTION_SQUARE;
				data->TexIndex = TEXTURE_SQUARE;

				data->r = 1.000f;
				data->g = 0.800f;
				data->b = 0.200f;

				break;
			}

			case SHAPE_TRIANGLE:
			{
				data->TexRes = TEXTURE_RESOLUTION_TRIANGLE;
				data->TexIndex = TEXTURE_TRIANGLE;

				data->r = 1.000f;
				data->g = 0.250f;
				data->b = 0.250f;

				break;
			}

			case SHAPE_PENTAGON:
			{
				data->TexRes = TEXTURE_RESOLUTION_PENTAGON;
				data->TexIndex = TEXTURE_PENTAGON;

				data->r = 0.150f;
				data->g = 0.200f;
				data->b = 1.000f;

				break;
			}

			default: __builtin_unreachable();

			}

			break;
		}

		default: __builtin_unreachable();

		}


		data->r = LerpF(data->r, 1.000f, data->Damageness);
		data->b = LerpF(data->b, 0.000f, data->Damageness);
		data->g = LerpF(data->g, 0.000f, data->Damageness);


		DrawInput[DrawCount++] =
		(VkVertexInstanceInput)
		{
			.pos = { data->x, data->y, DRAW_DEPTH(data->depth) },
			.color = { data->r, data->g, data->b, data->A },
			.size = { data->w * 2.0f, data->h * 2.0f },
			.angle = data->angle,
			.tex_scale = { 1.0f, 1.0f },
			.tex_offset = { 0.5f, 0.5f },
			.TexRes = data->TexRes,
			.TexIndex = data->TexIndex
		};


		++Entity;
		++data;
	}

	data = EntityData;
	EntityDrawData* data_end = EntityData + MACRO_ARRAY_LEN(EntityData);

	while(data != data_end)
	{
		if(data->OpacityHP != 0.0f)
		{
			assert_eq(data->w, data->h);
			DrawBar(data->x, data->y + data->h + 1.0f, data->w, data->HPScale, data->OpacityHP, data->depth);
		}

		++data;
	}


	float MinimapHalfSize = 100.0f;
	float MinimapSize = MinimapHalfSize * 2.0f;
	float MinimapPadding = 20.0f;
	float MinimapX = 960.0f - MinimapHalfSize - MinimapPadding;
	float MinimapY = 540.0f - MinimapHalfSize - MinimapPadding;
	float MinimapBorder = 6.0f;
	float MinimapColor = 0.200f;


	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.pos = { MinimapX, MinimapY, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_RECT) },
		.color = { 1.000f, 1.000f, 1.000f, 0.1f },
		.size = { MinimapSize, MinimapSize },
		.angle = 0.0f,
		.tex_scale = { 1.0f, 1.0f },
		.tex_offset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_RECT,
		.TexIndex = TEXTURE_RECT
	};


	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.pos = { MinimapX - MinimapHalfSize, MinimapY, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_BORDER) },
		.color = { MinimapColor, MinimapColor, MinimapColor, 1.0f },
		.size = { MinimapBorder, MinimapSize },
		.angle = 0.0f,
		.tex_scale = { 1.0f, 1.0f },
		.tex_offset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_RECT,
		.TexIndex = TEXTURE_RECT
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.pos = { MinimapX + MinimapHalfSize, MinimapY, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_BORDER) },
		.color = { MinimapColor, MinimapColor, MinimapColor, 1.0f },
		.size = { MinimapBorder, MinimapSize },
		.angle = 0.0f,
		.tex_scale = { 1.0f, 1.0f },
		.tex_offset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_RECT,
		.TexIndex = TEXTURE_RECT
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.pos = { MinimapX, MinimapY - MinimapHalfSize, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_BORDER) },
		.color = { MinimapColor, MinimapColor, MinimapColor, 1.0f },
		.size = { MinimapSize, MinimapBorder },
		.angle = 0.0f,
		.tex_scale = { 1.0f, 1.0f },
		.tex_offset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_RECT,
		.TexIndex = TEXTURE_RECT
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.pos = { MinimapX, MinimapY + MinimapHalfSize, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_BORDER) },
		.color = { MinimapColor, MinimapColor, MinimapColor, 1.0f },
		.size = { MinimapSize, MinimapBorder },
		.angle = 0.0f,
		.tex_scale = { 1.0f, 1.0f },
		.tex_offset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_RECT,
		.TexIndex = TEXTURE_RECT
	};


	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.pos = { MinimapX - MinimapHalfSize, MinimapY - MinimapHalfSize, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_CORNER) },
		.color = { MinimapColor, MinimapColor, MinimapColor, 1.0f },
		.size = { MinimapBorder, MinimapBorder },
		.angle = 0.0f,
		.tex_scale = { 1.0f, 1.0f },
		.tex_offset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_CIRCLE,
		.TexIndex = TEXTURE_CIRCLE
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.pos = { MinimapX + MinimapHalfSize, MinimapY - MinimapHalfSize, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_CORNER) },
		.color = { MinimapColor, MinimapColor, MinimapColor, 1.0f },
		.size = { MinimapBorder, MinimapBorder },
		.angle = 0.0f,
		.tex_scale = { 1.0f, 1.0f },
		.tex_offset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_CIRCLE,
		.TexIndex = TEXTURE_CIRCLE
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.pos = { MinimapX - MinimapHalfSize, MinimapY + MinimapHalfSize, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_CORNER) },
		.color = { MinimapColor, MinimapColor, MinimapColor, 1.0f },
		.size = { MinimapBorder, MinimapBorder },
		.angle = 0.0f,
		.tex_scale = { 1.0f, 1.0f },
		.tex_offset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_CIRCLE,
		.TexIndex = TEXTURE_CIRCLE
	};

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.pos = { MinimapX + MinimapHalfSize, MinimapY + MinimapHalfSize, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_CORNER) },
		.color = { MinimapColor, MinimapColor, MinimapColor, 1.0f },
		.size = { MinimapBorder, MinimapBorder },
		.angle = 0.0f,
		.tex_scale = { 1.0f, 1.0f },
		.tex_offset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_CIRCLE,
		.TexIndex = TEXTURE_CIRCLE
	};


	float MinimapCameraX = (CameraX / (GAME_CONST_HALF_ARENA_SIZE + (GAME_CONST_BORDER_PADDING >> 1))) * MinimapHalfSize;
	float MinimapCameraY = (CameraY / (GAME_CONST_HALF_ARENA_SIZE + (GAME_CONST_BORDER_PADDING >> 1))) * MinimapHalfSize;

	DrawInput[DrawCount++] =
	(VkVertexInstanceInput)
	{
		.pos = { MinimapX + MinimapCameraX, MinimapY + MinimapCameraY, DRAW_DEPTH(DRAW_DEPTH_MINIMAP_INDICATOR) },
		.color = { 0.100f, 0.100f, 0.100f, 1.0f },
		.size = { 6.0f, 6.0f },
		.angle = 0.0f,
		.tex_scale = { 1.0f, 1.0f },
		.tex_offset = { 0.5f, 0.5f },
		.TexRes = TEXTURE_RESOLUTION_CIRCLE,
		.TexIndex = TEXTURE_CIRCLE
	};


	sync_mtx_unlock(&StateMutex);

	return
	(window_draw_event_data_t)
	{
		.Input = DrawInput,
		.count = DrawCount
	};
}

#undef LERP
*/

/*
void
UIOnKeyDown(
	UIKey key
	)
{
	(void) key;
}


void
UIOnKeyUp(
	UIKey key
	)
{
	(void) key;
}


void
UIOnMouseDown(
	UIMouseDownEventData* data
	)
{
	(void) data;
}


void
UIOnMouseUp(
	UIMouseUpEventData* data
	)
{
	(void) data;
}


void
UIOnMouseMove(
	UIMouseMoveEventData* data
	)
{
	//printf("game mousemove %f %f\n", x, y);
}


void
UIOnMouseScroll(
	UIMouseScrollEventData* data
	)
{
	printf("game mousescroll %f\n", data->offset_y);
}
*/


void
WindowOnResize(
	float width,
	float height
	)
{
	printf("window resize %f %f\n", width, height);
}


void
WindowOnFocus(
	void
	)
{
	puts("window focused");
}


void
WindowOnBlur(
	void
	)
{
	puts("window blur");
}


void
WindowOnKeyDown(
	int key,
	int mods,
	int repeat
	)
{
	printf("window keydown %d %d %d\n", key, mods, repeat);
}


void
WindowOnKeyUp(
	int key,
	int mods
	)
{
	printf("window keyup %d %d\n", key, mods);
}


void
WindowOnText(
	const char* Text
	)
{
	printf("window text %s\n", Text);
}


void
WindowOnMouseDown(
	int button
	)
{
	printf("window mousedown %d\n", button);
}


void
lmfao(
	void* null,
	window_mouse_down_event_data_t* data
	)
{
	printf("window mousedown %d %.02f %.02f %hhu\n", data->button, data->pos.x, data->pos.y, data->clicks);
}

void
lol(
	void* null,
	window_mouse_move_event_data_t* data
	)
{
	// printf("window mousemove old %.02f %.02f new %.02f %.02f\n", data->old_pos.x, data->old_pos.y, data->new_pos.x, data->new_pos.y);
}

void
hehe(
	void* null,
	window_resize_event_data_t* data
	)
{
	printf("window resize old %.02f %.02f new %.02f %.02f\n", data->old_size.w, data->old_size.h, data->new_size.w, data->new_size.h);
}


// void
// hihi(
// 	void* null,
// 	UIResizeData* data
// 	)
// {
// 	printf("    ui resize old %.02f %.02f new %.02f %.02f\n", data->old_size.w, data->old_size.h, data->new_size.w, data->new_size.h);
// }


void
WindowOnMouseUp(
	int button
	)
{
	printf("window mouseup %d\n", button);
}


void
WindowOnMouseMove(
	float x,
	float y
	)
{
	printf("window mousemove %f %f\n", x, y);
}


void
WindowOnMouseScroll(
	float offset_y
	)
{
	printf("window mousescroll %f\n", offset_y);
}


window_draw_event_data_t
VulkanGetDrawData(
	void
	)
{
	return (window_draw_event_data_t){0};
}


void
GameInit(
	void
	)
{
	sync_mtx_init(&StateMutex);

	rand_set_seed(WindowGetTime());

	event_target_add(&window_mouse_down_target, (void*) lmfao, NULL);
	event_target_add(&window_mouse_move_target, (void*) lol, NULL);
	event_target_add(&window_resize_target, (void*) hehe, NULL);
}


void
GameFree(
	void
	)
{
	sync_mtx_free(&StateMutex);
}


void
CreateUI(
	void
	)
{
	UIInit();

	UIElement* Window = UIAllocContainer(
		(UIElementInfo)
		{
			.Opacity = 0xFF
		},
		(UIContainerInfo)
		{
			.Axis = UI_AXIS_VERTICAL
		}
		);

	UISetRootElement(Window);

	UIElement* MiddleC = UIAllocContainer(
		(UIElementInfo)
		{
			.BorderRadius = 10.0f,
			.BorderColor = (color_argb_t){ 0x80D0D0D0 },
			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_CENTER,
			.AlignY = UI_ALIGN_MIDDLE,
		},
		(UIContainerInfo)
		{
			.Axis = UI_AXIS_HORIZONTAL,

			.AutoW = true,
			.AutoH = true,

			.white_color = (color_argb_t){ 0x80D0D0D0 },
			.tex = TEXTURE_RECT
		}
		);

	UIInsertLast(MiddleC, Window);

	UIElement* FirstC = UIAllocContainer(
		(UIElementInfo)
		{
			.Margin =
			(half_extent_t)
			{
				.top = 5.0f,
				.left = 5.0f,
				.right = 5.0f,
				.bottom = 5.0f
			},

			.BorderRadius = 5.0f,
			.BorderColor = (color_argb_t){ 0xFFFF1080 },
			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_MIDDLE
		},
		(UIContainerInfo)
		{
			.Axis = UI_AXIS_VERTICAL,

			.AutoW = true,
			.AutoH = true,

			.white_color = (color_argb_t){ 0xFFFF1080 },
			.tex = TEXTURE_RECT
		}
		);

	UIInsertLast(FirstC, MiddleC);

	UIElement* CheckboxC = UIAllocContainer(
		(UIElementInfo)
		{
			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		},
		(UIContainerInfo)
		{
			.Axis = UI_AXIS_HORIZONTAL,

			.AutoW = true,
			.AutoH = true
		}
		);

	UIInsertLast(CheckboxC, FirstC);

	UIElement* Checkbox1 = UIAllocCheckbox(
		(UIElementInfo)
		{
			.Extent =
			(half_extent_t)
			{
				.w = 10.0f,
				.h = 10.0f
			},
			.Margin =
			(half_extent_t)
			{
				.top = 5.0f,
				.left = 5.0f,
				.right = 5.0f,
				.bottom = 5.0f
			},

			.BorderRadius = 10.0f,
			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		},
		(UICheckboxInfo)
		{
			.Checked = true,

			.CheckYes = (color_argb_t){ 0xFF00FF00 },
			.CheckYesBackground = (color_argb_t){ 0xFFFFFFFF },

			.CheckNo = (color_argb_t){ 0xFFFF0000 },
			.CheckNoBackground = (color_argb_t){ 0xFFFFFFFF }
		}
		);

	UIInsertLast(Checkbox1, CheckboxC);

	UIElement* Checkbox2 = UIAllocCheckbox(
		(UIElementInfo)
		{
			.Extent =
			(half_extent_t)
			{
				.w = 0.0f,
				.h = 0.0f
			},
			.Margin =
			(half_extent_t)
			{
				.top = 5.0f,
				.left = 5.0f,
				.right = 5.0f,
				.bottom = 5.0f
			},

			.BorderRadius = 15.0f,
			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		},
		(UICheckboxInfo)
		{
			.Checked = true,

			.CheckYes = (color_argb_t){ 0xFF00FF00 },
			.CheckYesBackground = (color_argb_t){ 0xFFFFFFFF },

			.CheckNo = (color_argb_t){ 0xFFFF0000 },
			.CheckNoBackground = (color_argb_t){ 0xFFFFFFFF }
		}
		);

	UIInsertLast(Checkbox2, CheckboxC);

	/*uint32_t Codes[] = { 'a', 'b', 'c', '\n', 'd', 'e', 'f', '\n' };

	UIElement* T = UIGetElement();
	*T = (UIElement)
	{
		.type = UI_TYPE_TEXT,
		.Text =
		(UIText)
		{
			.Codepoints = Codes,
			.len = MACRO_ARRAY_LEN(Codes),
			.MaxWidth = 120.0f,
			.FontSize = 100.0f
		}
	};

	puts("initializing our amazing text thing");

	UIInitialize(T);

	printf("\nafter initialization, we have %u lines\n", T->Text.LineCount);

	for(uint32_t i = 0; i < T->Text.LineCount; ++i)
	{
		UITextLine* Line = T->Text.Lines[i];
		printf("line #%u: %u glyphs\n", i + 1, Line->len);

		for(uint32_t j = 0; j < Line->len; ++j)
		{
			UITextGlyph* Glyph = Line->Glyphs + j;
			printf("glyph #%u: { top: %.02f, left: %.02f, Stride: %.02f, size: %.02f, tex: { idx: %hu, layer: %hu } }\n",
				j + 1, Glyph->top, Glyph->left, Glyph->Stride, Glyph->size, Glyph->tex.idx, Glyph->tex.layer);
		}

		puts("");
	}

	puts("");*/

	/*IHandle m1 = ICreateContainer(
		&((IElement)
		{
			.h = 300.0f,

			.BorderTop = 10.0f,
			.BorderLeft = 10.0f,
			.BorderRight = 10.0f,
			.BorderBottom = 10.0f,

			.Opacity = 0xFF,
			.BorderColor = (color_argb_t){ 0x333333FF },

			.AlignX = UI_ALIGN_CENTER,
			.AlignY = UI_ALIGN_MIDDLE,

			.InteractiveBorder = true
		}),
		&((IContainer)
		{
			.Axis = UI_AXIS_VERTICAL,

			.AutoW = true,
			.Scrollable = true,

			.white_color = (color_argb_t){ 0x333333FF },
			.tex = TEXTURE_RECT,
			.ScrollbarColor = (color_argb_t){ 0xFFAAAAAA },
			.ScrollbarAltColor = (color_argb_t){ 0xFF717171 },
		})
	);

	IHandle t1 = ICreateText(
		&((IElement)
		{
			.w = 1900.0f,

			.MarginTop = 5.0f,
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,
			.MarginBottom = 5.0f,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		}),
		&((IContainer)
		{
			.AutoH = true,

			.white_color = (color_argb_t){ 0xFFDDDDDD },
			.tex = TEXTURE_RECT
		}),
		&((IText)
		{
			.Str = "A",

			.FontSize = 128,
			.AlignX = UI_ALIGN_LEFT,

			.Selectable = true,
			.Editable = true,

			.Stroke = (color_argb_t){ 0xFF000000 },
			.InverseStroke = (color_argb_t){ 0xFFFFFFFF },
			.Fill = (color_argb_t){ 0xFFFFFFFF },
			.InverseFill = (color_argb_t){ 0xFF000000 },
			.Background = (color_argb_t){ 0xA0000000 },

			.type = I_TEXT_TYPE_MULTILINE_TEXT,
			.data =
			(ITextData)
			{
				.HexColor =
				(ITextHexColor)
				{
					{ .color_argb_t = 0x87654321 }
				}
			}
		})
	);

	IHandle t2 = ICreateText(
		&((IElement)
		{
			.MarginTop = 5.0f,
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,
			.MarginBottom = 5.0f,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		}),
		NULL,
		&((IText)
		{
			.Str = "yes hello",

			.FontSize = 50,
			.AlignX = UI_ALIGN_CENTER,

			.Selectable = true,
			.Editable = true,

			.Stroke = (color_argb_t){ 0xFF000000 },
			.InverseStroke = (color_argb_t){ 0xFFFFFFFF },
			.Fill = (color_argb_t){ 0xFFFFFF20 },
			.InverseFill = (color_argb_t){ 0xFF000020 },
			.Background = (color_argb_t){ 0xA0000000 }
		})
	);

	IHandle s1 = ICreateSlider(
		&((IElement)
		{
			.w = 200.0f,
			.h = 20.0f,

			.MarginTop = 5.0f,
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,
			.MarginBottom = 5.0f,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		}),
		&((ISlider)
		{
			.Axis = UI_AXIS_HORIZONTAL,
			.Sections = 9,
			.value = 4,

			.color = (color_argb_t){ 0xFF4C99E5 },
			.BgColor =(color_argb_t){ 0xFFFFFFFF }
		})
	);

	IHandle p1 = ICreateColorPicker(
		&((IElement)
		{
			.MarginTop = 5.0f,
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,
			.MarginBottom = 5.0f,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		}),
		&((IColorPicker)
		{
			.color = (color_argb_t){ 0x802288FF }
		})
	);

	IHandle p2 = ICreateColorPicker(
		&((IElement)
		{
			.MarginTop = 5.0f,
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,
			.MarginBottom = 5.0f,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		}),
		&((IColorPicker)
		{
			.color = (color_argb_t){ 0x80FF8822 }
		})
	);

	IHandle d1 = ICreateDropdown(
		&((IElement)
		{
			.w = 130.0f,

			.MarginTop = 5.0f,
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,
			.MarginBottom = 5.0f,

			.BorderTop = 5.0f,
			.BorderLeft = 5.0f,
			.BorderRight = 5.0f,
			.BorderBottom = 5.0f,

			.Opacity = 0xFF,
			.BorderColor = (color_argb_t){ 0xFF000000 },

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		}),
		&((IDropdown)
		{
			.BgColor = (color_argb_t){ 0xFFAAAAAA },

			.FontSize = 24,
			.Stroke = (color_argb_t){ 0xFF000000 },
			.Fill = (color_argb_t){ 0xFFFFFFFF },

			.Options =
			(IDropdownOption[])
			{
				{
					.Text = "Carrot"
				},
				{
					.Text = "Cabbage"
				},
				{
					.Text = "Potato"
				},
				{
					.Text = "Onion"
				}
			},
			.count = 4,
			.Chosen = 2
		})
	);

	IHandle d2 = ICreateDropdown(
		&((IElement)
		{
			.w = 240.0f,

			.MarginTop = 5.0f,
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,
			.MarginBottom = 5.0f,

			.BorderTop = 5.0f,
			.BorderLeft = 5.0f,
			.BorderRight = 5.0f,
			.BorderBottom = 5.0f,

			.Opacity = 0xFF,
			.BorderColor = (color_argb_t){ 0xFF000000 },

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_TOP
		}),
		&((IDropdown)
		{
			.BgColor = (color_argb_t){ 0xFFAAAAAA },

			.FontSize = 24,
			.Stroke = (color_argb_t){ 0xFF000000 },
			.Fill = (color_argb_t){ 0xFFFFFFFF },

			.Options =
			(IDropdownOption[])
			{
				{
					.Text = "These can get very long"
				},
				{
					.Text = "and honestly could also contain"
				},
				{
					.Text = "miscellaneous elements besides text"
				},
				{
					.Text = "but for now it is what it is"
				}
			},
			.count = 4,
			.Chosen = 0
		})
	);

	IHandle m2 = ICreateContainer(
		&((IElement)
		{
			.MarginTop = 5.0f,
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,
			.MarginBottom = 5.0f,

			.BorderTop = 5.0f,
			.BorderLeft = 5.0f,
			.BorderRight = 5.0f,
			.BorderBottom = 5.0f,

			.Opacity = 0xFF,
			.BorderColor = (color_argb_t){ 0xFF4C00FF },

			.AlignX = UI_ALIGN_CENTER,
			.AlignY = UI_ALIGN_TOP,
			.pos = UI_POSITION_RELATIVE,
			.RelativeAlignX = UI_ALIGN_CENTER,
			.RelativeAlignY = UI_ALIGN_TOP,

			.Relative = &p1
		}),
		&((IContainer)
		{
			.Axis = UI_AXIS_HORIZONTAL,

			.AutoW = true,
			.AutoH = true,

			.color = (color_argb_t){ 0xFF804CB2 },
			.tex = TEXTURE_RECT
		})
	);

	IHandle c1 = ICreateCheckbox(
		&((IElement)
		{
			.w = 32.0f,
			.h = 32.0f,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_CENTER
		}),
		&((ICheckbox)
		{
			.Checked = 1,

			.CheckYes = (color_argb_t){ 0xFF00FF00 },
			.CheckNo = (color_argb_t){ 0xFFFF0000 },
			.Background = (color_argb_t){ 0xFFFFFFFF }
		})
	);

	IHandle t3 = ICreateText(
		&((IElement)
		{
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,

			.Opacity = 0xFF,

			.AlignX = UI_ALIGN_LEFT,
			.AlignY = UI_ALIGN_MIDDLE
		}),
		NULL,
		&((IText)
		{
			.Str = "checkbox test",

			.FontSize = 24,
			.AlignX = UI_ALIGN_CENTER,

			.Stroke = (color_argb_t){ 0xFFFFFFFF },
			.Fill = (color_argb_t){ 0xFF000000 }
		})
	);

	IHandle b1 = ICreateText(
		&((IElement)
		{
			.w = 150.0f,

			.MarginTop = 10.0f,
			.MarginLeft = 10.0f,
			.MarginRight = 10.0f,
			.MarginBottom = 10.0f,

			.BorderTop = 5.0f,
			.BorderLeft = 5.0f,
			.BorderRight = 5.0f,
			.BorderBottom = 5.0f,

			.Opacity = 0xFF,
			.BorderColor = (color_argb_t){ 0xFF4C00FF },

			.AlignX = UI_ALIGN_CENTER,
			.AlignY = UI_ALIGN_TOP,
			.pos = UI_POSITION_RELATIVE,
			.RelativeAlignX = UI_ALIGN_CENTER,
			.RelativeAlignY = UI_ALIGN_TOP,

			.Relative = &d1
		}),
		&((IContainer)
		{
			.Axis = UI_AXIS_HORIZONTAL,

			.AutoH = true,
			.Scrollable = true,

			.color = (color_argb_t){ 0xFFFFFFFF },
			.tex = TEXTURE_RECT,
			.ScrollbarColor = (color_argb_t){ 0xFFAAAAAA },
			.ScrollbarAltColor = (color_argb_t){ 0xFF717171 },
		}),
		&((IText)
		{
			.Str = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()_+-=[]{}\\|;:'\",.<>/?`~",
			.Placeholder = "Your nickname",

			.FontSize = 32,
			.AlignX = UI_ALIGN_CENTER,

			.Selectable = true,
			.Editable = true,

			.Stroke = (color_argb_t){ 0xFF000000 },
			.InverseStroke = (color_argb_t){ 0xFFFFFFFF },
			.Fill = (color_argb_t){ 0xFFFFFFFF },
			.InverseFill = (color_argb_t){ 0xFF000000 },
			.Background = (color_argb_t){ 0xA0000000 },

			.type = I_TEXT_TYPE_SINGLELINE_TEXT
		})
	);


	IAddElementLast(&m1, &IWindow);

	IAddElementLast(&t1, &m1);
	// IAddElementLast(&t2, &m1);

	// IAddElementLast(&s1, &m1);

	// IAddElementLast(&p1, &m1);
	// IAddElementLast(&p2, &m1);

	// IAddElementLast(&d1, &m1);
	// IAddElementLast(&d2, &m1);

	// IAddElementAfter(&m2, &d2);
	// IAddElementLast(&c1, &m2);
	// IAddElementLast(&t3, &m2);

	// IAddElementAfter(&b1, &m2);
*/
	//UIActivate();
}


int
main(
	void
	)
{
	CreateUI();

	window_init();

	//SocketInit();

	GameInit();

	window_run();

	//SocketFree();

	GameFree();

	window_free();

	UIFree();

	return 0;
}
