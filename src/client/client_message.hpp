#ifndef __CLIENT_MESSAGE_HPP
#define __CLIENT_MESSAGE_HPP

#include "../message.hpp"

namespace cppnat
{
	inline long GetHeader(unsigned short size, unsigned short cmd) { return (size << 16) | cmd; }

	void FnNewNatRequest(MsgNewNatRequest *newNatRequest, SOCKET serverFd, DataManager &dataManager)
	{
		sockaddr_in &dstSockAddr = dataManager[DataId::PRIVATE_SOCKADDR];
		static SOCKET tempFd = INVALID_SOCKET;
		tempFd = socket(AF_INET, SOCK_STREAM, 0);
		if (tempFd == INVALID_SOCKET)
		{
			return;
		}
		if (connect(tempFd, (sockaddr *)&dstSockAddr, sizeof(dstSockAddr)) == SOCKET_ERROR)
		{
			return;
		}
		BindMap &bindMap = dataManager[DataId::BIND_MAP];
		SOCKET srcFd = newNatRequest->fd;
		bindMap[srcFd] = tempFd;
		bindMap[tempFd] = srcFd;
	}
}

#endif