#ifndef __CPPNAT_MESSAGES_H__
#define __CPPNAT_MESSAGES_H__

#include "prefix.h"
#include "message_handler.h"

NAMESPACE_CPPNAT_START

namespace message
{
    struct MsgResponse
    {
        std::uint16_t id;
        char msg[120];
    };

    namespace server
    {
        void RecvMsgResponse(const MsgResponse& msg, MessageWriter& writer)
        {
        }
    } // namespace server

    namespace client
    {

    }

    struct MsgNewConnection
    {
        std::uint16_t id;
    };
} // namespace message

NAMESPACE_CPPNAT_END

#endif