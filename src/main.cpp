#include <string_view>
#include <iostream>
#include <map>
#include <functional>

using namespace std;

struct Transfer
{
	Transfer(const char *p) : p_(p) {}
	struct Proxy
	{
		Proxy(const char *p) { this->p_ = p; }

		template <typename T>
		operator const T &() { return *reinterpret_cast<const T *>(p_); }

		template <typename T>
		operator T &() = delete;

		template <typename T>
		operator T() = delete;

	protected:
		const char *p_;
	};

	Proxy Data()
	{
		return Proxy(p_);
	}

protected:
	const char *p_;
};

using FuncPointer = void *;

void PrintInt(int a)
{
	cout << "int " << a << std::endl;
}

void PrintDouble(double d)
{
	cout << "double " << d << std::endl;
}

std::map<unsigned short, FuncPointer> funcMap;

template <typename T>
void Call(unsigned short cmd, T t)
{
	reinterpret_cast<void (*)(T)>(funcMap[cmd])(t);
}

#include "message.h"
using namespace cppnat;

int main(int argc, char **argv)
{
	PacketWriter writer;
	Packet packet = writer.Packet();
	packet.size = 1;
	packet.cmd = 2;
	long long &val = packet.Data();
	val = 0x01020304;
	char *buffer = writer.Buffer();
	for (size_t i = 0; i < 16; i++)
	{
		cout << size_t(buffer[i]) << " ";
	}

	PacketReader reader;
	buffer = reader.GetNextBuffer();
	memcpy(buffer, writer.Buffer(), 128);
	ConstPacket readerPacket = reader.Packet();
	cout << readerPacket.size << " " << readerPacket.cmd << std::endl;
	const long long &newVal = readerPacket.Data();
	cout << newVal << std::endl;
}