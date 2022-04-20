#include "util.hpp"

namespace cppnat
{
	template <int packetSize, int headerSize>
	struct PacketWrapper
	{
		char header[headerSize];
		char data[packetSize - headerSize];

		struct Proxy
		{
			Proxy(char *p) { this->p = p; }

			template <typename T>
			operator T &() { return *reinterpret_cast<T *>(&(header)[0]); }

			template <typename T>
			operator T() = delete;

			Proxy(const Proxy &proxy) { this->p = proxy.p; }

		protected:
			char *p;
		};

		using PacketInstance = PacketWrapper<packetSize, headerSize>;

		Proxy Header() { return Proxy(header); }
		Proxy Data() { return Proxy(data); }

		int GetPacketSize() { return packetSize; }
		int GetHeaderSize() { return headerSize; }
	};
}