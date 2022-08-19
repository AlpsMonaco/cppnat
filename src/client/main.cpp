#include <jsonserializer/json.h>

#include <fstream>
#include <sstream>

#include "../client.h"
#include "../log.h"

int main(int argc, char** argv) {
  cppnat::Log::SetLogName("nat_client");
  cppnat::Log::Info("cppnat client start");
  std::ifstream ifs("client.json");
  if (!ifs.is_open()) {
    cppnat::Log::Error("open client.json failed");
    return 1;
  }
  std::string config_file_content((std::istreambuf_iterator<char>(ifs)),
                                  (std::istreambuf_iterator<char>()));
  jsr::Json json;
  auto err = json.Parse(config_file_content);
  if (err) {
    cppnat::Log::Error(err.Message());
    return 1;
  }
  std::string server_ip;
  std::uint16_t server_port;
  std::string proxy_ip;
  std::uint16_t proxy_port;
  err = json.Unmarshal({
      {"server_ip", server_ip},
      {"server_port", server_port},
      {"proxy_ip", proxy_ip},
      {"proxy_port", proxy_port},
  });
  if (err) {
    cppnat::Log::Error(err.Message());
    return 1;
  }
  for (;;) {
    cppnat::Client client(server_ip, server_port, proxy_ip, proxy_port);
    client.Start();
    std::this_thread::sleep_for(std::chrono::seconds(5));
  };
}