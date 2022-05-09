#ifndef __CPP_NAT_UTIL_HPP__
#define __CPP_NAT_UTIL_HPP__

#include <assert.h>
#include <string>
#include <iostream>

namespace cppnat
{

#define ASSERT(what, expression)                                        \
	if (!(expression))                                                  \
	{                                                                   \
		printf("%s:%d:%s:%s\n", __FILE__, __LINE__, what, #expression); \
		fflush(stdout);                                                 \
		assert(expression);                                             \
	}

#define LOG_INFO(what)                                                                                       \
	do                                                                                                       \
	{                                                                                                        \
		std::cout << std::string("[INFO] ") + __FILE__ + ":" + std::to_string(__LINE__) + " " + what + "\n"; \
	} while (0)
#define LOG_ERROR(what)                                                                                       \
	do                                                                                                        \
	{                                                                                                         \
		std::cout << std::string("[ERROR] ") + __FILE__ + ":" + std::to_string(__LINE__) + " " + what + "\n"; \
	} while (0)
#define LOG_DEBUG(what)                                                                                       \
	do                                                                                                        \
	{                                                                                                         \
		std::cout << std::string("[DEBUG] ") + __FILE__ + ":" + std::to_string(__LINE__) + " " + what + "\n"; \
	} while (0)
}
#endif
