#ifndef __CPP_NAT_MESSAGE
#define __CPP_NAT_MESSAGE

namespace cppnat
{
	/**
	 * message format.
	 * size + cmd + body
	 */
	constexpr int bufferSize = 65535;
	constexpr int headerSize = 2;
	constexpr int cmdSize = 2;
	constexpr int bodySize = bufferSize - headerSize - cmdSize;

	constexpr unsigned short CppNatVersion = 0x0001;
	struct VersionInfo
	{
		unsigned short Version;
		char Msg[15];
	} versionInfo{
		CppNatVersion,
		"hello server"};

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
	};

	enum class MsgEnum : unsigned short
	{
		ECHO = 0x0000,
		ECHO_VERSION = 0x0001,

		ON_NEW_CONNECTION = 0x0101,
		ON_DATA_TRANSFER = 0x0102,
	};

	template <typename T>
	inline unsigned short GetNumber(T t) { return static_cast<unsigned short>(t); }

	struct NewNat
	{
		unsigned short Fd;
	};

	struct NewNatResult
	{
		unsigned short Code;
		unsigned short Fd;
	};

	constexpr int FdSize = 2;
	constexpr int DataCarrySize = bodySize - FdSize;
	struct DataTransfer
	{
		unsigned short Fd;
		char Data[DataCarrySize];
	};

	struct SocketClosed
	{
		unsigned short Fd;
	};
}

#endif
