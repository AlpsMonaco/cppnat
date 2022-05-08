#ifndef __CPP_NAT_SERVER
#define __CPP_NAT_SERVER

#include <string>
#include <memory>
#include "../message.h"

namespace cppnat
{
	class Server
	{
	public:
		Server(const char *listenAddr, unsigned short listenPort);
		~Server();
		bool Start();
		void Stop();
		std::string Error();
		int Errno();
		void AddHandler(MessageCmd cmd, const PacketReader::PacketHandlerType::Callback &callback);

	protected:
		class Impl;
		std::unique_ptr<Impl> pImpl_;
	};
}

#endif