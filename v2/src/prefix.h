#ifndef __CPPNAT_PREFIX_H__
#define __CPPNAT_PREFIX_H__

#define NAMESPACE_CPPNAT_START \
    namespace cppnat           \
    {

#define NAMESPACE_CPPNAT_END }

NAMESPACE_CPPNAT_START

class Protocol
{
public:
    using Size = unsigned short;
    using Cmd = unsigned short;

    static constexpr Size data_offset = sizeof(Size) + sizeof(Cmd);
    static constexpr Size max_size = ~0;
};

NAMESPACE_CPPNAT_END

#endif