#include <string_view>
#include <iostream>
#include <map>
#include <functional>

using namespace std;

struct Transfer
{
	Transfer(const char *p) : p_(p) {}
	struct Proxy
	{
		Proxy(const char *p) { this->p_ = p; }

		template <typename T>
		operator const T &() { return *reinterpret_cast<const T *>(p_); }

		template <typename T>
		operator T &() = delete;

		template <typename T>
		operator T() = delete;

	protected:
		const char *p_;
	};

	Proxy Data()
	{
		return Proxy(p_);
	}

protected:
	const char *p_;
};

using FuncPointer = void *;

void PrintInt(int a)
{
	cout << "int " << a << std::endl;
}

void PrintDouble(double d)
{
	cout << "double " << d << std::endl;
}

std::map<unsigned short, FuncPointer> funcMap;

template <typename T>
void Call(unsigned short cmd, T t)
{
	reinterpret_cast<void (*)(T)>(funcMap[cmd])(t);
}

int main(int argc, char **argv)
{
	funcMap[0] = PrintInt;
	funcMap[1] = PrintDouble;
	int i = 1;
	double b = 2.033;
	Call(0, i);
	Call(1, b);

	const std::function<bool()> &f = [i]() -> bool
	{ return true; };
	f();
}