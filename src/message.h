#ifndef __CPP_NAT_MESSAGE_HPP__
#define __CPP_NAT_MESSAGE_HPP

#include <map>
#include <string_view>
#include "util.h"
#include "packet.h"

namespace cppnat
{
	constexpr size_t kBufferSize = 65535;
	using Protocol = PacketProtocol<uint16_t, uint16_t, kBufferSize>;
	using PacketReader = PacketReadBuffer<Protocol>;
	using PacketWriter = PacketWriteBuffer<Protocol>;
	using Packet = typename PacketWriter::PacketType &;
	using ConstPacket = const typename PacketReader::PacketType &;

	class Handshake
	{
	public:
		template <typename T>
		static bool IsMatch(const char *data, size_t size) { return std::string_view(data, size) == T::dataView; }

		template <typename T>
		static const char *Data() { return T::data; }

		template <typename T>
		static size_t Size() { return T::dataView.size(); }

		template <class T>
		class Protocol
		{
		public:
			static const char *Data() { return T::data; }
			static size_t Size() { return T::dataView.size(); }
			static bool IsMatch(const char *data, size_t size) { return std::string_view(data, size) == T::dataView; }
		};

		struct VerifyClient
		{
			static constexpr char data[] = "CPP_NAT_CS_VERIFY_INFO_V0.0.1";
			static constexpr std::string_view dataView = data;
		};

		struct ResponseOK
		{
			static constexpr char data[] = "1";
			static constexpr std::string_view dataView = data;
		};

		struct ResponseFailed
		{
			static constexpr char data[] = "0";
			static constexpr std::string_view dataView = data;
		};
	};
}

#endif
