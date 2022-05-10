#include <fstream>
#include <streambuf>
#include <iostream>
#include <string>
#include <rapidjson/document.h>

using namespace rapidjson;

int main(int argc, char **argv)
{
	std::ifstream ifs("config.json");
	if (!ifs.is_open())
	{
		return -1;
	}
	std::string config((std::istreambuf_iterator<char>(ifs)),
					   std::istreambuf_iterator<char>());
	ifs.close();
	std::cout << config << std::endl;

	Document doc;
	doc.Parse(config.c_str());
	std::cout << doc["server"]["ip"].GetString() << std::endl;
	std::cout << doc["server"]["port"].GetInt() << std::endl;
	std::cout << doc["proxy"]["ip"].GetString() << std::endl;
	std::cout << doc["proxy"]["port"].GetInt() << std::endl;
}