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

	template <typename T>
	inline constexpr unsigned short GetNumber(T t) { return static_cast<const unsigned short>(t); }

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

		template <typename T>
		operator T &() { return *reinterpret_cast<T *>(&(data)[0]); }

		template <typename T>
		operator T() = delete;

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

	enum class DataId : unsigned short
	{
		CLIENT = 0x0000,
		SERVER,
		WRITE_BUFFER,
		FD_SET,
		PRIVATE_SOCKADDR,
		BIND_MAP,

		END,
	};

	template <typename LockerType>
	class LockerProxy
	{
	public:
		LockerProxy(LockerType &locker) : locker(locker) {}
		void SetDataId(DataId dataId) { this->dataId = dataId; }

		template <typename T>
		operator T &() { return *reinterpret_cast<T *>(locker.Fetch(dataId)); }

		template <typename T>
		operator T() = delete;

	protected:
		DataId dataId;
		LockerType &locker;
	};

	template <int lockerSize>
	class Locker
	{
	public:
		using any = void *;
		using LockerInstance = Locker<lockerSize>;

		Locker() : proxy(*this) {}
		~Locker() {}

		LockerInstance &operator=(const LockerInstance &rhs) = delete;

		void Put(DataId dataId, any p)
		{
			assert(dataId < DataId::END);
			this->locker[static_cast<int>(dataId)] = p;
		}

		any Fetch(DataId dataId)
		{
			assert(dataId < DataId::END);
			return this->locker[static_cast<int>(dataId)];
		}

		LockerProxy<LockerInstance> &Get(DataId dataId)
		{
			proxy.SetDataId(dataId);
			return proxy;
		}

		LockerProxy<LockerInstance> &operator[](DataId dataId)
		{
			proxy.SetDataId(dataId);
			return proxy;
		}

	protected:
		any locker[lockerSize];
		LockerProxy<LockerInstance> proxy;
	};

	using DataManager = Locker<GetNumber(DataId::END)>;
	using BindMap = std::map<SOCKET, SOCKET>;
}

#endif
