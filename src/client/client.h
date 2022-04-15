#ifndef __CPP_NAT_CLIENT
#define __CPP_NAT_CLIENT

#ifdef _WIN32
#include <WinSock2.h>
#else
#endif

#include "message.h"

namespace cppnat
{
	constexpr unsigned short defaultClientPort = 64411;

	class Client
	{
	public:
		Client(const char *serverAddr, unsigned short serverPort, unsigned short port = defaultClientPort);
		~Client();
		int Errno();
		const char *Error();
		bool Begin();

	protected:
		void SetErrorMessage(const char *msg);
		SOCKET fd;
		SOCKET serverFd;
		sockaddr_in serverAddr;
		unsigned short listenPort;
		char *errorMessage;
	};
}

#endif
