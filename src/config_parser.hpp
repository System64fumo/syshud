#pragma once
#include <string>
#include <map>

// INI parser
class config_parser {
	public:
		config_parser(const std::string&);
		std::map<std::string, std::map<std::string, std::string>> data;
		bool available;

	private:
		std::string trim(const std::string&);
};
