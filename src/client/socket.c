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

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ws2tcpip.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <../include/debug.h>
#include <../include/socket.h>

#endif


void
TcpSocketThreadFN(
	void* _Socket
	)
{
	TcpSocket* Socket = _Socket;

	struct sockaddr_in Addr = {0};
	Addr.sin_family = AF_INET;
	inet_pton(AF_INET, Socket->Host, &Addr.sin_addr);
	Addr.sin_port = htons(Socket->Port);

	SocketID id = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	assert_neq(id, -1);

	int Error = setsockopt(id, SOL_SOCKET, SO_RCVBUF, (const char*) &Socket->BufferSize, 4);
	assert_neq(Error, -1);

	Error = connect(id, (struct sockaddr*) &Addr, sizeof(Addr));

	if(Error)
	{
		goto goto_close;
	}

	atomic_store_explicit(&Socket->id, id, memory_order_release);
	Socket->Open();

	while(1)
	{
		int bytes = recv(id, (void*)(Socket->buffer + Socket->BufferUsed), Socket->BufferSize - Socket->BufferUsed, 0);

		if(bytes <= 0)
		{
			goto goto_close;
		}

		Socket->BufferUsed += bytes;

		while(1)
		{
			uint32_t Read = Socket->Read(Socket->buffer, Socket->BufferUsed);

			if(Read == 0)
			{
				break;
			}

			Socket->BufferUsed -= Read;

			if(!Socket->BufferUsed)
			{
				break;
			}

			(void) memmove(Socket->buffer, Socket->buffer + Read, Socket->BufferUsed);
		}
	}

	goto_close:

#ifdef _WIN32
	closesocket(id);
#else
	close(id);
#endif
	Socket->Close();
	ThreadQuit();
}


void
SocketInit(
	void* _Socket
	)
{
	TcpSocket* Socket = _Socket;

	if(Socket->Secure)
	{
		assert_unreachable();
	}

	Socket->BufferUsed = 0;
	Socket->buffer = malloc(Socket->BufferSize);
	assert_not_null(Socket->buffer);

	thread_init(&Socket->thread, TcpSocketThreadFN, Socket);
}


void
SocketFree(
	void* _Socket
	)
{
	TcpSocket* Socket = _Socket;

	if(Socket->Secure)
	{
		assert_unreachable();
	}

	thread_free(Socket->thread);

	free(Socket->buffer);
}


void
SocketSendData(
	void* _Socket,
	const void* data,
	uint32_t len
	)
{
	TcpSocket* Socket = _Socket;

	if(Socket->Secure)
	{
		assert_unreachable();
	}

	int id = atomic_load_explicit(&Socket->id, memory_order_acquire);

	(void) send(id, data, len, 0);
}
