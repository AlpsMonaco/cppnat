#include <sstream>
#include <thread>
#include <string>
#include <iostream>
#include "server/server.h"
#include "client/client.h"

class StreamWriter : public std::stringstream
{
public:
	StreamWriter() : std::stringstream() {}
	~StreamWriter() {}

	void Write()
	{
		std::cout << this->str();
	}

	void Reset()
	{
		this->str("");
	}
};

constexpr char *kAddress = "127.0.0.1";
constexpr unsigned short kPort = 8888;

using namespace cppnat;
int main(int argc, char **argv)
{
	std::thread serverThread([]() -> void
							 {
								Server s(kAddress, kPort);
								s.AddHandler(MessageCmd::CMD_ECHO,
								[](ConstPacket packet)->void{
									StreamWriter sw;
									sw << "server: " << "cmd: " << unsigned long long(packet.Cmd())  << std::endl
									<< "size: " << packet.Size() << std::endl
									<< "data: " << packet.Data() << std::endl
									<< std::endl;
									sw.Write();
								}
								);
								if (!s.Start())
								{
									StreamWriter sw;
									sw << s.Errno() << " " << s.Error() << std::endl;
									sw.Write();
								} });

	std::this_thread::sleep_for(std::chrono::seconds(1));
	Client c(kAddress, kPort, 0, 0);
	c.Start();
	if (!c.SendEchoMessage("hello1"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello2"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello3"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello4"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello5"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello6"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello7"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello8"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello9"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello10"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello11"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello12"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello13"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello14"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello15"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello16"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello17"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello18"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello19"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello20"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello21"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello22"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello23"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello24"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello25"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello26"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello27"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello28"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello29"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello30"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello31"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello32"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello33"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello34"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello35"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello36"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello37"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,1"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,2"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,3"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,4"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,5"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,6"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,7"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,8"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,9"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,10"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,11"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,12"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,13"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,14"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,15"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,16"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,17"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,18"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,19"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,20"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,21"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,22"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,23"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,24"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,25"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,26"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,27"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,28"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,29"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,30"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,31"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,32"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,33"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,34"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,35"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,36"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}
	if (!c.SendEchoMessage("hello world,37"))
	{
		std::cout << c.Errno() << c.Error() << std::endl;
	}

	serverThread.join();
	return 0;
}