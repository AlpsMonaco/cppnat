#include "client.h"
#include <vector>
#include <boost/asio.hpp>
#include "../message.h"

using namespace boost;
using namespace cppnat;

#define CImpl Client::Impl

class CImpl
{
public:
	CImpl::Impl(const char *serverAddr, unsigned short serverPort,
				const char *forwardAddr, unsigned short forwardPort)
		: serverSocket_(ios_),
		  serverEndpoint_(asio::ip::address::from_string(serverAddr),
						  serverPort)
	{
	}
	CImpl::~Impl() {}

	bool CImpl::Start()
	{
		serverSocket_.connect(serverEndpoint_, ec_);
		if (ec_)
			return false;
		if (!Handshake())
			return false;
		return true;
	}

	bool SendEchoMessage(const std::string &message)
	{
		Packet &packet = writeBuffer_.Packet();
		packet.Cmd(MessageCmd::CMD_Echo);
		packet.Size(message.size() + 1);
		strcpy(packet.Body(), message.c_str());
		serverSocket_.write_some(asio::buffer(packet.Buffer(), packet.Size()),
								 ec_);
		if (ec_)
		{
			return false;
		}
		return true;
	}

	bool CImpl::Handshake()
	{
		static constexpr size_t tempBufferSize = 64;
		char tempBuffer[tempBufferSize];

		static size_t size = serverSocket_.write_some(
			asio::buffer(
				Handshake::VerifyClient::data,
				Handshake::VerifyClient::size),
			ec_);
		if (ec_)
			return false;
		if (size != Handshake::VerifyClient::size)
		{
			ec_ = asio::error::eof;
			return false;
		}
		size = serverSocket_.read_some(asio::buffer(tempBuffer, tempBufferSize), ec_);
		if (ec_)
			return false;
		if (!Handshake::IsMatch<Handshake::ResponseOK>(tempBuffer, size))
		{
			ec_ = asio::error::invalid_argument;
			return false;
		}
		return true;
	}

	inline const char *Error()
	{
		return ec_.message().c_str();
	}

	inline int Errno()
	{
		return ec_.value();
	}

	void Close() {}

protected:
	asio::io_service ios_;
	asio::ip::tcp::endpoint serverEndpoint_;
	asio::ip::tcp::socket serverSocket_;
	boost::system::error_code ec_;
	PacketWriter writeBuffer_;
};

Client::Client(const char *serverAddr, unsigned short serverPort,
			   const char *forwardAddr, unsigned short forwardPort)
{
	pImpl_ = std::make_unique<CImpl>(serverAddr, serverPort,
									 forwardAddr, forwardPort);
}

Client::~Client()
{
}

void Client::Close()
{
	return pImpl_->Close();
}

bool Client::Start()
{
	return pImpl_->Start();
}

const char *Client::Error()
{
	return pImpl_->Error();
}

int Client::Errno()
{
	return pImpl_->Errno();
}

bool Client::SendEchoMessage(const std::string &msg)
{
	return pImpl_->SendEchoMessage(msg);
}