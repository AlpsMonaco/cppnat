#ifndef __CPP_NAT_UTIL_HPP__
#define __CPP_NAT_UTIL_HPP__
#include <assert.h>
#include <string.h>

#define ASSERT(what, expression)                                        \
	if (!(expression))                                                  \
	{                                                                   \
		printf("%s:%d:%s:%s\n", __FILE__, __LINE__, what, #expression); \
		fflush(stdout);                                                 \
		assert(expression);                                             \
	}

#endif

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
