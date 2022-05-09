#include "client.h"
#include <vector>
#include <boost/asio.hpp>
#include "../message.h"
#include "../connection.h"

using namespace boost;
using namespace cppnat;

#define HandleError(ec)                               \
	do                                                \
	{                                                 \
		StreamWriter sw;                              \
		sw << ec << " " << ec.message() << std::endl; \
		sw.Write();                                   \
	} while (0)

#define CImpl Client::Impl

class CImpl
{
protected:
	using ConnHelper = typename ConnManager<asio::ip::tcp::socket, kBufferSize>::ConnHelper;

public:
	CImpl::Impl(const char *serverAddr, unsigned short serverPort,
				const char *forwardAddr, unsigned short forwardPort)
		: serverSocket_(ios_),
		  serverEndpoint_(asio::ip::address::from_string(serverAddr), serverPort),
		  targetEndpoint_(asio::ip::address::from_string(forwardAddr), forwardPort),
		  connManager_(128)
	{
		InitHandler();
	}
	CImpl::~Impl() {}

	void OnRecvTargetData(ConnHelper *connHelper)
	{
		Packet &packet = Packet::ToPacket(connHelper->buffer, kBufferSize);
		Msg::DataTransfer &dt = packet.To();
		connHelper->conn->async_read_some(
			asio::buffer(
				dt.data,
				Msg::DataTransfer::kDataTransferSize),
			[this, connHelper, &packet, &dt](
				const boost::system::error_code &ec,
				size_t bytes_transffered) -> void
			{
				std::cout << "client got " << bytes_transffered << std::endl;
				StreamWriter sw;
				for (size_t i = 0; i < bytes_transffered; i++)
				{
					sw << size_t(unsigned char(dt.data[i])) << " ";
				}
				sw << std::endl;
				sw.Write();
				if (ec)
				{
					HandleError(ec);
					OnConnError(connHelper);
					connHelper->conn->close();
				}
				else
				{
					packet.Cmd(MessageCmd::CMD_DataTransfer);
					packet.Size(Msg::DataTransfer::kDataTransferHeaderSize + bytes_transffered);
					dt.size = bytes_transffered;
					dt.id = connHelper->id;
					WriteToServer(packet);
					OnRecvTargetData(connHelper);
				}
			});
	}

	void OnConnError(ConnHelper *connHelper)
	{
		Packet &writePacket = writer_.Packet();
		connHelper->conn->close();
		writePacket.Cmd(MessageCmd::CMD_ConnClosed);
		writePacket.Size(sizeof(Msg::ConnClosed));
		Msg::ConnClosed &msg = writePacket.To();
		msg.id = connHelper->id;
		WriteToServer(writer_.Packet());
	}

	void InitHandler()
	{
		reader_.AddCallback(
			MessageCmd::CMD_RequestNewConn,
			[this](ConstPacket &packet) -> void
			{
				const Msg::NewConnAccept &msg = packet.To();
				ConnHelper *connHelper = connManager_.NewAt(msg.id, ios_);
				connHelper->conn->async_connect(
					targetEndpoint_,
					[this, connHelper](const boost::system::error_code &ec) -> void
					{
						Packet &packet = writer_.Packet();
						if (ec)
						{
							HandleError(ec);
							std::cout << targetEndpoint_ << " failed" << std::endl;
							packet.Cmd(MessageCmd::CMD_RejectNewConn);
							packet.Size(sizeof(Msg::RejectNewConn));
							Msg::RejectNewConn &msg = packet.To();
							msg.id = connHelper->id;
							WriteToServer(packet);
						}
						else
						{
							packet.Cmd(MessageCmd::CMD_AcceptNewConn);
							packet.Size(sizeof(Msg::AcceptNewConn));
							Msg::AcceptNewConn &msg = packet.To();
							msg.id = connHelper->id;
							WriteToServer(packet);
							OnRecvTargetData(connHelper);
						}
					});
			});

		reader_.AddCallback(MessageCmd::CMD_ConnClosed, [this](ConstPacket &packet) -> void
							{
								const Msg::ConnClosed &msg = packet.To();
								connManager_.Get(msg.id)->conn->close(ec_);
								if (ec_)
								{
									HandleError(ec_);
								} });
		reader_.AddCallback(MessageCmd::CMD_DataTransfer, [this](ConstPacket &packet) -> void
							{
								const Msg::DataTransfer &msg = packet.To();
								ConnHelper *connHelper = connManager_.Get(msg.id);
								memcpy(connHelper->writeBuffer, msg.data, msg.size);
								asio::async_write(*connHelper->conn,
												  asio::buffer(connHelper->writeBuffer, msg.size),
												  [this, connHelper](const boost::system::error_code &ec,
												   size_t bytes_transffered) -> void {
													   if(ec){
														   HandleError(ec);
														   OnConnError(connHelper);
													   }
												   }
								); });
	}

	void WriteToServer(ConstPacket &packet)
	{
		StreamWriter sw;
		sw << "to server:" << std::endl;
		for (size_t i = 0; i < packet.Size(); i++)
		{
			sw << size_t(unsigned char(packet.Buffer()[i])) << " ";
		}
		sw << std::endl;
		sw.Write();
		asio::write(serverSocket_,
					asio::buffer(packet.Buffer(), packet.Size()),
					ec_);
		if (ec_)
		{
			HandleError(ec_);
			ios_.stop();
		}
	}

	bool CImpl::Start()
	{
		ios_.restart();
		serverSocket_.connect(serverEndpoint_, ec_);
		if (ec_)
			return false;
		if (!Handshake())
			return false;
		ServerMessageLoop();
		ios_.run();
		return true;
	}

	void ServerMessageLoop()
	{
		serverSocket_.async_read_some(asio::buffer(reader_.GetNextBuffer(), reader_.GetNextSize()),
									  [this](const boost::system::error_code &ec, size_t bytes) -> void
									  {
										  if (ec)
										  {
											  HandleError(ec);
											  ec_ = ec;
											  ios_.stop();
											  return;
										  }
										  reader_.ReadBytes(bytes);
										  ServerMessageLoop();
									  });
	}

	bool SendEchoMessage(const std::string &message)
	{
		Packet &packet = writer_.Packet();
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
	asio::ip::tcp::endpoint targetEndpoint_;
	asio::ip::tcp::socket serverSocket_;
	boost::system::error_code ec_;
	PacketWriter writer_;
	PacketReader reader_;
	ConnManager<asio::ip::tcp::socket, kBufferSize> connManager_;
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