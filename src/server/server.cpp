#include "server.h"
#include <iostream>
#include "../message.h"

using namespace cppnat;

#define LOGINFO(s)                                \
	do                                            \
	{                                             \
		std::cout << "[INFO] " << s << std::endl; \
	} while (0)

#define LOGERROR(s)                                \
	do                                             \
	{                                              \
		std::cout << "[ERROR] " << s << std::endl; \
	} while (0)

Server::Server(const char *addr, int port) : stop(false)
{
#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
	this->addr.sin_family = AF_INET;
	this->addr.sin_port = htons(port);
	this->addr.sin_addr.s_addr = inet_addr(addr);
	this->addr.sin_zero[0] = 0;
}

Server::~Server()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

#ifdef _WIN32
typedef int socklen_t;
#else
#endif

bool Server::Listen()
{
	this->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->fd == INVALID_SOCKET)
		return false;
	if (bind(this->fd, (sockaddr *)&this->addr, sizeof(this->addr)) == SOCKET_ERROR)
		return false;
	if (listen(this->fd, 10) == SOCKET_ERROR)
		return false;
	return true;
}

int Server::Errno()
{
#ifdef _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
}

inline void PutSocketList(SOCKET socketList[], size_t socketListSize, SOCKET fd)
{
	for (size_t i = 0; i < socketListSize; i++)
	{
		if (socketList[i] == INVALID_SOCKET)
		{
			socketList[i] = fd;
			return;
		}
	}
}

inline void RemoveSocketList(SOCKET socketList[], size_t socketListSize, SOCKET fd)
{
	for (size_t i = 0; i < socketListSize; i++)
	{
		if (socketList[i] == fd)
		{
			socketList[i] = INVALID_SOCKET;
			return;
		}
	}
}

inline void InitSocketList(SOCKET socketList[], size_t socketListSize)
{
	for (size_t i = 0; i < socketListSize; i++)
		socketList[i] = INVALID_SOCKET;
}

inline bool AcceptAsClient(const char *buffer, int recvSize)
{
	if (recvSize < sizeof(VersionInfo))
		return false;
	VersionInfo *info = (VersionInfo *)buffer;
	return (info->Version == versionInfo.Version && strcmp(info->Msg, versionInfo.Msg) == 0);
}

inline bool RequestNewNat(SOCKET fd) {}

bool Server::Begin()
{
	fd_set fdSet, rlist;
	FD_ZERO(&fdSet);
	FD_SET(this->fd, &fdSet);

	SOCKET maxSocket = this->fd;
	SOCKET uniqueClientSocket = INVALID_SOCKET;
	constexpr size_t socketNum = 128;
	SOCKET socketList[socketNum];
	InitSocketList(socketList, socketNum);

	sockaddr_in tempSockAddr;
	socklen_t tempSockLen;
	SOCKET tempFd = INVALID_SOCKET;

	char buffer[bufferSize];

	timeval timeout{1, 0};
	int count, recvSize;

	for (;;)
	{
		if (this->stop)
		{
			//@todo recycle socket
			break;
		}

		rlist = fdSet;
		count = select(maxSocket + 1, &rlist, nullptr, nullptr, &timeout);
		if (count == SOCKET_ERROR)
			return false;
		if (count == 0)
			continue;
		if (FD_ISSET(this->fd, &fdSet))
		{
			count--;
			tempFd = accept(this->fd, (sockaddr *)&tempSockAddr, &tempSockLen);
			if (tempFd == INVALID_SOCKET)
				return false;
			do
			{
				if (uniqueClientSocket != INVALID_SOCKET)
				{
					if (!RequestNewNat(tempFd))
					{
						closesocket(tempFd);
						break;
					}
				}
				FD_SET(tempFd, &fdSet);
				maxSocket = tempFd > maxSocket ? tempFd : maxSocket;
				PutSocketList(socketList, socketNum, fd);
			} while (0);
		}
		if (uniqueClientSocket != INVALID_SOCKET)
		{
			for (int i = 0; i < socketNum; i++)
			{
				if (count == 0)
					break;
			}
		}
		else
		{
			for (int i = 0; i < socketNum; i++)
			{
				if (count == 0)
					break;
				tempFd = socketList[i];
				if (tempFd == INVALID_SOCKET)
					continue;
				if (FD_ISSET(tempFd, &rlist))
				{
					recvSize = recv(tempFd, buffer, bufferSize, 0);
					if (recvSize == SOCKET_ERROR)
					{
						FD_CLR(tempFd, &fdSet);
						RemoveSocketList(socketList, socketNum, tempFd);
					}
					if (AcceptAsClient(buffer, recvSize))
					{
						uniqueClientSocket = tempFd;
						RemoveSocketList(socketList, socketNum, tempFd);
						unsigned short code = GetNumber(MsgCode::SUCCESS);
						int sendSize = send(tempFd, (char *)&code, sizeof(code), 0);
						if (sendSize == SOCKET_ERROR)
						{
							FD_CLR(tempFd, &fdSet);
							RemoveSocketList(socketList, socketNum, tempFd);
						}
					}
					else
					{
						closesocket(tempFd);
						FD_CLR(tempFd, &fdSet);
						RemoveSocketList(socketList, socketNum, tempFd);
					}
				}
			}
		}
	}
}

void Server::Stop() { this->stop = true; }
