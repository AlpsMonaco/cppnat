#include <iostream>
#include <string_view>

std::string_view GetStringView() { return std::string_view("Hello"); }
const char *GetChar()
{
	const char *a = "Hello";
	printf("%p\n", a);
	return a;
}

int main()
{
	std::string_view sv = GetStringView();
	std::cout << sv << std::endl;
	printf("%p\n", GetChar());
	std::string_view sv("Hello");
}