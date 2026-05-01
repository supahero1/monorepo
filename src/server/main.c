#include <math.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <../include/io.h>
#include <../include/rand.h>
#include <../include/sort.h>
#include <../include/debug.h>
#include <../include/shared.h>
#include <../include/quadtree.h>


typedef struct GameClient
{
	int FD;
	uint32_t next;

	uint64_t ConnectionIdle;
	uint64_t ActionIdle;

	float CameraX;
	float CameraY;
	float CameraW;
	float CameraH;
	float FoV;

	float Vertical;
	float Horizontal;

	float MovementVX;
	float MovementVY;

	uint32_t BodyIndex;

	uint8_t buffer[GAME_CONST_CLIENT_PACKET_SIZE];
	uint32_t BufferUsed;

	uint8_t Valid:1;

	uint8_t* EntityBits;
	uint16_t* EntitiesInView;
	uint16_t EntitiesInViewCount;
}
GameClient;


GameClient Clients[GAME_CONST_MAX_PLAYERS];
uint32_t ClientsUsed = 0;
uint32_t FreeClient = -1;

uint8_t* EntityBits;
uint16_t* EntitiesInView;
uint16_t EntitiesInViewCount;

GameClient* Client = NULL;
uint64_t CurrentTick = 0;
uint64_t LastTickAt;
uint64_t CurrentTickAt;

QUADTREE Quadtree = {0};

typedef struct GameEntity
{
	EntityType type;
	uint32_t Subtype;

	float VX;
	float VY;

	float ColVX;
	float ColVY;

	float Angle;
	float AngleDir;

	uint32_t HP;
	uint32_t MaxHP;

	uint64_t DamagedAt;

	uint32_t ConstrainToArena:1;
	uint32_t ResetPX:1;
	uint32_t ResetNX:1;
	uint32_t ResetPY:1;
	uint32_t ResetNY:1;

	uint32_t UpdateHP:1;
	uint32_t TookDamage:1;
}
GameEntity;

GameEntity* Entities = NULL;


GameEntity*
GetEntity(
	const QUADTREE_ENTITY* Entity
	)
{
	uint32_t idx = QuadtreeInsert(&Quadtree, Entity);
	assert_eq(Quadtree.EntitiesSize, GAME_CONST_MAX_ENTITIES);

	GameEntity* Ret = Entities + idx;
	*Ret = (GameEntity){ 0 };

	return Ret;
}


void
RetEntity(
	const GameEntity* Entity
	)
{
	QuadtreeRemove(&Quadtree, Entity - Entities);
}


GameClient*
GetClient(
	void
	)
{
	GameClient* Ret = Clients;

	if(FreeClient != -1)
	{
		Ret += FreeClient;
		FreeClient = Ret->next;
	}
	else
	{
		Ret += ClientsUsed++;
	}

	*Ret = (GameClient){ 0 };
	Ret->Valid = 1;

	return Ret;
}


void
RetClient(
	void
	)
{
	Client->next = FreeClient;
	Client->Valid = 0;
	FreeClient = Client - Clients;
}


float
LerpF(
	float old,
	float New,
	float By
	)
{
	return old + (New - old) * By;
}


void
GetSpawnCoords(
	QUADTREE_ENTITY* QTEntity
	)
{
	QTEntity->x = rand_f32() * (GAME_CONST_HALF_ARENA_SIZE * 2 - QTEntity->w) - GAME_CONST_HALF_ARENA_SIZE + QTEntity->w;
	QTEntity->y = rand_f32() * (GAME_CONST_HALF_ARENA_SIZE * 2 - QTEntity->h) - GAME_CONST_HALF_ARENA_SIZE + QTEntity->h;
}


void
SpawnShape(
	Shape Subtype
	)
{
	QUADTREE_ENTITY QTEntity =
	{
		.w = ShapeHitbox[Subtype],
		.h = ShapeHitbox[Subtype]
	};

	GetSpawnCoords(&QTEntity);

	GameEntity* Entity = GetEntity(&QTEntity);

	Entity->type = ENTITY_TYPE_SHAPE;
	Entity->Subtype = Subtype;
	Entity->Angle = rand_angle();
	Entity->AngleDir = rand_bool() ? -1 : 1;
	Entity->MaxHP = ShapeMaxHP[Subtype];
	Entity->HP = MACRO_MIN(Entity->MaxHP, rand_f32() * Entity->MaxHP * 6);
	Entity->ConstrainToArena = 1;
}


int
QuadtreeUpdateFN(
	QUADTREE* Quadtree,
	uint32_t EntityIdx
	)
{
	QUADTREE_ENTITY* QTEntity = Quadtree->Entities + EntityIdx;
	GameEntity* Entity = Entities + EntityIdx;

	switch(Entity->type)
	{

	case ENTITY_TYPE_TANK:
	{
		break;
	}

	case ENTITY_TYPE_SHAPE:
	{
		Entity->VX = cosf(Entity->Angle) * 0.3f;
		Entity->VY = sinf(Entity->Angle) * 0.3f;
		Entity->Angle += 0.0005f * Entity->AngleDir;

		break;
	}

	default: __builtin_unreachable();

	}

	QTEntity->x += Entity->ColVX;
	QTEntity->y += Entity->ColVY;

	Entity->ColVX = LerpF(Entity->ColVX, 0, 0.095f);
	Entity->ColVY = LerpF(Entity->ColVY, 0, 0.095f);

	QTEntity->x += Entity->VX;
	QTEntity->y += Entity->VY;

	if(Entity->ConstrainToArena)
	{
		float Limit = GAME_CONST_HALF_ARENA_SIZE + GAME_CONST_BORDER_PADDING;

		if(QTEntity->x - QTEntity->w < -Limit)
		{
			QTEntity->x = -Limit + QTEntity->w;
			Entity->ResetNX = 1;
		}
		else if(QTEntity->x + QTEntity->w > Limit)
		{
			QTEntity->x = Limit - QTEntity->w;
			Entity->ResetPX = 1;
		}
		else
		{
			Entity->ResetPX = 0;
			Entity->ResetNX = 0;
		}

		if(QTEntity->y - QTEntity->h < -Limit)
		{
			QTEntity->y = -Limit + QTEntity->h;
			Entity->ResetNY = 1;
		}
		else if(QTEntity->y + QTEntity->h > Limit)
		{
			QTEntity->y = Limit - QTEntity->h;
			Entity->ResetPY = 1;
		}
		else
		{
			Entity->ResetPY = 0;
			Entity->ResetNY = 0;
		}
	}

	Entity->UpdateHP = 0;
	Entity->TookDamage = 0;

	if(Entity->HP != Entity->MaxHP && CurrentTick - Entity->DamagedAt >= 200)
	{
		Entity->HP = MACRO_MIN(Entity->MaxHP, Entity->HP + 1);
		Entity->UpdateHP = 1;
	}

	return 1;
}


int
QuadtreeIsCollidingFN(
	const QUADTREE* Quadtree,
	uint32_t EntityIdxA,
	uint32_t EntityIdxB
	)
{
	QUADTREE_ENTITY* QTEntityA = Quadtree->Entities + EntityIdxA;
	QUADTREE_ENTITY* QTEntityB = Quadtree->Entities + EntityIdxB;

	float DiffX = QTEntityA->x - QTEntityB->x;
	float DiffY = QTEntityA->y - QTEntityB->y;
	float SumR = QTEntityA->r + QTEntityB->r;

	return DiffX * DiffX + DiffY * DiffY < SumR * SumR;
}


void
QuadtreeCollideFN(
	QUADTREE* Quadtree,
	uint32_t EntityIdxA,
	uint32_t EntityIdxB
	)
{
	QUADTREE_ENTITY* QTEntityA = Quadtree->Entities + EntityIdxA;
	GameEntity* EntityA = Entities + EntityIdxA;

	QUADTREE_ENTITY* QTEntityB = Quadtree->Entities + EntityIdxB;
	GameEntity* EntityB = Entities + EntityIdxB;

	float DiffX = QTEntityA->x - QTEntityB->x;
	float DiffY = QTEntityA->y - QTEntityB->y;

	float Dist = sqrtf(DiffX * DiffX + DiffY * DiffY);

	if(!Dist)
	{
		Dist = 1.0f;
	}

	DiffX /= Dist;
	DiffY /= Dist;

	EntityA->ColVX += DiffX * 5.0f;
	EntityA->ColVY += DiffY * 5.0f;

	EntityB->ColVX -= DiffX * 5.0f;
	EntityB->ColVY -= DiffY * 5.0f;

	if(EntityA->HP)
	{
		--EntityA->HP;
	}
	EntityA->UpdateHP = 1;
	EntityA->TookDamage = 1;
	EntityA->DamagedAt = CurrentTick;

	if(EntityB->HP)
	{
		--EntityB->HP;
	}
	EntityB->UpdateHP = 1;
	EntityB->TookDamage = 1;
	EntityB->DamagedAt = CurrentTick;
}


void
QuadtreeViewQueryFN(
	QUADTREE* Quadtree,
	uint32_t EntityIdx
	)
{
	EntityBits[EntityIdx >> 3] |= 1 << (EntityIdx & 7);
	EntitiesInView[EntitiesInViewCount++] = EntityIdx;
}


void
ClientChangeFoV(
	float FoV
	)
{
	Client->FoV = FoV;
	Client->CameraW = 960.0f / Client->FoV + 200.0f;
	Client->CameraH = 540.0f / Client->FoV + 200.0f;
}


void
ClientCreate(
	void
	)
{
	Client->ConnectionIdle = CurrentTick;
	Client->ActionIdle = CurrentTick;

	ClientChangeFoV(0.5f);

	GameEntity* Body = GetEntity(&(
		(QUADTREE_ENTITY)
		{
			.x = 0,
			.y = 0,
			.w = 70,
			.h = 70
		}
	));
	Body->ConstrainToArena = 1;
	Body->MaxHP = 1000;
	Body->HP = Body->MaxHP;

	Client->BodyIndex = Body - Entities;

	Client->EntityBits = calloc(MACRO_TO_BYTES(GAME_CONST_MAX_ENTITIES), sizeof(uint8_t));
	assert_not_null(Client->EntityBits);
}


void
ClientClose(
	void
	)
{
	shutdown(Client->FD, SHUT_RDWR);
}


void
ClientSend(
	const bit_buffer_t* buffer
	)
{
	ssize_t bytes = send(Client->FD, buffer->buffer, buffer->len, MSG_NOSIGNAL);

	if(bytes != buffer->len)
	{
		ClientClose();
	}
}


uint32_t
ClientRead(
	void
	)
{
	bit_buffer_t buffer = {0};
	buffer.buffer = Client->buffer;
	buffer.at = buffer.buffer;
	buffer.len = Client->BufferUsed;

	if(buffer.len < MACRO_TO_BYTES(CLIENT_OPCODE__BITS))
	{
		return 0;
	}

	uintptr_t OpCode = bit_buffer_get_bits(&buffer, CLIENT_OPCODE__BITS);

	switch(OpCode)
	{

	case CLIENT_OPCODE_INPUT:
	{
		if(buffer.len < MACRO_TO_BYTES(
			CLIENT_OPCODE__BITS +
			FIELD_SIZE_MOUSE_X +
			FIELD_SIZE_MOUSE_Y +
			WINDOW_KEY_BUTTON__BITS))
		{
			break;
		}

		uintptr_t MouseX = bit_buffer_get_bits(&buffer, FIELD_SIZE_MOUSE_X);
		if(MouseX > 1920)
		{
			ClientClose();
			break;
		}

		uintptr_t MouseY = bit_buffer_get_bits(&buffer, FIELD_SIZE_MOUSE_Y);
		if(MouseY > 1080)
		{
			ClientClose();
			break;
		}

		uintptr_t Keys = bit_buffer_get_bits(&buffer, WINDOW_KEY_BUTTON__BITS);

		float Vertical = 0;
		float Horizontal = 0;

		if(Keys & SHIFT(WINDOW_KEY_BUTTON_W))
		{
			--Vertical;
		}

		if(Keys & SHIFT(WINDOW_KEY_BUTTON_A))
		{
			--Horizontal;
		}

		if(Keys & SHIFT(WINDOW_KEY_BUTTON_S))
		{
			++Vertical;
		}

		if(Keys & SHIFT(WINDOW_KEY_BUTTON_D))
		{
			++Horizontal;
		}

		if(Vertical || Horizontal)
		{
			float Dist = 1 / sqrtf(Vertical * Vertical + Horizontal * Horizontal);

			Vertical *= Dist;
			Horizontal *= Dist;
		}

		Client->Vertical = Vertical * GAME_CONST_MAX_MOVEMENT_SPEED;
		Client->Horizontal = Horizontal * GAME_CONST_MAX_MOVEMENT_SPEED;

		return BitBufferConsumed(&buffer);
	}

	default:
	{
		ClientClose();

		break;
	}

	}

	return 0;
}


void
ClientDestroy(
	void
	)
{
	RetEntity(Entities + Client->BodyIndex);
	free(Client->EntityBits);
}


void
GameUpdate(
	void
	)
{
	Client = Clients;
	GameClient* ClientEnd = Clients + ClientsUsed;

	for(; Client != ClientEnd; ++Client)
	{
		if(!Client->Valid)
		{
			continue;
		}

		QUADTREE_ENTITY* QTEntity = Quadtree.Entities + Client->BodyIndex;
		GameEntity* Entity = Entities + Client->BodyIndex;

		Client->MovementVX = LerpF(Client->MovementVX, Client->Horizontal, 0.095f);
		Client->MovementVY = LerpF(Client->MovementVY, Client->Vertical, 0.095f);

		if((Entity->ResetPX && Client->MovementVX > 0) || (Entity->ResetNX && Client->MovementVX < 0))
		{
			Client->MovementVX = 0;
		}

		if((Entity->ResetPY && Client->MovementVY > 0) || (Entity->ResetNY && Client->MovementVY < 0))
		{
			Client->MovementVY = 0;
		}

		QTEntity->x += Client->MovementVX;
		QTEntity->y += Client->MovementVY;
	}

	QuadtreeUpdate(&Quadtree);
	QuadtreeCollide(&Quadtree);

	Client = Clients;
	Quadtree.Query = QuadtreeViewQueryFN;

	for(; Client != ClientEnd; ++Client)
	{
		if(!Client->Valid)
		{
			continue;
		}

		QUADTREE_ENTITY* QTEntity = Quadtree.Entities + Client->BodyIndex;

		Client->CameraX = QTEntity->x;
		Client->CameraY = QTEntity->y;

		bit_buffer_t buffer = {0};
		buffer.len = GAME_CONST_SERVER_PACKET_SIZE;
		buffer.buffer = calloc(buffer.len, sizeof(uint8_t));
		assert_not_null(buffer.buffer);
		buffer.at = buffer.buffer;

		bit_buffer_set_bits(&buffer, SERVER_OPCODE_UPDATE, SERVER_OPCODE__BITS);

		bit_buffer_ctx_t PacketLength = bit_buffer_save(&buffer);
		bit_buffer_skip_bits(&buffer, GAME_CONST_SERVER_PACKET_SIZE__BITS);

		bit_buffer_set_bits(&buffer, (CurrentTickAt - LastTickAt) / 10000, FIELD_SIZE_TICK_DURATION);

		bit_buffer_set_fixed_point(&buffer, Client->FoV, FIXED_POINT(FOV));
		bit_buffer_set_signed_fixed_point(&buffer, Client->CameraX, FIXED_POINT(POS));
		bit_buffer_set_signed_fixed_point(&buffer, Client->CameraY, FIXED_POINT(POS));

		bit_buffer_ctx_t EntitiesCount = bit_buffer_save(&buffer);
		bit_buffer_skip_bits(&buffer, GAME_CONST_MAX_ENTITIES__BITS);

		EntityBits = calloc(MACRO_TO_BYTES(GAME_CONST_MAX_ENTITIES), sizeof(uint8_t));
		assert_not_null(EntityBits);

		EntitiesInViewCount = 0;

		QuadtreeQuery(&Quadtree, Client->CameraX, Client->CameraY, Client->CameraW, Client->CameraH);

		uint16_t* NewEntitiesInView = malloc(sizeof(*NewEntitiesInView) * EntitiesInViewCount);
		assert_not_null(NewEntitiesInView);

		(void) memcpy(NewEntitiesInView, EntitiesInView, sizeof(*EntitiesInView) * EntitiesInViewCount);

		uint32_t NewEntitiesInViewCount = EntitiesInViewCount;

		uint16_t* EntityInView = Client->EntitiesInView;
		uint16_t* EntityInViewEnd = EntityInView + Client->EntitiesInViewCount;

		while(EntityInView != EntityInViewEnd)
		{
			if(!(EntityBits[*EntityInView >> 3] & (1 << (*EntityInView & 7))))
			{
				EntitiesInView[EntitiesInViewCount++] = *EntityInView;
			}

			++EntityInView;
		}

		free(Client->EntitiesInView);
		Client->EntitiesInView = NewEntitiesInView;
		Client->EntitiesInViewCount = NewEntitiesInViewCount;

		QuickSort(EntitiesInView, EntitiesInViewCount);

		EntityInView = EntitiesInView;
		EntityInViewEnd = EntityInView + EntitiesInViewCount;

		while(EntityInView != EntityInViewEnd)
		{
			uint8_t OldSet = !!(Client->EntityBits[*EntityInView >> 3] & (1 << (*EntityInView & 7)));
			uint8_t NewSet = !!(        EntityBits[*EntityInView >> 3] & (1 << (*EntityInView & 7)));

			bit_buffer_set_bits(&buffer, OldSet, 1);
			bit_buffer_set_bits(&buffer, NewSet, 1);

			QUADTREE_ENTITY* QTEntity = Quadtree.Entities + *EntityInView;
			GameEntity* Entity = Entities + *EntityInView;

			if(!OldSet)
			{
				/* Creation */

				assert_eq(!!NewSet, 1);

				bit_buffer_set_bits(&buffer, Entity->type, ENTITY_TYPE__BITS);
				bit_buffer_set_bits(&buffer, Entity->Subtype, TypeToSubtypeBits[Entity->type]);

				bit_buffer_set_signed_fixed_point(&buffer, (QTEntity->x - Client->CameraX) * Client->FoV, FIXED_POINT(SCREEN_POS));
				bit_buffer_set_signed_fixed_point(&buffer, (QTEntity->y - Client->CameraY) * Client->FoV, FIXED_POINT(SCREEN_POS));


				switch(Entity->type)
				{

				case ENTITY_TYPE_TANK:
				{
					bit_buffer_set_fixed_point(&buffer, QTEntity->r, FIXED_POINT(RADIUS));

					break;
				}

				case ENTITY_TYPE_SHAPE:
				{
					break;
				}

				default: __builtin_unreachable();

				}


				switch(Entity->type)
				{

				case ENTITY_TYPE_TANK:
				case ENTITY_TYPE_SHAPE:
				{
					int WriteHP = Entity->HP != Entity->MaxHP;
					bit_buffer_set_bits(&buffer, WriteHP, 1);

					bit_buffer_set_bits(&buffer, Entity->TookDamage, 1);

					if(WriteHP)
					{
						bit_buffer_set_bits(&buffer, Entity->HP, ShapeHPBits[Entity->Subtype]);
					}

					break;
				}

				default: __builtin_unreachable();

				}
			}
			else if(NewSet)
			{
				/* Update */

				switch(Entity->type)
				{

				case ENTITY_TYPE_TANK:
				case ENTITY_TYPE_SHAPE:
				{
					bit_buffer_set_signed_fixed_point(&buffer, (QTEntity->x - Client->CameraX) * Client->FoV, FIXED_POINT(SCREEN_POS));
					bit_buffer_set_signed_fixed_point(&buffer, (QTEntity->y - Client->CameraY) * Client->FoV, FIXED_POINT(SCREEN_POS));

					bit_buffer_set_bits(&buffer, Entity->UpdateHP, 1);
					bit_buffer_set_bits(&buffer, Entity->TookDamage, 1);

					break;
				}

				default: __builtin_unreachable();

				}


				switch(Entity->type)
				{

				case ENTITY_TYPE_TANK:
				case ENTITY_TYPE_SHAPE:
				{
					if(Entity->UpdateHP)
					{
						bit_buffer_set_bits(&buffer, Entity->HP, ShapeHPBits[Entity->Subtype]);
					}

					break;
				}

				default: __builtin_unreachable();

				}
			}
			else
			{
				/* Removal */
			}

			++EntityInView;
		}

		free(Client->EntityBits);
		Client->EntityBits = EntityBits;

		buffer.len = BitBufferConsumed(&buffer);

		bit_buffer_restore(&buffer, &EntitiesCount);
		bit_buffer_set_bits(&buffer, EntitiesInViewCount, GAME_CONST_MAX_ENTITIES__BITS);

		bit_buffer_restore(&buffer, &PacketLength);
		bit_buffer_set_bits(&buffer, buffer.len, GAME_CONST_SERVER_PACKET_SIZE__BITS);

		ClientSend(&buffer);
		free(buffer.buffer);
	}
}


int
main(
	int argc,
	char** argv
	)
{
	int Error;

	Entities = calloc(GAME_CONST_MAX_ENTITIES, sizeof(*Entities));
	assert_not_null(Entities);

	Quadtree.x = 0;
	Quadtree.y = 0;
	Quadtree.w = GAME_CONST_HALF_ARENA_SIZE + GAME_CONST_HALF_ARENA_CLEAR_ZONE;
	Quadtree.h = GAME_CONST_HALF_ARENA_SIZE + GAME_CONST_HALF_ARENA_CLEAR_ZONE;
	Quadtree.MinSize = GAME_CONST_MIN_QUADTREE_NODE_SIZE;
	Quadtree.Update = QuadtreeUpdateFN;
	Quadtree.IsColliding = QuadtreeIsCollidingFN;
	Quadtree.Collide = QuadtreeCollideFN;
	Quadtree.EntitiesSize = GAME_CONST_MAX_ENTITIES;
	QuadtreeInit(&Quadtree);

	Entities = calloc(Quadtree.EntitiesSize, sizeof(*Entities));
	assert_not_null(Entities);

	for(int i = 0; i < 600; ++i)
	{
		SpawnShape(SHAPE_SQUARE);
	}

	for(int i = 0; i < 300; ++i)
	{
		SpawnShape(SHAPE_TRIANGLE);
	}

	for(int i = 0; i < 100; ++i)
	{
		SpawnShape(SHAPE_PENTAGON);
	}

	EntitiesInView = malloc(sizeof(*EntitiesInView) * GAME_CONST_MAX_ENTITIES);
	assert_not_null(EntitiesInView);

	int EpollFD = epoll_create1(0);
	assert_neq(EpollFD, -1);

	struct epoll_event Events[GAME_CONST_MAX_PLAYERS];

	int ServerFD = socket(AF_INET6, SOCK_STREAM | SOCK_NONBLOCK, 0);
	assert_neq(ServerFD, -1);

	struct sockaddr_in6 Addr = {0};
	Addr.sin6_family = AF_INET6;
	Addr.sin6_addr = in6addr_any;
	Addr.sin6_port = htons(GAME_CONST_PORT);

	int ReceiveBufferSize = GAME_CONST_SERVER_RECV_SIZE;
	Error = setsockopt(ServerFD, SOL_SOCKET, SO_RCVBUF, &ReceiveBufferSize, sizeof(ReceiveBufferSize));
	assert_neq(Error, -1);

	int True = 1;
	Error = setsockopt(ServerFD, SOL_SOCKET, SO_REUSEADDR, &True, sizeof(True));
	assert_neq(Error, -1);

	Error = setsockopt(ServerFD, SOL_SOCKET, SO_REUSEPORT, &True, sizeof(True));
	assert_neq(Error, -1);

	Error = bind(ServerFD, (struct sockaddr*) &Addr, sizeof(Addr));
	assert_neq(Error, -1);

	Error = listen(ServerFD, 64);
	assert_neq(Error, -1);

	struct timespec Wait;
	(void) clock_gettime(CLOCK_REALTIME, &Wait);

	LastTickAt = Wait.tv_nsec + Wait.tv_sec * 1000000000 - GAME_CONST_TICK_RATE_MS * 1000000;

	while(1)
	{
		++CurrentTick;

		struct timespec time = {0};
		clock_gettime(CLOCK_REALTIME, &time);
		CurrentTickAt = time.tv_nsec + time.tv_sec * 1000000000;

		int count = epoll_wait(EpollFD, Events, GAME_CONST_MAX_PLAYERS, 0);

		struct epoll_event* Event = Events;
		struct epoll_event* EventEnd = Event + count;

		for(; Event != EventEnd; ++Event)
		{
			uint32_t flags = Event->events;
			Client = Event->data.ptr;

			if(flags & (EPOLLHUP | EPOLLERR | EPOLLRDHUP))
			{
				ClientDestroy();

				close(Client->FD);
				RetClient();

				continue;
			}

			if(flags & EPOLLIN)
			{
				ssize_t bytes = read(Client->FD, Client->buffer +
					Client->BufferUsed, MACRO_ARRAY_LEN(Client->buffer) - Client->BufferUsed);

				if(bytes >= 0)
				{
					Client->BufferUsed += bytes;

					while(Client->BufferUsed)
					{
						uint32_t Read = ClientRead();

						if(Read == 0)
						{
							break;
						}

						Client->BufferUsed -= Read;

						(void) memmove(Client->buffer, Client->buffer + Read, Client->BufferUsed);
					}
				}
			}
		}

		while(ClientsUsed != GAME_CONST_MAX_PLAYERS || FreeClient != -1)
		{
			struct sockaddr_in6 Addr;
			socklen_t AddrLen = sizeof(Addr);
			int SocketFD = accept(ServerFD, (struct sockaddr*) &Addr, &AddrLen);

			if(SocketFD == -1)
			{
				assert_neq(errno, ENOMEM);
				assert_neq(errno, ENFILE);
				assert_neq(errno, EMFILE);
				assert_neq(errno, EINVAL);

				if(errno == EAGAIN)
				{
					break;
				}

				continue;
			}

			Client = GetClient();
			Client->FD = SocketFD;

			ClientCreate();

			(void) fcntl(SocketFD, F_SETFL, fcntl(SocketFD, F_GETFL, 0) | O_NONBLOCK);

			struct epoll_event Event =
			{
				.events = EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLRDHUP,
				.data =
				{
					.ptr = Client
				}
			};
			epoll_ctl(EpollFD, EPOLL_CTL_ADD, SocketFD, &Event);
		}

		GameUpdate();

		LastTickAt = CurrentTickAt;

		Wait.tv_nsec += GAME_CONST_TICK_RATE_MS * 1000000;
		if(Wait.tv_nsec >= 1000000000)
		{
			Wait.tv_nsec -= 1000000000;
			++Wait.tv_sec;
		}

		Error = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &Wait, NULL);
		assert_neq(Error, -1);
	}

	return 0;
}
