#include "server.h"
#include <iostream>

using namespace cppnat;

constexpr int bufferSize = 65536;
constexpr int headerSize = 2;

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

/**
 * @brief a pair contains two sockets
 * @param fd1 default -1
 * @param fd2 default -1
 */
struct ConnPair
{
	ConnPair(int c1 = INVALID_SOCKET, int c2 = INVALID_SOCKET) : c1(c1), c2(c2)
	{
		memset(&s1, 0, sizeof(s1));
		memset(&s2, 0, sizeof(s2));
	}
	/**
	 * @brief call this function when one socket is close;
	 * @param fd socket to be closed
	 */
	void Disconnect(int fd)
	{
		if (fd == c1)
			c1 = INVALID_SOCKET;
		else if (fd == c2)
			c2 = INVALID_SOCKET;
	}
	SOCKET c1;
	sockaddr_in s1;
	SOCKET c2;
	sockaddr_in s2;
};

inline void Handle(SOCKET &fd, SOCKET &another, FD_SET *pfdset, ConnPair &connPair)
{
	static char buf[bufferSize];
	int len = recv(fd, buf, bufferSize, 0);
	if (len == 0)
	{
		connPair.Disconnect(fd);
		FD_CLR(fd, pfdset);
		closesocket(fd);
		/**
		 * @todo:send fd is disconnected to another
		 */
		return;
	}
	send(another, buf, len, 0);
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
		rlist = fdset;
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
				LOGINFO(std::string("accept c1"));
			}
			else if (connPair.c2 == INVALID_SOCKET)
			{
				connPair.c2 = accept(this->fd, (sockaddr *)&connPair.s2, &len2);
				if (connPair.c2 == INVALID_SOCKET)
					break;
				FD_SET(connPair.c2, &fdset);
				if (connPair.c2 > maxSocket)
					maxSocket = connPair.c2;
				LOGINFO(std::string("accept c2"));
			}
			else
			{
				static sockaddr_in tempAddr;
				static SOCKET tempFd;
				static socklen_t tempLen;
				tempFd = accept(this->fd, (sockaddr *)&tempAddr, &tempLen);
				closesocket(tempFd);
				send(tempFd, "forbidden", 9, 0);
				LOGINFO(std::string("invalid client connect"));
			}
		}
		while (count > 0)
		{
			count--;
			if (FD_ISSET(connPair.c1, &rlist))
				Handle(connPair.c1, connPair.c2, &fdset, connPair);
			else
				Handle(connPair.c2, connPair.c1, &fdset, connPair);
		}
	}
}

void Server::Stop() { this->stop = true; }
