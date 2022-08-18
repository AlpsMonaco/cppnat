#include "../server.h"

int main(int argc, char** argv) {
    cppnat::Server server("127.0.0.1",54432);
    server.Start();
}