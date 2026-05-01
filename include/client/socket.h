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

#include <stdint.h>
#include <threads.h>
#include <winsock2.h>
#include <stdatomic.h>
#include <sys/socket.h>


typedef void
(*SocketCallback)(
	void
	);


typedef uint32_t
(*SocketReadCallback)(
	uint8_t* data,
	uint32_t len
	);


typedef struct TcpSocket
{
	_Atomic SocketID id;
	const char* Host;
	uint8_t* buffer;
	SocketCallback Open;
	SocketReadCallback Read;
	SocketCallback Close;
	ThreadID thread;
	uint16_t Port;
	uint16_t Secure;
	uint32_t BufferUsed;
	uint32_t BufferSize;
}
TcpSocket;


extern void
SocketInit(
	void* Socket
	);


extern void
SocketFree(
	void* Socket
	);


extern void
SocketSendData(
	void* Socket,
	const void* data,
	uint32_t len
	);


extern void
SocketSendInputPacket(
	TcpSocket* Socket,
	uint32_t MouseX,
	uint32_t MouseY,
	uint32_t Keys
	);
