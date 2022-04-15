#include "client.h"
#include <string>
#include <vector>

#ifdef _WIN32
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#endif

using namespace cppnat;

#define CImpl Client::Impl

class Client::Impl
{
public:
	Impl();
	inline void SetServer(const char *serverAddr, unsigned short port);
	inline void SetForward(const char *forwardAddr, unsigned short forwardPort);
	inline bool Start();
	inline const char *Error();
	inline void HandleServerMessage();

protected:
	void SetErrorInfo(const std::string &errorInfo);
	std::string info;
	sockaddr_in serverSockAddr;
	SOCKET serverFd;
	SOCKET dstFd;
	sockaddr_in dstAddr;
	char buffer[65536];
};

CImpl::Impl() : serverFd(INVALID_SOCKET), dstFd(INVALID_SOCKET), info("") {}
const char *CImpl::Error() { return info.c_str(); }
void CImpl::SetForward(const char *addr, unsigned short port)
{
	this->dstAddr.sin_family = AF_INET;
	this->dstAddr.sin_addr.s_addr = inet_addr(addr);
	this->dstAddr.sin_port = htons(port);
}

void CImpl::SetServer(const char *addr, unsigned short port)
{
	this->serverSockAddr.sin_family = AF_INET;
	this->serverSockAddr.sin_addr.s_addr = inet_addr(addr);
	this->serverSockAddr.sin_port = htons(port);
}

bool CImpl::Start()
{
	this->serverFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->serverFd == INVALID_SOCKET)
	{
		this->SetErrorInfo("create server socket failed");
		return false;
	}
	if (connect(this->serverFd, (sockaddr *)&this->serverSockAddr, sizeof(this->serverSockAddr)) == SOCKET_ERROR)
	{
		this->SetErrorInfo("connect to server failed");
		return false;
	}
	fd_set fdSet;
	FD_ZERO(&fdSet);
	FD_SET(this->serverFd, &fdSet);
	fd_set rlist;
	timeval timeout{1, 0};
	int count;
	SOCKET maxSocket = serverFd;
	constexpr int maxSocketNum = 128;
	SOCKET socketList[maxSocketNum];
	for (int i = 0; i < maxSocketNum; ++i)
		socketList[i] = INVALID_SOCKET;
	for (;;)
	{
		count = select(maxSocket + 1, &rlist, nullptr, nullptr, &timeout);
		if (count == SOCKET_ERROR)
		{
			this->SetErrorInfo("select failed");
			return false;
		}
		if (count == 0)
			continue;
		if (FD_ISSET(this->serverFd, &rlist))
		{
			count--;
			this->HandleServerMessage();
		}
	}
}

void Client::Impl::SetErrorInfo(const std::string &errorInfo) { info = errorInfo; }

Client::Client(const char *serverAddr, unsigned short serverPort, const char *forwardAddr, unsigned short forwardPort)
{
	this->pImpl = std::make_unique<Impl>();
	this->pImpl->SetServer(serverAddr, serverPort);
	this->pImpl->SetForward(forwardAddr, forwardPort);
}
const char *Client::Error() { return this->pImpl->Error(); }
bool Client::Start() { return this->pImpl->Start(); }
