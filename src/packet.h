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
			Proxy(const char *p) { this->p = p; }

			template <typename T>
			operator const T &() { return *reinterpret_cast<const T *>(p); }

			template <typename T>
			operator T &() = delete;

			template <typename T>
			operator T() = delete;

			template <typename T>
			operator const T() = delete;

			Proxy(const Proxy &proxy) { this->p = proxy.p; }

		protected:
			const char *p;
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
		virtual ~Reader() {}
	};

	template <typename Protocol>
	class BufferPacket
	{
	public:
		using BufferType = Buffer<Protocol::PacketSize>;
		using PacketType = PacketWrapper<Protocol>;

		BufferPacket() : buffer_(),
						 readSize_(0),
						 extraOffset_(0) {}

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

		bool Parse(size_t readSize)
		{
			readSize_ += readSize;
			if (readSize_ < Protocol::HeaderLength)
				return false;
			const PacketType &packet = PacketType::ToPacket(buffer_.Get(), Protocol::PacketSize);
			if (readSize_ < packet.size)
				return false;
			if (readSize_ == packet.size)
				readSize_ = 0;
			else
				extraOffset_ = packet.size;
			return true;
		}
		~BufferPacket() {}

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
	};

	template <typename Protocol>
	class PacketParser
	{
	public:
		using PacketType = PacketWrapper<Protocol>;
		enum class Status : unsigned short
		{
			Short,
			Equal,
			Extra,
		};
		static Status Parse(const char *buffer, size_t bytes)
		{
			if (bytes < Protocol::HeaderLength)
				return Status::Short;
			const PacketType &packet = PacketType::ToPacket(buffer_,
															Protocol::PacketSize);
			if (bytes < packet.size)
				return Status::Short;
			else if (bytes == packet.size)
				return Status::Equal;
			else
				return Status::Extra;
		}
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

		PacketStreamer() = delete;
		PacketStreamer(Reader &reader) : reader_(reader),
										 readSize_(0),
										 extraIndex_(0),
										 extraSize_(0)
		{
			memset(buffer_, 0, Protocol::PacketSize);
		}
		bool Next()
		{
			if (extraSize_ > 0)
			{
				assert(extraIndex_ > 0);
				memcpy(buffer_, buffer_ + extraIndex_, extraSize_);
				readSize_ = extraSize_;
				extraSize_ = 0;
				extraIndex_ = 0;
				if (ParseBuffer() != Status::Short)
					return true;
			}
			for (;;)
			{
				Reader::Size size = reader_.Read(buffer_ + readSize_,
												 Protocol::PacketSize - readSize_);
				if (size <= Reader::ReaderClose)
					return false;
				readSize_ += size;
				Status status = ParseBuffer();
				if (status == Status::Short)
					continue;
				else
					return true;
			}
		}

		char *GetBuffer() { return buffer_; }
		inline PacketType &GetPacket() { return *reinterpret_cast<PacketType *>(&(buffer_[0])); }

	protected:
		inline Status ParseBuffer()
		{
			if (readSize_ < Protocol::HeaderLength)
				return Status::Short;
			const PacketType &packet = PacketType::ToPacket(buffer_,
															Protocol::PacketSize);
			if (readSize_ < packet.size)
			{
				return Status::Short;
			}
			else if (packet.size == readSize_)
			{
				extraSize_ = 0;
				extraIndex_ = 0;
				readSize_ = 0;
				return Status::Equal;
			}
			else
			{
				extraSize_ = readSize_ - packet.size;
				extraIndex_ = packet.size;
				return Status::Extra;
			}
		}

		char buffer_[Protocol::PacketSize];
		Reader &reader_;
		size_t readSize_;
		size_t extraIndex_;
		size_t extraSize_;
	};

	template <typename Protocol>
	class PacketHandler
	{
	public:
		using PacketType = PacketWrapper<Protocol>;
		using Callback = std::function<void(const PacketType &)>;
		PacketHandler() {}

		inline void AddCallback(const Callback &callback)
		{
			callbacks_.emplace_back(callback);
		}

	protected:
		std::map<Protocol::CmdType, Callback> handlers_;
	};
}

#endif