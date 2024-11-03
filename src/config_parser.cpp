#include "config_parser.hpp"
#include <fstream>
#include <sstream>

config_parser::config_parser(const std::string& filename) {
	std::ifstream file(filename);
	std::string line;
	std::string current_section;

	available = file.is_open();

	if (available) {
		while (std::getline(file, line)) {
			line = trim(line);

			if (line.empty() || line[0] == ';' || line[0] == '#') {
				continue;
			}
			else if (line[0] == '[' && line[line.size() - 1] == ']') {
				current_section = line.substr(1, line.size() - 2);
			}
			else {
				size_t delimiter_pos = line.find('=');
				if (delimiter_pos != std::string::npos) {
					std::string key = trim(line.substr(0, delimiter_pos));
					std::string value = trim(line.substr(delimiter_pos + 1));
					data[current_section][key] = value;
				}
			}
		}
		file.close();
	}
	else {
		std::fprintf(stderr, "Unable to open file: %s\n", filename.c_str());
	}
}

std::string config_parser::trim(const std::string& str) {
	const size_t first = str.find_first_not_of(' ');
	if (std::string::npos == first) {
		return str;
	}
	const size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}
