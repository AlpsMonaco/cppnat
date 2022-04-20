#ifndef __CPP_NAT_CLIENT
#define __CPP_NAT_CLIENT

#include <memory>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

namespace cppnat
{
	class EXPORT Client
	{
	public:
		friend class ClientExport;
		Client(const char *serverAddr, unsigned short serverPort, const char *forwardAddr, unsigned short forwardPort);
		void Close();
		bool Start();
		const char *Error();

	protected:
		class Impl;
		std::unique_ptr<Impl> pImpl;
	};
}

#endif
