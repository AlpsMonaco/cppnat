#ifndef __CPP_NAT_SERVER
#define __CPP_NAT_SERVER

#include <memory>

namespace cppnat
{
	class Server
	{
	public:
		Server(const char *listenAddr, int listenPort);
		~Server();
		bool Begin();
		void Stop();
		const char *Error();

	protected:
		class Impl;
		std::unique_ptr<Impl> pImpl;
	};
}

#endif