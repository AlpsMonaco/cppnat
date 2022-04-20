#ifndef __PACKET_H__
#define __PACKET_H__

#include "util.hpp"

namespace cppnat
{
	template <typename SizeType, typename CmdType, int packetSize = 65535>
	struct PacketWrapper
	{
		SizeType size;
		CmdType cmd;
		char data[packetSize - sizeof(SizeType) - sizeof(CmdType)];

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

		SizeType &Size() { return size; }
		CmdType &Cmd() { return cmd; }
		Proxy Data() { return Proxy(&(data)[0]); }
	};
}

#endif