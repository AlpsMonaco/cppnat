#ifndef __CPP_NAT_MESSAGE_HPP__
#define __CPP_NAT_MESSAGE_HPP

#ifdef _WIN32
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#endif

#include <map>
#include "util.h"
#include "packet.h"

namespace cppnat
{
	constexpr int bufferSize = 65535;
	using Protocol = PacketProtocol<uint16_t, uint16_t, bufferSize>;
	using Streamer = PacketStreamer<Protocol>;
	using Packet = Streamer::PacketType;

	struct SocketReader : public Reader
	{
		SocketReader() : fd(INVALID_SOCKET) {}
		~SocketReader() {}
		SocketReader &operator()(SOCKET fd)
		{
			this->fd = fd;
			return *this;
		}

		SocketReader(const SocketReader &) = default;
		SocketReader &operator=(const SocketReader &) = default;
		inline Reader::Size Read(char *buffer, size_t bufferSize) override { return recv(fd, buffer, bufferSize, 0); }

	protected:
		SOCKET fd;
	};

	enum class MsgCode : unsigned char
	{
		Success,
		Failed,
	};

	constexpr char CSVerifyInfo[] = "CPP_NAT_CS_VERIFY_INFO_V0.0.1";
}

#endif
