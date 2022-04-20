#ifndef __CLIENT_MESSAGE_HPP
#define __CLIENT_MESSAGE_HPP

#include "../message.hpp"

namespace cppnat
{
	void CbNewNatRequest(MsgNewNatRequest *newNatRequest, SOCKET fd, DataManager &dataManager)
	{
		SOCKET srcFd = newNatRequest->fd;
		sockaddr_in &proxySockAddr = dataManager[DataId::PRIVATE_SOCKADDR];

		SOCKET proxyFd = socket(AF_INET, SOCK_STREAM, 0);
	}
}

#endif