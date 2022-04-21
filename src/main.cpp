#include "locker.h"

int main()
{
	using namespace cppnat;
	int i = 1;
	unsigned short j = 2;
	long long k = 3;

	Locker<3> locker;
	locker.Put<int>(0, &i);
	locker.Put<unsigned short>(1, &j);
	locker.Put<long long>(2, &k);

	int &ri = locker[0];
	unsigned short &rj = locker[1];
	unsigned long long &rk = locker[2];
	Println(ri, rj, rk);
}