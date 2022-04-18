#ifndef __CPP_NAT_MESSAGE_HPP__
#define __CPP_NAT_MESSAGE_HPP

#ifdef _WIN32
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#endif

#include <map>
#include <functional>
#include "util.hpp"

namespace cppnat
{
	/**
	 * message format.
	 * size + cmd + body
	 */
	constexpr int bufferSize = 65535;
	constexpr int headerSize = 4;

	constexpr unsigned short CppNatVersion = 0x0001;
	struct VersionInfo
	{
		unsigned short Version;
		char Msg[15];
	} versionInfo{
		CppNatVersion,
		"hello server"};

	enum class MsgCode : unsigned short
	{
		FAILED = 0x00,
		SUCCESS = 0x01,
	};

	enum class MsgEnum : unsigned short
	{
		ECHO = 0x0000,
		ECHO_VERSION = 0x0001,

		NEW_NAT_REQUEST = 0x0101,
		NEW_NAT_RESULT = 0x0102,
		DATA_TRANSFER = 0x0103,
		SOCKET_CLOSED = 0x0104,
	};

	enum class DataId : unsigned short
	{
		CLIENT = 0x0000,
		SERVER = 0x0001,
		WRITE_BUFFER = 0x0003,

		END,
	};

	template <typename T>
	inline constexpr unsigned short GetNumber(T t)
	{
		return static_cast<const unsigned short>(t);
	}

	template <int bufferSize, int headerSize>
	class Buffer
	{
	public:
		Buffer()
		{
			memset(buffer, 0, bufferSize);
			this->data = buffer + headerSize;
			this->header = buffer;
		}

		template <typename T>
		void SetHeader(T t)
		{
			assert(sizeof(T) == headerSize);
			*reinterpret_cast<T *>(&(header)[0]) = t;
		}

		template <typename T>
		void SetData(T t) { *reinterpret_cast<T *>(&(data)[0]) = t; }

		template <typename T>
		T &Get() { return *reinterpret_cast<T *>(&(data)[0]); }

		char *GetBuffer() { return buffer; }
		const char *GetBuffer() const { return buffer; }

	protected:
		char buffer[bufferSize];
		char *data;
		char *header;
	};

	using WriteBuffer = Buffer<bufferSize, headerSize>;

	template <typename FnPtr>
	class MessageHandler
	{
	public:
		using Callback = FnPtr;
		MessageHandler() {}

		template <typename Fn>
		void AddCallback(MsgEnum msg, Fn fn) { this->callbackMap.emplace(msg, reinterpret_cast<Callback>(fn)); }

	protected:
		std::map<MsgEnum, Callback> callbackMap;
	};

	struct MsgNewNatRequest
	{
		unsigned short fd;
	};

	template <int lockerSize>
	class Locker
	{
	public:
		using any = void *;
		Locker() {}
		~Locker() {}

		void Put(int index, any p)
		{
			assert(index < lockerSize);
			this->locker[index] = p;
		}

		template <typename T>
		T &Get(int index)
		{
			assert(index < lockerSize);
			return *reinterpret_cast<T *>(this->locker[index]);
		}

		template <typename T>
		T &Get(DataId dataId)
		{
			return *reinterpret_cast<T *>(this->locker[GetNumber(dataId)]);
		}

		template <typename T>
		T &operator[](DataId dataId)
		{
			return *reinterpret_cast<T *>(this->locker[GetNumber(dataId)]);
		}

	protected:
		any locker[lockerSize];
	};

	using DataManager = Locker<GetNumber(DataId::END)>;
}

#endif
