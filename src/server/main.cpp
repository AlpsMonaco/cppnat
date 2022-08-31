#include <jsonserializer/json.h>

#include <fstream>
#include <sstream>

#include "../log.h"
#include "../server.h"

int main(int argc, char** argv)
{
    cppnat::Log::SetLogName("nat_server");
    cppnat::Log::Info("cppnat server start");
    std::ifstream ifs("server.json");
    if (!ifs.is_open())
    {
        cppnat::Log::Error("open server.json failed");
        return 1;
    }
    std::string config_file_content((std::istreambuf_iterator<char>(ifs)),
                                    (std::istreambuf_iterator<char>()));
    jsr::Json json;
    auto err = json.Parse(config_file_content);
    if (err)
    {
        cppnat::Log::Error(err.Message());
        return 1;
    }
    std::string ip;
    std::uint16_t port;
    err = json.Unmarshal({
        {"bind_ip", ip},
        {"bind_port", port},
    });
    if (err)
    {
        cppnat::Log::Error(err.Message());
        return 1;
    }
    cppnat::Log::Info("server listening on {}:{}", ip, port);
    cppnat::Server server(ip, port);
    server.Start();
}