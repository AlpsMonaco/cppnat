#include <sstream>
#include <thread>
#include <string>
#include <iostream>
#include "util.h"
#include "server/server.h"
#include "client/client.h"

constexpr char *kAddress = "127.0.0.1";
constexpr unsigned short kPort = 8888;

using namespace cppnat;
int main(int argc, char **argv)
{
	std::thread serverThread([]() -> void
							 {
								Server s(kAddress, kPort);
								if (!s.Start())
								{
									StreamWriter sw;
									sw << s.Errno() << " " << s.Error() << std::endl;
									sw.Write();
								} });

	std::this_thread::sleep_for(std::chrono::seconds(1));
	Client c(kAddress, kPort, 0, 0);
	if (!c.Start())
	{
		std::cout << c.Errno() << " " << c.Error() << std::endl;
		return 1;
	}
	std::string s;
	for (size_t i = 0; i < 1000; i++)
	{
		s += std::to_string(i);
		if (!c.SendEchoMessage(s))
		{
			std::cout << c.Errno() << " " << c.Error() << std::endl;
			return 1;
		}
	}
	serverThread.join();
	return 0;
}