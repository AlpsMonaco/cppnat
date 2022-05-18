#include "log.h"
#include <cstdio>
#include <iostream>
#include <rapidjson/document.h>
#include <string>
#include <rapidjson/filereadstream.h>
#include <boost/system.hpp>

using namespace rapidjson;

const char *kConfigFile = "config.json";

bool ParseConfigFile(Document &d)
{
    FILE *fp = fopen(kConfigFile, "rb");
    if (fp == nullptr)
    {
        LOG_ERROR(std::string("open config file failed: ") + kConfigFile);
        return false;
    }
    char buffer[1024];
    FileReadStream is(fp, buffer, sizeof(buffer));
    d.ParseStream(is);
    fclose(fp);
    if (d.HasParseError())
    {
        LOG_ERROR(std::string("parse config file failed: ") + kConfigFile);
        return false;
    }
    return true;
}

struct NatConfig
{
    std::string serverIp;
    unsigned short serverPort;
    std::string proxyIp;
    unsigned short proxyPort;
    NatConfig() : serverPort(0), proxyPort(0) {}

    void Read(Document &d, NatConfig &config)
    {
        if (d.HasMember("server"))
        {
            const Value &serverObject = d["server"];
            config.serverIp = (serverObject.HasMember("ip") && serverObject["ip"].IsString()) ? serverObject["ip"].GetString() : "";
            config.serverPort = (serverObject.HasMember("port") && serverObject["port"].IsUint()) ? serverObject["port"].GetUint() : 0;
        }
        if (d.HasMember("proxy"))
        {
            const Value &proxyObject = d["proxy"];
            config.proxyIp = (proxyObject.HasMember("ip") && proxyObject["ip"].IsString()) ? proxyObject["ip"].GetString() : "";
            config.proxyPort = (proxyObject.HasMember("port") && proxyObject["port"].IsUint()) ? proxyObject["port"].GetUint() : 0;
        }
    }
};

using namespace cppnat;
void Start(NatConfig &config);
#ifdef __CPP_NAT_BUILD_SERVER__
#include "server/server.h"
void Start(NatConfig &config)
{
    if (config.serverIp == "")
    {
        LOG_ERROR("server ip is empty");
        return;
    }
    Server server(config.serverIp.c_str(),
                  config.serverPort);
    server.Start();
}
#else
#include "client/client.h"
void Start(NatConfig &config)
{
    if (config.serverIp == "")
    {
        LOG_ERROR("server ip is empty");
        return;
    }
    if (config.proxyIp == "")
    {
        LOG_ERROR("proxy ip is empty");
        return;
    }

    Client client(config.serverIp.c_str(), config.serverPort,
                  config.proxyIp.c_str(), config.proxyPort);
    for (;;)
    {

        if (!client.Start())
        {
            LOG_ERROR("connect to server failed.");
            std::this_thread::sleep_for(std::chrono::seconds(3));
            LOG_INFO("retrying...");
        }
    }
}
#endif

int main(int argc, char **argv)
{
    Document d;
    if (!ParseConfigFile(d))
        return 1;
    NatConfig config;
    config.Read(d, config);
    Start(config);
}
