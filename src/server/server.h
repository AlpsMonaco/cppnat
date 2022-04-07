#ifndef __CPP_NAT_SERVER
#define __CPP_NAT_SERVER

#include <WinSock2.h>

namespace cppnat
{
	class Server
	{
	public:
		Server(const char *addr, int port);
		~Server();
		bool Listen();
		void Begin();
		void Stop();
		int Errno();

	protected:
		sockaddr_in addr;
		SOCKET fd;
		bool stop;
	};
}

#endif