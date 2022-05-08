#include "server.h"
#include "../message.h"
#include <string>
#include <string_view>
#include <boost/asio.hpp>

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
		InitHandler();
		for (;;)
		{
			if (!WaitForClient())
				return false;
			ClientMessageLoop();
			ios_.run();
		}
	}

	std::string Error() { return ec_.message(); }
	int Errno() { return ec_.value(); }

protected:
	asio::io_service ios_;
	asio::ip::tcp::acceptor acceptor_;
	asio::ip::tcp::socket client_;
	BoostErrorCode ec_;
	PacketReader reader_;

	void InitHandler()
	{
		reader_.AddCallback(MessageCmd::CMD_ECHO,
							[](ConstPacket packet) -> void {
#ifndef __DISABLE_ECHO_OUTPUT__
#else
				StreamWriter sw;
				sw << "--------CMD_ECHO--------" << std::endl
				   << "cmd: " << unsigned long long(packet.Cmd()) << std::endl
				   << "size: " << packet.Size() << std::endl
				   << "data: " << packet.Data() << std::endl
				   << "------CMD_ECHO_END------" << std::endl;
				sw.Write();
#endif
							});
	}

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
												Handshake::ResponseOK::size),
								   ec_);
				return true;
			}
		}
	}

	void ClientMessageLoop()
	{
		client_.async_read_some(asio::buffer(reader_.GetNextBuffer(), reader_.GetNextSize()),
								[this](const BoostErrorCode &ec, size_t bytes) -> void
								{
									if (ec)
									{
										ec_ = ec;
										ios_.stop();
										return;
									}
									reader_.ReadBytes(bytes);
									ClientMessageLoop();
								});
	}
};

Server::Server(const char *listenAddr,
			   unsigned short listenPort)
{
	pImpl_ = std::make_unique<Server::Impl>(listenAddr,
											listenPort);
}

Server::~Server() {}

bool Server::Start() { return pImpl_->Start(); }

void Server::Stop()
{
}

std::string Server::Error() { return pImpl_->Error(); }
int Server::Errno() { return pImpl_->Errno(); }