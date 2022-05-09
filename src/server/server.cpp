#include "server.h"
#include "../message.h"
#include <string>
#include <string_view>
#include <boost/asio.hpp>
#include "../connection.h"

#define HandleError(ec)                               \
	do                                                \
	{                                                 \
		StreamWriter sw;                              \
		sw << ec << " " << ec.message() << std::endl; \
		sw.Write();                                   \
	} while (0)

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
		InitHandler();
		for (;;)
		{
			ios_.restart();
			if (!WaitForClient())
				return false;
			BeginProxy();
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
	PacketWriter writer_;
	ConnManager<asio::ip::tcp::socket, kBufferSize> connManager_;

	using ConnHelper = typename ConnManager<asio::ip::tcp::socket, kBufferSize>::ConnHelper;

	void HandleUserData(size_t id)
	{
		ConnHelper *connHelper = connManager_.Get(id);
		Msg::DataTransfer &dt = Packet::ToPacket(connHelper->buffer, kBufferSize).To();
		connHelper->conn->async_read_some(
			asio::buffer(dt.data,
						 Msg::DataTransfer::kDataTransferSize),
			[this, &dt, connHelper](const boost::system::error_code &ec,
									size_t bytes_transffered) -> void
			{
				Packet &writePacket = writer_.Packet();
				if (ec)
				{
					HandleError(ec);
					connHelper->conn->close();
					writePacket.Cmd(MessageCmd::Cmd_ConnClosed);
					writePacket.Size(sizeof(Msg::ConnClosed));
					Msg::ConnClosed &msg = writePacket.To();
					msg.id = connHelper->id;
					SendToClient(writer_.Packet());
				}
				else
				{
					writePacket.Cmd(MessageCmd::Cmd_DataTransfer);
					writePacket.Size(Msg::DataTransfer::kDataTransferHeaderSize + bytes_transffered);
					Msg::DataTransfer &msg = writePacket.To();
					msg.size = bytes_transffered;
					msg.id = connHelper->id;
					SendToClient(writer_.Packet());
					HandleUserData(connHelper->id);
				}
			});
	}

	void InitHandler()
	{
		reader_.AddCallback(MessageCmd::CMD_Echo,
							[](ConstPacket &packet) -> void {
#ifndef __DISABLE_ECHO_OUTPUT__
#else
				StreamWriter sw;
				sw << "--------CMD_ECHO--------" << std::endl
				   << "cmd: " << unsigned long long(packet.Cmd()) << std::endl
				   << "size: " << packet.Size() << std::endl
				   << "data: " << packet.Body() << std::endl
				   << "------CMD_ECHO_END------" << std::endl;
				sw.Write();
#endif
							});

		reader_.AddCallback(MessageCmd::CMD_AcceptNewConn,
							[this](ConstPacket &packet) -> void
							{
								const Msg::AcceptNewConn &msg = packet.To();
								HandleUserData(msg.id);
							});
		reader_.AddCallback(MessageCmd::Cmd_RejectNewConn,
							[this](ConstPacket &packet) -> void
							{
								const Msg::AcceptNewConn &msg = packet.To();
								connManager_.Get(msg.id)->conn->close(ec_);
							});

		reader_.AddCallback(MessageCmd::Cmd_DataTransfer, [this](ConstPacket &packet) -> void
							{
								const Msg::DataTransfer &msg = packet.To();
								ConnHelper *connHelper = connManager_.Get(msg.id);
								memcpy(connHelper->buffer, msg.data, msg.size);
								asio::async_write(*connHelper->conn,
												  asio::buffer(connHelper->buffer, msg.size),
												  [this, connHelper](const boost::system::error_code &ec,
												   size_t bytes_transffered) -> void {
													   if(ec){
														   HandleError(ec);
														   connHelper->conn->close();
														   Packet &writePacket = writer_.Packet();
														   writePacket.Cmd(MessageCmd::Cmd_ConnClosed);
														   writePacket.Size(sizeof(Msg::ConnClosed));
														   Msg::ConnClosed &msg = writePacket.To();
														   msg.id = connHelper->id;
														   SendToClient(writer_.Packet());
													   }
												   }
								); });
		reader_.AddCallback(MessageCmd::Cmd_ConnClosed, [this](ConstPacket &packet) -> void
							{
								const Msg::ConnClosed &msg = packet.To();
								connManager_.Get(msg.id)->conn->close(ec_);
								if (ec_)
								{
									HandleError(ec_);
								} });
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
										HandleError(ec);
										ec_ = ec;
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
			HandleError(ec_);
			ios_.stop();
		}
	}

	void OnNewUserConnection(ConnHelper *helper)
	{
		Packet &packet = writer_.Packet();
		packet.Cmd(MessageCmd::CMD_AcceptNewConn);
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
					HandleError(ec);
					ec_ = ec;
					ios_.stop();
					return;
				}
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