#include "server.h"
using namespace cppnat;

constexpr int BufferSize = 65536;

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
	if (listen(this->fd, 40) == SOCKET_ERROR)
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

struct ConnPair
{
	ConnPair(int c1 = INVALID_SOCKET, int c2 = INVALID_SOCKET) : c1(c1), c2(c2)
	{
		memset(&s1, 0, sizeof(s1));
		memset(&s2, 0, sizeof(s2));
	}
	SOCKET c1;
	sockaddr_in s1;
	SOCKET c2;
	sockaddr_in s2;
};

inline void Handle(SOCKET &fd, SOCKET &another, FD_SET *pfdset)
{
	static char buf[BufferSize];
	int len = recv(fd, buf, BufferSize, 0);
	if (len == 0)
	{
	}
}

void Server::Begin()
{
	FD_SET fdset;
	FD_ZERO(&fdset);
	FD_SET rlist;
	FD_ZERO(&rlist);
	FD_SET(this->fd, &fdset);
	ConnPair connPair;
	socklen_t len1 = sizeof(connPair.s1);
	socklen_t len2 = sizeof(connPair.s2);
	SOCKET maxSocket = this->fd;
	int count;
	timeval timeout{1, 0};

	for (;;)
	{
		if (this->stop)
			break;
		count = select(maxSocket + 1, &rlist, NULL, NULL, NULL);
		if (count == SOCKET_ERROR)
			break;
		if (FD_ISSET(this->fd, &rlist))
		{
			count--;
			if (connPair.c1 == INVALID_SOCKET)
			{
				connPair.c1 = accept(this->fd, (sockaddr *)&connPair.s1, &len1);
				if (connPair.c1 == INVALID_SOCKET)
					break;
				FD_SET(connPair.c1, &fdset);
				if (connPair.c1 > maxSocket)
					maxSocket = connPair.c1;
			}
			else if (connPair.c2 == INVALID_SOCKET)
			{
				connPair.c2 = accept(this->fd, (sockaddr *)&connPair.s2, &len2);
				if (connPair.c2 == INVALID_SOCKET)
					break;
				FD_SET(connPair.c2, &fdset);
				if (connPair.c2 > maxSocket)
					maxSocket = connPair.c2;
			}
			else
			{
				static sockaddr_in tempAddr;
				static SOCKET tempFd;
				static socklen_t tempLen;
				tempFd = accept(this->fd, (sockaddr *)&tempAddr, &tempLen);
				closesocket(tempFd);
			}
		}
		while (count > 0)
		{
			count--;
			if (FD_ISSET(connPair.c1, &rlist))
				Handle(connPair.c1, connPair.c2, &fdset);
			else
				Handle(connPair.c2, connPair.c1, &fdset);
		}
	}
}

void Server::Stop() { this->stop = true; }
