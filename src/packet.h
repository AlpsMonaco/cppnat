#ifndef __PACKET_H__
#define __PACKET_H__

#include <map>
#include <functional>
#include "util.h"
#include "buffer.h"

namespace cppnat
{
	template <typename Size, typename Cmd, size_t packetSize>
	struct PacketProtocol
	{
		using SizeType = Size;
		using CmdType = Cmd;
		static constexpr size_t SizeLength = sizeof(SizeType);
		static constexpr size_t CmdLength = sizeof(CmdType);
		static constexpr size_t HeaderLength = SizeLength + CmdLength;
		static constexpr size_t PacketSize = packetSize;
		static constexpr size_t DataSize = PacketSize - HeaderLength;
	};

	template <typename Protocol>
	struct PacketWrapper
	{
		typename Protocol::SizeType size;
		typename Protocol::CmdType cmd;
		char data[Protocol::PacketSize - Protocol::SizeLength - Protocol::CmdLength];

		struct Proxy
		{
			Proxy(char *p) { this->p = p; }

			template <typename T>
			operator const T &() const { return *reinterpret_cast<const T *>(p); }

			template <typename T>
			operator const T() const = delete;

			template <typename T>
			operator T &() { return *reinterpret_cast<T *>(p); }

			template <typename T>
			operator T() = delete;

			Proxy(const Proxy &proxy) { this->p = proxy.p; }

		protected:
			char *p;
		};

		using PacketInstance = PacketWrapper<Protocol>;
		static inline const PacketInstance &ToPacket(const char *buffer,
													 size_t bufferSize)
		{
			assert(bufferSize >= Protocol::PacketSize);
			return *reinterpret_cast<const PacketInstance *>(buffer);
		}

		PacketInstance() = delete;
		PacketInstance(const PacketInstance &packet) = delete;
		PacketInstance &operator=(const PacketInstance &packet) = delete;

		inline Proxy Data() { return Proxy(data); }
		inline const Proxy Data() const { return Proxy(const_cast<char *>(data)); }
	};

	template <typename Protocol>
	class PacketHandler
	{
	public:
		using PacketType = PacketWrapper<Protocol>;
		using Callback = std::function<void(const PacketType &)>;

		PacketHandler() {}
		~PacketHandler() {}

		inline void Add(typename Protocol::CmdType cmd, const Callback &callback)
		{
			callbacks[cmd] = callback;
		}

		inline bool Handle(const PacketType &packet)
		{
			assert(callbacks.find(packet.cmd) != callbacks.end());
			callbacks[packet.cmd](packet);
		}

	protected:
		std::map<typename Protocol::CmdType, Callback> callbacks;
	};

	template <typename Protocol>
	class PacketWriteBuffer
	{
	public:
		using PacketType = PacketWrapper<Protocol>;
		using BufferType = Buffer<Protocol::PacketSize>;

		PacketWriteBuffer() : buffer_(),
							  packet_(*reinterpret_cast<PacketType *>(buffer_.Get()))
		{
		}

		inline PacketType &Packet()
		{
			return packet_;
		}

		inline char *Buffer()
		{
			return buffer_.Get();
		}

	protected:
		BufferType buffer_;
		PacketType &packet_;
	};

	template <typename Protocol>
	class PacketReadBuffer
	{
	public:
		using BufferType = Buffer<Protocol::PacketSize>;
		using PacketHandlerType = PacketHandler<Protocol>;
		using PacketType = typename PacketHandlerType::PacketType;

		PacketReadBuffer() : buffer_(),
							 readSize_(0),
							 extraOffset_(0),
							 handler_() {}

		inline void AddCallback(const typename PacketHandlerType::Callback &callback)
		{
			handler_.Add(callback);
		}

		inline char *GetNextBuffer()
		{
			if (extraOffset_ > 0)
				MoveOldBuffer();
			return buffer_.Get() + readSize_;
		}

		inline size_t GetNextSize()
		{
			if (extraOffset_ > 0)
				MoveOldBuffer();
			return Protocol::PacketSize - readSize_;
		}

		inline const char *Data()
		{
			return buffer_.Get();
		}

		inline const PacketType &Packet()
		{
			return PacketType::ToPacket(buffer_.Get(), Protocol::PacketSize);
		}

		void ReadBytes(size_t readSize)
		{
			readSize_ += readSize;
			if (readSize_ < Protocol::HeaderLength)
				return;
			const PacketType &packet = PacketType::ToPacket(buffer_.Get(), Protocol::PacketSize);
			if (readSize_ < packet.size)
				return;
			if (readSize_ == packet.size)
				readSize_ = 0;
			else
				extraOffset_ = packet.size;
			handler_.Handle(packet);
		}

		~PacketReadBuffer() {}

	protected:
		void MoveOldBuffer()
		{
			memcpy(buffer_.Get(), buffer_.Get() + extraOffset_, readSize_ - extraOffset_);
			readSize_ -= extraOffset_;
			extraOffset_ = 0;
		}

		BufferType buffer_;
		size_t readSize_;
		size_t extraOffset_;
		PacketHandlerType handler_;
	};
}

#endif