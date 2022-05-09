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
		static constexpr size_t BodySize = PacketSize - HeaderLength;
	};

	template <typename Protocol>
	struct PacketWrapper
	{
		using PacketInstance = PacketWrapper<Protocol>;

		PacketInstance() = delete;
		PacketInstance(const PacketInstance &packet) = delete;
		PacketInstance &operator=(const PacketInstance &packet) = delete;

		size_t Size() { return const_cast<const PacketInstance *>(this)->Size(); }
		void Size(size_t size)
		{
			assert(size_ <= Protocol::BodySize);
			size_ = Protocol::HeaderLength + size;
		}
		void Cmd(typename Protocol::CmdType cmd) { cmd_ = cmd; }
		typename Protocol::CmdType Cmd() { return const_cast<const PacketInstance *>(this)->Cmd(); }
		char *Body() { return body_; }
		const char *Body() const { return body_; }
		inline char *Buffer() { return reinterpret_cast<char *>(this); }
		inline const char *Buffer() const { return reinterpret_cast<const char *>(this); }

		size_t Size() const { return size_; }
		typename Protocol::CmdType Cmd() const { return cmd_; }

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

		static inline const PacketInstance &ToPacket(const char *buffer,
													 size_t bufferSize)
		{
			assert(bufferSize >= Protocol::PacketSize);
			return *reinterpret_cast<const PacketInstance *>(buffer);
		}

		static inline PacketInstance &ToPacket(char *buffer, size_t bufferSize)
		{
			assert(bufferSize >= Protocol::PacketSize);
			return *reinterpret_cast<PacketInstance *>(buffer);
		}

		inline Proxy To() { return Proxy(body_); }
		inline const Proxy To() const { return Proxy(const_cast<char *>(body_)); }

		static const size_t bodySize = Protocol::BodySize;

	private:
		typename Protocol::SizeType size_;
		typename Protocol::CmdType cmd_;
		char body_[bodySize];
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

		inline void Handle(const PacketType &packet)
		{
			assert(callbacks.find(packet.Cmd()) != callbacks.end());
			callbacks[packet.Cmd()](packet);
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

		inline void AddCallback(typename Protocol::CmdType cmd,
								const typename PacketHandlerType::Callback &callback)
		{
			handler_.Add(cmd, callback);
		}

		inline char *GetNextBuffer()
		{
			return buffer_.Get() + readSize_;
		}

		inline size_t GetNextSize()
		{
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
			if (readSize_ < packet.Size())
				return;
			if (readSize_ == packet.Size())
				readSize_ = 0;
			else
				extraOffset_ = packet.Size();
#ifdef __PRINT_READER_BUFFER__
			StreamWriter sw;
			sw << "recv size:" << readSize_ << std::endl;
			sw << "recv bytes: " << std::endl;
			for (size_t i = 0; i < readSize_; i++)
				sw << unsigned short(buffer_.Get()[i]) << " ";
			sw << std::endl;
			sw.Write();
			handler_.Handle(packet);
#endif
			if (extraOffset_ > 0)
				ParseExtraData();
		}

		~PacketReadBuffer() {}

	protected:
		void ParseExtraData()
		{
			static size_t remainSize = 0;
			do
			{
				remainSize = readSize_ - extraOffset_;
				if (remainSize < Protocol::HeaderLength)
				{
					memcpy(buffer_.Get(), buffer_.Get() + extraOffset_, remainSize);
					readSize_ = remainSize;
					extraOffset_ = 0;
					return;
				}
				const PacketType &packet = PacketType::ToPacket(buffer_.Get() + extraOffset_, Protocol::PacketSize);
				if (remainSize < packet.Size())
				{
					memcpy(buffer_.Get(), buffer_.Get() + extraOffset_, remainSize);
					extraOffset_ = 0;
					readSize_ = remainSize;
					return;
				}
				if (remainSize == packet.Size())
				{
					readSize_ = 0;
					extraOffset_ = 0;
				}
				else
					extraOffset_ += packet.Size();
#ifdef __PRINT_READER_BUFFER__
				StreamWriter sw;
				sw << "remainSize:" << remainSize << std::endl
				   << "readSize_:" << readSize_ << std::endl
				   << "extraOffset_:" << extraOffset_ << std::endl;
				sw.Write();
#endif
				handler_.Handle(packet);
			} while (extraOffset_ > 0);
		}

		BufferType buffer_;
		size_t readSize_;
		size_t extraOffset_;
		PacketHandlerType handler_;
	};
}

#endif