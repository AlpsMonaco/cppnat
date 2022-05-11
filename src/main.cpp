#include "server/server.h"
#include "client/client.h"
#include <thread>

const char *kAddress = "127.0.0.1";
const int kPort = 9999;
const char *kAddress2 = "192.168.1.202";
const int kPort2 = 33123;

int main(int argc, char **argv)
{
    using namespace cppnat;
    Server s(kAddress, kPort);
    Client c(kAddress, kPort, kAddress2, kPort2);

    std::thread([&]() -> void
                { s.Start(); })
        .detach();
    for (;;)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        c.Start();
    }
}