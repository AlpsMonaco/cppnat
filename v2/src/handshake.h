#ifndef __CPPNAT_HANDSHAKE_H__
#define __CPPNAT_HANDSHAKE_H__

NAMESPACE_CPPNAT_START

#include "prefix.h"

class Handshake
{
public:
    static constexpr std::string_view server_message = "cpp_nat_hello_server";
    static constexpr std::string_view client_message = "cpp_nat_hello_client";
};

NAMESPACE_CPPNAT_END

#endif