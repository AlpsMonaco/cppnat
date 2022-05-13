#ifndef __CPP_NAT_SERVER
#define __CPP_NAT_SERVER

#include <string>
#include <memory>

namespace cppnat
{
	class Server
	{
	public:
		Server(const char *listenAddr, unsigned short listenPort);
		~Server();
		bool Start();
		void Stop();

	protected:
		class Impl;
		std::unique_ptr<Impl> pImpl_;
	};
}

#endif