#include "client.h"
#include "client_message.hpp"
#include <string>
#include <vector>

using namespace cppnat;

#define CImpl Client::Impl

class Client::Impl
{
public:
	Impl();
	~Impl();

	inline void SetServer(const char *serverAddr, unsigned short port);
	inline void SetForward(const char *forwardAddr, unsigned short forwardPort);
	inline bool Start();
	inline const char *Error();

	using any = void *;

protected:
	void InitHandlers();
	void InitDataManager();
	void SetErrorInfo(const std::string &errorInfo);

	std::string info;
	sockaddr_in serverSockAddr;
	SOCKET serverFd;
	sockaddr_in dstAddr;
	fd_set fdSet;
	SocketReader reader;
	Streamer streamer;
};

CImpl::Impl() : serverFd(INVALID_SOCKET), info(""), streamer(reader)
{
#ifdef _WIN32
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

CImpl::~Impl()
{
#ifdef _WIN32
	WSACleanup();
#endif
}

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

void CImpl::InitHandlers()
{
}

void CImpl::InitDataManager()
{
}

inline bool VerifyWithServer(SOCKET serverFd)
{
	int ret = send(serverFd, CSVerifyInfo, sizeof(CSVerifyInfo), 0);
	if (ret == SOCKET_ERROR)
		return false;
}

bool CImpl::Start()
{
	serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd == INVALID_SOCKET)
	{
		SetErrorInfo("create socket failed");
		return false;
	}
	int ret = connect(serverFd, (sockaddr *)&serverSockAddr, sizeof(serverSockAddr));
	if (ret == SOCKET_ERROR)
	{
		SetErrorInfo("connect to server failed");
		return false;
	}
	if (!VerifyWithServer(serverFd))
	{
		SetErrorInfo("verify with server failed");
		return false;
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
