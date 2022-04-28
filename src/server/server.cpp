#include "server.h"
#include <WinSock2.h>
#include <thread>
#include <string_view>

using namespace cppnat;

#define SImpl Server::Impl

class SImpl
{
public:
	SImpl::Impl();
	SImpl::~Impl();

	bool BindAndListen();
	void SetListenAddr(const char *listenAddr, int listenPort);
	bool BeginCSThread();
	bool BeginNatThread();
	const char *Error();

protected:
	void SetErrorInfo(const std::string &info);
	sockaddr_in sockaddr;
	SOCKET sock;
	std::string error;
};

/**
 * @brief export server implementation
 */
Server::Server(const char *listenAddr, int listenPort)
{
	pImpl = std::make_unique<SImpl>();
	pImpl->SetListenAddr(listenAddr, listenPort);
}
Server::~Server() {}
bool Server::Begin()
{
	if (!pImpl->BindAndListen())
		return false;
	return true;
}

void Server::Stop() {}
inline const char *Server::Error() { return pImpl->Error(); }

/****************************************/

SImpl::Impl() : sock(INVALID_SOCKET)
{
	WSADATA wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);
}

SImpl::~Impl() { WSACleanup(); }

void SImpl::SetListenAddr(const char *listenAddr, int listenPort)
{
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(listenPort);
	sockaddr.sin_addr.s_addr = inet_addr(listenAddr);
}

bool SImpl::BindAndListen()
{
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		this->SetErrorInfo("create socket failed");
		return false;
	}
	if (bind(sock, (SOCKADDR *)&sockaddr, sizeof(sockaddr)) == SOCKET_ERROR)
	{
		this->SetErrorInfo("bind socket failed");
		closesocket(sock);
		return false;
	}
	if (listen(sock, 20) == SOCKET_ERROR)
	{
		this->SetErrorInfo("listen socket failed");
		closesocket(sock);
		return false;
	}
	return true;
}

inline const char *SImpl::Error() { return error.c_str(); }