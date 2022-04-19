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

int main()
{
	int i = 1;
	short j = 2;
	cppnat::DataManager dataManager;
	dataManager.Put(cppnat::DataId::CLIENT, &i);
	dataManager.Put(cppnat::DataId::SERVER, &j);

	short &k = dataManager[cppnat::DataId::SERVER];
	k = 3;

	cppnat::Buffer<65535, 4> buffer;
	buffer.SetHeader(long(0x04030201));
	long long &value = buffer;
	value = 0x05060708;

	const char *b = buffer.GetBuffer();
	for (int i = 0; i < 64; i++)
		Println(int(b[i]));
}