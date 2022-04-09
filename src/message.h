#ifndef __CPP_NAT_MESSAGE
#define __CPP_NAT_MESSAGE

namespace cppnat
{
	template <int bufferSize, int headerSize>
	class Buffer
	{
	public:
		Buffer() : data(nullptr)
		{
			memset(this->buffer, 0, bufferSize);
			data = this->buffer + headerSize;
		}
		int BufferSize() { return bufferSize; }
		int HeaderSize() { return headerSize; }

		char *GetBuffer() { return buffer; }
		char *GetData() { return data; }

	protected:
		char buffer[bufferSize];
		char *data;
	};

	enum class MsgCode : unsigned short
	{
		FAILED = 0x00,
		SUCCESS = 0x01,
		REMOTE_NOT_CONNECT = 0x02
	};

	enum class MsgEnum : unsigned short
	{
		ECHO = 0x0000,
		ECHO_VERSION = 0x0001,

		CMD_REQUEST_NAT = 0x0101,
		CMD_TRANSPORT_DATA = 0x0102,
	};

	template <typename T>
	unsigned short GetNumber(T t) { return static_cast<unsigned short>(t); }

	struct RequestNAT
	{
		unsigned short port;
		char ip[16];
	};
}

#endif
