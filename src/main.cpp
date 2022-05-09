#include "server/server.h"
#include "client/client.h"
#include <thread>
#include <iostream>

constexpr char *kAddress = "127.0.0.1";
constexpr char *kTargetAddress = "119.91.79.104";
constexpr unsigned short kListenPort = 8878;
constexpr unsigned short kTargetPort = 3389;

using namespace cppnat;
int main(int argc, char **argv)
{
	std::thread serverThread(
		[]() -> void
		{
			Server server(kAddress, kListenPort);
			if (!server.Start())
			{
				std::cout << server.Error() << std::endl;
				return;
			}
		});

	Client client(kAddress, kListenPort,
				  kTargetAddress, kTargetPort);

	if (!client.Start())
	{
		std::cout << client.Error() << std::endl;
		return -1;
	}
	serverThread.join();
}