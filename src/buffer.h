#ifndef __BUFFER_H__
#define __BUFFER_H__

namespace cppnat
{
	template <size_t bufferSize>
	class Buffer
	{
	public:
		Buffer() {}
		~Buffer() {}

		inline char *Get() { return data_; }
		inline const size_t BufferSize() { return bufferSize; }

	protected:
		char data_[bufferSize];
	};
}

#endif