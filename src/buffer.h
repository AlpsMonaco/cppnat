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

		char *Get() { return buffer_; }
		inline const size_t BufferSize() { return bufferSize; }

	protected:
		char data_[bufferSize];
	};
}

#endif