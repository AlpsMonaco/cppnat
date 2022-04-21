#ifndef __PACKET_H__
#define __PACKET_H__

#include "util.hpp"

namespace cppnat
{
	template <typename SizeType, typename CmdType, size_t packetSize>
	struct PacketProtocol
	{
		using Size = SizeType;
		using Cmd = CmdType;
		static constexpr size_t SizeLength = sizeof(SizeType);
		static constexpr size_t CmdLength = sizeof(CmdType);
		static constexpr size_t HeaderLength = SizeLength + CmdLength;
		static constexpr size_t PacketSize = packetSize;
		static constexpr size_t DataSize = PacketSize - HeaderLength;
	};

	template <typename Protocol>
	struct PacketWrapper
	{
		typename Protocol::Size size;
		typename Protocol::Cmd cmd;
		char data[Protocol::PacketSize - Protocol::SizeLength - Protocol::CmdLength];

		struct Proxy
		{
			Proxy(char *p) { this->p = p; }

			template <typename T>
			operator T &() { return *reinterpret_cast<T *>(p); }

			template <typename T>
			operator T() = delete;

			Proxy(const Proxy &proxy) { this->p = proxy.p; }

		protected:
			char *p;
		};

		using PacketInstance = PacketWrapper<Protocol>;
		static PacketInstance &ToPacket(char *buffer, size_t bufferSize)
		{
			assert(bufferSize >= Protocol::PacketSize);
			return *reinterpret_cast<PacketInstance *>(buffer);
		}

		PacketInstance() = delete;
		PacketInstance(const PacketInstance &packet) = delete;
		PacketInstance &operator=(const PacketInstance &packet) = delete;

		inline char operator[](size_t index) { return reinterpret_cast<char *>(this)[index]; }
		Proxy Data() { return Proxy(data); }
	};

	/**
	 * return -1 where Read error
	 */
	struct Reader
	{
		using Size = long long;
		constexpr static Size ReaderError = -1;
		constexpr static Size ReaderClose = 0;
		virtual Size Read(char *buffer, size_t bufferSize) = 0;
	};

	template <typename Protocol>
	class PacketStreamer
	{
	public:
		using PacketType = PacketWrapper<Protocol>;
		enum class Status : unsigned short
		{
			Short,
			Equal,
			Extra,
		};

		PacketStreamer(Reader &reader) : reader(reader), readSize(0), extraIndex(0), extraSize(0) { memset(buffer, 0, Protocol::PacketSize); }
		bool Next()
		{
			if (extraSize > 0)
			{
				assert(extraIndex > 0);
				memcpy(buffer, buffer + extraIndex, extraSize);
				readSize = extraSize;
				extraSize = 0;
				extraIndex = 0;
				if (Parse(buffer) != Status::Short)
					return true;
			}
			for (;;)
			{
				Reader::Size size = reader.Read(buffer + readSize, Protocol::PacketSize - readSize);
				if (size <= Reader::ReaderClose)
					return false;
				readSize += size;
				Status status = Parse(buffer);
				if (status == Status::Short)
					continue;
				else
					return true;
			}
		}
		inline PacketType &GetPacket() { return *reinterpret_cast<PacketType *>(&(buffer[0])); }

	protected:
		inline Status Parse(char *buffer)
		{
			if (readSize < Protocol::HeaderLength)
				return Status::Short;
			PacketType &packet = PacketType::ToPacket(buffer, Protocol::PacketSize);
			if (readSize < packet.size)
			{
				return Status::Short;
			}
			else if (packet.size == readSize)
			{
				extraSize = 0;
				extraIndex = 0;
				readSize = 0;
				return Status::Equal;
			}
			else
			{
				extraSize = readSize - packet.size;
				extraIndex = packet.size;
				return Status::Extra;
			}
		}

		char buffer[Protocol::PacketSize];
		Reader &reader;
		size_t readSize;
		size_t extraIndex;
		size_t extraSize;
	};
}

#endif