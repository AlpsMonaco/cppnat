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

struct MyStruct
{
	long long dda1;
	unsigned char dda;
	int type;
};

struct TestStruct
{
	char a;
	short b;
	short c;
	char s[255];
};

int main(int argc, char *argv[])
{
	MyStruct ms{1, 'a', 1};
	Println(sizeof(double));
	Println(sizeof(char));
	Println(sizeof(int));
	Println(ms.dda1, ms.dda, ms.type);
	char data[64];
	memset(data, 0, 64);
	memcpy(data, &ms, sizeof(ms));
	data[sizeof(ms)] = 0;
	for (int i = 0; i < 64; i++)
	{
		Print(unsigned int(data[i]), "");
	}
}