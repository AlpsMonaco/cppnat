#include "../client.h"

int main(int argc, char** argv) {
  for (;;) {
    cppnat::Client client("127.0.0.1", 54432, "127.0.0.1", 33123);
    client.Start();
    std::this_thread::sleep_for(std::chrono::seconds(5));
  };
}