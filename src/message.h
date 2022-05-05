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
	using Streamer = PacketStreamer<Protocol>;
	using Packet = Streamer::PacketType;

	enum class MsgCode : unsigned char
	{
		Success,
		Failed,
	};

	class CSHandShake
	{
	public:
		struct VerifyInfo
		{
			static constexpr char kCSVerifyInfo[] = "CPP_NAT_CS_VERIFY_INFO_V0.0.1";
			static constexpr std::string_view kCSVerifyInfoView = kCSVerifyInfo;
		};
	};

	constexpr char kCSVerifyInfo[] = "CPP_NAT_CS_VERIFY_INFO_V0.0.1";
	constexpr std::string_view kCSVerifyInfoView = kCSVerifyInfo;
}

#endif
