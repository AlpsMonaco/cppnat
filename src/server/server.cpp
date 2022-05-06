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
	using Conn = std::shared_ptr<asio::ip::tcp::socket>;

	SImpl::Impl(const char *listenAddr,
				unsigned short listenPort) : ios_(),
											 acceptor_(ios_, tcp::endpoint(address::from_string(listenAddr), listenPort)),
											 client_(ios_)
	{
	}
	SImpl::~Impl() {}

	bool Start()
	{
		for (;;)
		{
			if (!WaitForClient())
				return false;
			BeginProxy();
		}
	}

	inline std::string Error() { return ec_.message(); }
	inline int Errno() { return ec_.value(); }

protected:
	asio::io_service ios_;
	asio::ip::tcp::acceptor acceptor_;
	asio::ip::tcp::socket client_;
	BoostErrorCode ec_;
	ProtocolBuffer protocolBuffer_;

	bool WaitForClient()
	{
		static constexpr size_t tempBufferSize = 64;
		char tempBuffer[tempBufferSize];
		asio::mutable_buffer tempBufferPtr = asio::buffer(tempBuffer, tempBufferSize);
		size_t readNum = 0;
		for (;;)
		{
			client_ = acceptor_.accept(ec_);
			if (ec_)
				return false;
			readNum = client_.read_some(tempBufferPtr, ec_);
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
			if (!Handshake::IsMatch<Handshake::VerifyClient>(tempBuffer, readNum))
			{
				client_.close();
				continue;
			}
			else
			{
				client_.write_some(asio::buffer(Handshake::ResponseOK::data,
												Handshake::ResponseOK::dataView.size()),
								   ec_);
				return true;
			}
		}
	}

	void ClientHandler()
	{
		client_.async_read_some(asio::buffer(protocolBuffer_.GetNextBuffer(), protocolBuffer_.GetNextSize()),
								[this](const BoostErrorCode &ec, size_t bytes) -> void
								{
									if (ec)
									{
										ec_ = ec;
										ios_.stop();
										return;
									}
									if (protocolBuffer_.Parse(bytes))
									{
									}
									ClientHandler();
								});
	}

	void BeginProxy()
	{
		ClientHandler();
	}
};

inline Server::Server(const char *listenAddr,
					  unsigned short listenPort)
{
	pImpl_ = std::make_unique<Server::Impl>(listenAddr,
											listenPort);
}

Server::~Server() {}

inline bool Server::Start() { return pImpl_->Start(); }

void Server::Stop()
{
}

inline std::string Server::Error() { return pImpl_->Error(); }
inline int Server::Errno() { return pImpl_->Errno(); }