#include "server.h"
#include "../message.h"
#include <string>
#include <string_view>
#include <boost/asio.hpp>
#include "../connection.h"

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
											 client_(ios_),
											 connManager_(128)
	{
	}
	SImpl::~Impl() {}

	bool Start()
	{
		LOG_INFO("server start");
		InitHandler();
		for (;;)
		{
			ios_.restart();
			if (!WaitForClient())
				return false;
			LOG_INFO("client connected");
			BeginProxy();
			ios_.run();
		}
	}

protected:
	asio::io_service ios_;
	asio::ip::tcp::acceptor acceptor_;
	asio::ip::tcp::socket client_;
	BoostErrorCode ec_;
	PacketReader reader_;
	PacketWriter writer_;
	ConnManager<asio::ip::tcp::socket, kBufferSize> connManager_;

	using ConnHelper = typename ConnManager<asio::ip::tcp::socket, kBufferSize>::ConnHelper;

	void OnConnError(ConnHelper *connHelper)
	{
		Packet &writePacket = writer_.Packet();
		connHelper->conn->close();
		writePacket.Cmd(MessageCmd::CMD_ConnClosed);
		writePacket.Size(sizeof(Msg::ConnClosed));
		Msg::ConnClosed &msg = writePacket.To();
		msg.id = connHelper->id;
		SendToClient(writer_.Packet());
	}

	void HandleUserData(size_t id)
	{
		ConnHelper *connHelper = connManager_.Get(id);
		Packet &packet = Packet::ToPacket(connHelper->buffer, kBufferSize);
		Msg::DataTransfer &dt = packet.To();
		connHelper->conn->async_read_some(
			asio::buffer(dt.data,
						 Msg::DataTransfer::kDataTransferSize),
			[this, &dt, &packet, connHelper](const boost::system::error_code &ec,
											 size_t bytes_transffered) -> void
			{
				if (ec)
				{
					LOG_ERROR(std::string("user connection error: connId:") +
							  std::to_string(connHelper->id) + " " +
							  std::to_string(ec.value()) + " " + ec.message());
					OnConnError(connHelper);
				}
				else
				{
					packet.Cmd(MessageCmd::CMD_DataTransfer);
					packet.Size(Msg::DataTransfer::kDataTransferHeaderSize + bytes_transffered);
					dt.size = bytes_transffered;
					dt.id = connHelper->id;
					SendToClient(packet);
					HandleUserData(connHelper->id);
				}
			});
	}

	void InitHandler()
	{
		reader_.AddCallback(MessageCmd::CMD_Echo,
							[](ConstPacket &packet) -> void {
							});

		reader_.AddCallback(MessageCmd::CMD_AcceptNewConn,
							[this](ConstPacket &packet) -> void
							{
								const Msg::AcceptNewConn &msg = packet.To();
								HandleUserData(msg.id);
							});
		reader_.AddCallback(MessageCmd::CMD_RejectNewConn,
							[this](ConstPacket &packet) -> void
							{
								const Msg::AcceptNewConn &msg = packet.To();
								connManager_.Get(msg.id)->conn->close(ec_);
							});

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
														   LOG_ERROR("CMD_DataTransfer:"+ ec.message());
														   OnConnError(connHelper);
													   }
												   }
								); });
		reader_.AddCallback(MessageCmd::CMD_ConnClosed, [this](ConstPacket &packet) -> void
							{
								const Msg::ConnClosed &msg = packet.To();
								LOG_INFO("server:closing conn,id: " + std::to_string(msg.id));
								connManager_.Get(msg.id)->conn->close(ec_);
								if (ec_)
								{
									LOG_ERROR(std::string("CMD_ConnClosed") + ec_.message());
								} });
	}

	bool WaitForClient()
	{
		LOG_INFO("waiting for client");
		static constexpr size_t tempBufferSize = 64;
		char tempBuffer[tempBufferSize];
		asio::mutable_buffer tempBufferPtr = asio::buffer(tempBuffer, tempBufferSize);
		size_t readNum = 0;
		for (;;)
		{
			client_ = acceptor_.accept(ec_);
			LOG_DEBUG("accepted new conn:" + client_.remote_endpoint().address().to_string() + ":" + std::to_string(client_.remote_endpoint().port()));
			if (ec_)
			{
				LOG_ERROR("accept failed " + ec_.message());
				return false;
			}
			readNum = client_.read_some(tempBufferPtr, ec_);
			if (ec_)
			{
				LOG_ERROR("read failed while handshaking " + ec_.message());
				client_.close();
				continue;
			}
			if (readNum == 0)
			{
				LOG_ERROR("server read 0 byte while handshaking");
				client_.close();
				continue;
			}
			if (!Handshake::IsMatch<Handshake::VerifyClient>(tempBuffer, readNum))
			{
				LOG_ERROR("!Handshake::IsMatch<Handshake::VerifyClient>(tempBuffer, readNum)");
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
										LOG_ERROR("client read failed " + ec.message());
										ios_.stop();
										return;
									}
									reader_.ReadBytes(bytes);
									ClientMessageLoop();
								});
	}

	void SendToClient(ConstPacket &packet)
	{
		asio::write(client_,
					asio::buffer(packet.Buffer(), packet.Size()),
					ec_);
		if (ec_)
		{
			LOG_ERROR("write failed " + ec_.message());
			ios_.stop();
		}
	}

	void OnNewUserConnection(ConnHelper *helper)
	{
		LOG_INFO("new user connection");
		Packet &packet = writer_.Packet();
		packet.Cmd(MessageCmd::CMD_RequestNewConn);
		packet.Size(sizeof(Msg::NewConnAccept));
		Msg::NewConnAccept &msg = packet.To();
		msg.id = helper->id;
		SendToClient(packet);
	}

	void AcceptUserConnection()
	{
		static asio::ip::tcp::socket tempSocket(ios_);
		acceptor_.async_accept(
			tempSocket,
			[this](const boost::system::error_code &ec) -> void
			{
				if (ec)
				{
					LOG_ERROR(std::string("accept failed ") + ec.message());
					ios_.stop();
					return;
				}
				LOG_DEBUG("accepted new conn:" + client_.remote_endpoint().address().to_string() + ":" + std::to_string(client_.remote_endpoint().port()));
				ConnHelper *helper =
					connManager_.New(std::move(tempSocket));
				if (helper == nullptr)
					tempSocket.close();
				else
					OnNewUserConnection(helper);
				AcceptUserConnection();
			});
	}

	void BeginProxy()
	{
		ClientMessageLoop();
		AcceptUserConnection();
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