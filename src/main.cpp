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

struct test
{
	short a;
	short b;
	short c;
	~test() { Println("~test"); }
	void print() { Println(a, b, c); }
};

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
	value = 0xFFFFFFFFFFFF;

	const char *b = buffer.GetBuffer();
	for (int i = 0; i < 32; i++)
		Println(int(b[i]));
	test &testRef = buffer;
	testRef.print();

	cppnat::MsgNewNatRequest *newNatRequest = new cppnat::MsgNewNatRequest();
	delete newNatRequest;
	Println("endl");
}