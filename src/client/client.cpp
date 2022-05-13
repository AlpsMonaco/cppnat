#include "client.h"
#include <vector>
#include <boost/asio.hpp>
#include "../message.h"
#include "../connection.h"

using namespace boost;
using namespace cppnat;

#define HandleError(ec)                            \
	do                                             \
	{                                              \
		std::stringstream sw;                      \
		sw << "client:" << ec << " " << ec.what(); \
		LOG_ERROR(sw.str());                       \
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
				if (ec)
				{
					LOG_ERROR(std::string("host connection error: connId:") +
							  std::to_string(connHelper->id) + " " +
							  std::to_string(ec.value()) + " " + ec.message());
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
				if (connHelper == nullptr)
				{
					LOG_ERROR(std::string("binding opened conn id: ") + std::to_string(msg.id));
					return;
				}
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
								LOG_INFO("client:closing conn,id: " + std::to_string(msg.id));
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
		LOG_INFO("connecting to server");
		if (serverSocket_.is_open())
			serverSocket_.close();
		serverSocket_.connect(serverEndpoint_, ec_);
		if (ec_)
		{
			LOG_ERROR(ec_.message());
			return false;
		}
		if (!Handshake())
			return false;
		LOG_INFO("connected to server");
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
		LOG_INFO("handshaking with server");

		size_t size = asio::write(serverSocket_,
								  asio::buffer(Handshake::VerifyClient::data,
											   Handshake::VerifyClient::size),
								  ec_);
		if (ec_)
		{
			LOG_ERROR("error occurs while write to server:" + ec_.message());
			return false;
		}
		if (size != Handshake::VerifyClient::size)
		{
			LOG_ERROR("size of sending VerifyClient not match" +
					  std::to_string(size) + " != " + std::to_string(Handshake::VerifyClient::size));
			return false;
		}
		static constexpr size_t tempBufferSize = 64;
		char tempBuffer[tempBufferSize];
		size = serverSocket_.read_some(asio::buffer(tempBuffer, tempBufferSize), ec_);
		if (ec_)
		{
			LOG_ERROR("error occurs while read from server:" + ec_.message());
			return false;
		}
		if (!Handshake::IsMatch<Handshake::ResponseOK>(tempBuffer, size))
		{
			LOG_ERROR("!Handshake::IsMatch<Handshake::ResponseOK>(tempBuffer, size)");
			return false;
		}
		return true;
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

bool Client::SendEchoMessage(const std::string &msg)
{
	return pImpl_->SendEchoMessage(msg);
}