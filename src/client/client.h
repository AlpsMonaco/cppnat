#ifndef __CPP_NAT_CLIENT
#define __CPP_NAT_CLIENT

#include <memory>
#include <string>

namespace cppnat
{
	class Client
	{
	public:
		Client(const char *serverAddr, unsigned short serverPort,
			   const char *forwardAddr, unsigned short forwardPort);
		~Client();
		void Close();
		bool Start();
		const char *Error();
		int Errno();
		bool SendEchoMessage(const std::string &message);

	protected:
		class Impl;
		std::unique_ptr<Impl> pImpl_;
	};
}

#endif
