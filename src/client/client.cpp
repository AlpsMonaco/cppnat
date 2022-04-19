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
	inline void HandleServerMessage();

	using any = void *;
	using Callback = void (*)(any, SOCKET, DataManager &dataManager);

protected:
	void InitHandlers();
	void InitDataManager();
	void SetErrorInfo(const std::string &errorInfo);
	std::string info;
	sockaddr_in serverSockAddr;
	SOCKET serverFd;
	sockaddr_in dstAddr;
	MessageHandler<Callback> handler;
	fd_set fdSet;
	WriteBuffer writeBuffer;
	DataManager dataManager;
	BindMap bindMap;
};

CImpl::Impl() : serverFd(INVALID_SOCKET), info("")
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
	this->handler.AddCallback(MsgEnum::NEW_NAT_REQUEST, &FnNewNatRequest);
}

void CImpl::InitDataManager()
{
	this->dataManager.Put(DataId::CLIENT, this);
	this->dataManager.Put(DataId::WRITE_BUFFER, &this->writeBuffer);
	this->dataManager.Put(DataId::FD_SET, &this->fdSet);
	this->dataManager.Put(DataId::PRIVATE_SOCKADDR, &this->dstAddr);
	this->dataManager.Put(DataId::BIND_MAP, &this->bindMap);
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

	int count;
	count = send(this->serverFd, (const char *)&versionInfo, sizeof(VersionInfo), 0);
	if (count == SOCKET_ERROR)
	{
		this->SetErrorInfo("send version info failed");
		return false;
	}
	char buffer[bufferSize];
	count = recv(this->serverFd, buffer, bufferSize, 0);
	if (count == SOCKET_ERROR)
	{
		this->SetErrorInfo("recv version info failed");
		return false;
	}
	unsigned short code = GetNumber(MsgCode::SUCCESS);
	if (count < sizeof(code))
	{
		this->SetErrorInfo("recv version info size error");
		return false;
	}
	if (code != *((unsigned short *)buffer))
	{
		this->SetErrorInfo("recv version info error");
		return false;
	}

	this->InitHandlers();

	FD_ZERO(&fdSet);
	FD_SET(this->serverFd, &fdSet);
	fd_set rlist;
	timeval timeout{1, 0};

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
