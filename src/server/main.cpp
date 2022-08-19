#include "../server.h"
#include "../log.h"

int main(int argc, char** argv) {
    cppnat::Log::Info("cppnat server start");
    cppnat::Server server("127.0.0.1",54432);
    server.Start();
}