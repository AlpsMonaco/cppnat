#ifndef __CLIENT_MESSAGE_HPP
#define __CLIENT_MESSAGE_HPP

#include "../message.hpp"

namespace cppnat
{
	inline long GetHeader(unsigned short size, unsigned short cmd) { return (size << 16) | cmd; }

	void FnNewNatRequest(MsgNewNatRequest *newNatRequest, SOCKET serverFd, DataManager *dataManager)
	{
	}
}

#endif