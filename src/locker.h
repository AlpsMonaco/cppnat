#ifndef __LOCKER_H__
#define __LOCKER_H__

#include "util.h"

namespace cppnat
{
	class LockerItemType
	{
	public:
		LockerItemType() {}
		virtual ~LockerItemType() {}
		virtual size_t ID() = 0;
		virtual void *Get() = 0;

		template <typename T>
		inline operator T &()
		{
			assert(this->ID() == typeid(T).hash_code());
			return *reinterpret_cast<T *>(this->Get());
		}

		template <typename T>
		operator T() = delete;
	};

	template <typename T>
	class LockerItem : public LockerItemType
	{
	public:
		LockerItem(T *t) : p(t) {}
		~LockerItem() {}
		virtual size_t ID() override { return typeid(T).hash_code(); }
		virtual void *Get() override { return this->p; }

	protected:
		void *p;
	};

	template <size_t size>
	class Locker
	{
	public:
		Locker()
		{
			for (size_t i = 0; i < size; i++)
				items[i] = nullptr;
		}
		~Locker()
		{
			for (size_t i = 0; i < size; i++)
				delete items[i];
		}

		template <typename T>
		void Put(size_t index, void *p)
		{
			assert(index < size);
			items[index] = new LockerItem<T>(reinterpret_cast<T *>(p));
		}

		LockerItemType &operator[](size_t index)
		{
			assert(index < size);
			return *items[index];
		}

	protected:
		LockerItemType *items[size];
	};

	template <size_t size>
	class LockerNoVirtual
	{
	public:
		using LockerNoVirtualInstance = LockerNoVirtual<size>;
		class Proxy
		{
		public:
			Proxy(LockerNoVirtualInstance &locker) : locker(locker) {}
			Proxy &operator()(size_t index)
			{
				this->index = index;
				return *this;
			}

			template <typename T>
			operator T &()
			{
				assert(this->index < size);
				assert(locker.IdMap[this->index] == typeid(T).hash_code());
				return *reinterpret_cast<T *>(locker.items[this->index]);
			}

			template <typename T>
			operator T() = delete;

		protected:
			LockerNoVirtualInstance &locker;
			size_t index;
		};

		using item = void *;
		LockerNoVirtual() : proxy(*this)
		{
			for (size_t i = 0; i < size; i++)
			{
				items[i] = nullptr;
				IdMap[i] = 0;
			}
		}

		template <typename T>
		void Put(size_t index, item p)
		{
			assert(index < size);
			items[index] = p;
			IdMap[index] = typeid(T).hash_code();
		}

		Proxy &operator[](size_t index)
		{
			assert(index < size);
			return proxy(index);
		}

	protected:
		Proxy proxy;
		item items[size];
		size_t IdMap[size];
	};
}

#endif