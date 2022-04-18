#ifndef __PRINT_MULTI_ARGS__
#define __PRINT_MULTI_ARGS__
#include <iostream>

template <typename T>
inline void Print(T t) { std::cout << t; }

template <typename T, typename... Args>
void Print(T t, Args... args)
{
	std::cout << t << " ";
	Print(args...);
}

template <typename... Args>
void Println(Args... args)
{
	Print(args...);
	std::cout << std::endl;
}

#endif

#include "message.hpp"

inline long GetHeader(unsigned short size, unsigned short cmd)
{
	return (size << 16) | cmd;
}

int main(int argc, char *argv[])
{
	cppnat::Buffer<65535, 4> buffer;
	buffer.SetHeader(0x0301);
	buffer.Get<unsigned short>() = 23;

	auto a = buffer.GetBuffer();
	for (int i = 0; i < 64; i++)
	{
		Println(int(a[i]));
	}

	unsigned int value = 0x04030201;
	char *aaa = reinterpret_cast<char *>(&value);
	Println(int(aaa[0]));
	Println(int(aaa[1]));
	Println(int(aaa[2]));
	Println(int(aaa[3]));
	Println(sizeof(long));

	Println(GetHeader(0x0403, 0x0201));
}