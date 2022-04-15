#include "client.h"

using namespace cppnat;
inline SOCKET ListenLocal(unsigned short port)
{
	SOCKET fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd == INVALID_SOCKET)
		return INVALID_SOCKET;
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	addr.sin_zero[0] = 0;
	if (bind(fd, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
		return INVALID_SOCKET;
	if (listen(fd, 10) == SOCKET_ERROR)
		return INVALID_SOCKET;
	return fd;
}

Client::Client(const char *serverAddr, unsigned short serverPort, unsigned short port) : listenPort(port), errorMessage(nullptr)
{
	this->fd = ListenLocal(port);
	if (this->fd == INVALID_SOCKET)
	{
		SetErrorMessage("create client service failed");
		return;
	}
	this->serverAddr.sin_family = AF_INET;
	this->serverAddr.sin_port = htons(serverPort);
	this->serverAddr.sin_addr.s_addr = inet_addr(serverAddr);
	this->serverAddr.sin_zero[0] = 0;
	this->serverFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->serverFd == INVALID_SOCKET)
	{
		SetErrorMessage("create server socket failed");
		return;
	}
	if (connect(this->serverFd, (sockaddr *)&this->serverAddr, sizeof(this->serverAddr)) == SOCKET_ERROR)
	{
		this->serverFd = INVALID_SOCKET;
		SetErrorMessage("connect to server failed");
	}
}

inline void MessageLoop(SOCKET clientFd, SOCKET serverFd)
{
	FD_SET fdset;
	FD_ZERO(&fdset);
	FD_SET rlist;
	FD_ZERO(&rlist);
	FD_SET(clientFd, &fdset);
	FD_SET(serverFd, &fdset);
	int count;
	SOCKET maxFd = clientFd > serverFd ? clientFd : serverFd;

	for (;;)
	{
		rlist = fdset;
		count = select(maxFd + 1, &fdset, nullptr, nullptr, nullptr);
		if (FD_ISSET(clientFd, &fdset))
		{
			count--;
		}
		if (FD_ISSET(serverFd, &fdset))
		{
			count--;
		}
	}
}

bool Client::Begin()
{
	if (this->fd == INVALID_SOCKET)
		return false;
	if (this->serverFd == INVALID_SOCKET)
		return false;
	MessageLoop(this->fd, this->serverFd);
	return true;
}

Client::~Client()
{
	if (this->fd != INVALID_SOCKET)
		closesocket(this->fd);
	if (this->serverFd != INVALID_SOCKET)
		closesocket(this->serverFd);
	if (this->errorMessage != nullptr)
		delete[] this->errorMessage;
}

void Client::SetErrorMessage(const char *msg)
{
	if (this->errorMessage != nullptr)
		delete[] this->errorMessage;
	this->errorMessage = new char[strlen(msg) + 1];
	strcpy(this->errorMessage, msg);
}

const char *Client::Error() { return this->errorMessage; }