#ifndef __CLIENT_MESSAGE_HPP
#define __CLIENT_MESSAGE_HPP

#include "../message.hpp"

namespace cppnat
{
	void FnNewNatRequest(MsgNewNatRequest *newNatRequest, SOCKET serverFd, DataManager &dataManager)
	{
	}

	void FnRemoteSocketDisconnect()
	{
	}

	void FnForwardLocalData()
	{
	}

	void FnLocalSocketDisconnect()
	{
	}

}

#endif