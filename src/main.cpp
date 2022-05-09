#include <vector>
#include <iostream>

template <typename T, typename... Args>
void TestAssign(T &t, Args &&...args)
{
	t = T(std::forward<Args>(args)...);
}

int main(int argc, char **argv)
{
	int i;
	TestAssign(i, 111);
	std::cout << i << std::endl;
}