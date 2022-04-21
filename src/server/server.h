#ifndef __CPP_NAT_SERVER
#define __CPP_NAT_SERVER

#include "server_message.hpp"

namespace cppnat
{
	class Server
	{
	public:
		Server(const char *addr, int port);
		~Server();
		bool Listen();
		bool Begin();
		void Stop();
		const char *Error();

	protected:
		class Impl;
		std::unique_ptr<Impl> pImpl;
	};
}

#endif