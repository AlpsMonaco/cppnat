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
}
#endif
