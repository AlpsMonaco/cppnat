#include "server.h"
#include <string>
#include <string_view>
#include <boost/asio.hpp>
#include "../buffer.h"
#include "../message.h"

using namespace cppnat;
using namespace boost;
using asio::ip::address;
using asio::ip::tcp;

using BoostErrorCode = boost::system::error_code;

#define SImpl Server::Impl
class Server::Impl
{
public:
	SImpl::Impl(const char *listenAddr,
				unsigned short listenPort) : ios_(),
											 acceptor_(
												 ios_,
												 tcp::endpoint(
													 address::from_string(listenAddr),
													 listenPort)),
											 client_(ios_),
											 csBufferPtr_(asio::buffer(csBuffer_.Get(), csBuffer_.BufferSize()))
	{
	}
	SImpl::~Impl() {}

	bool Start()
	{
		if (!WaitForClient())
			return false;
	}

	inline std::string Error() { return ec_.message(); }
	inline int Errno() { return ec_.value(); }

protected:
	asio::io_service ios_;
	asio::ip::tcp::acceptor acceptor_;
	asio::ip::tcp::socket client_;
	BoostErrorCode ec_;

	Buffer<kBufferSize> csBuffer_;
	asio::mutable_buffer csBufferPtr_;

	bool WaitForClient()
	{
		size_t readNum = 0;
		for (;;)
		{
			client_ = acceptor_.accept(ec_);
			if (ec_)
				return false;
			readNum = client_.read_some(csBufferPtr_, ec_);
			if (ec_)
			{
				client_.close();
				continue;
			}
			if (readNum == 0)
			{
				client_.close();
				continue;
			}
			if (std::string_view(csBuffer_.Get(), readNum) != kCSVerifyInfoView)
			{
				client_.close();
				continue;
			}
			else
			{
				return true;
			}
		}
	}
};

inline Server::Server(const char *listenAddr,
					  unsigned short listenPort) { pImpl_ =
													   std::make_unique<Server::Impl>(listenAddr,
																					  listenPort); }

Server::~Server() {}

inline bool Server::Start() { return pImpl_->Start(); }

void Server::Stop()
{
}

inline std::string Server::Error() { return pImpl_->Error(); }
inline int Server::Errno() { return pImpl_->Errno(); }