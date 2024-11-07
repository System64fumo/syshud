#include "main.hpp"
#include "config.hpp"
#include "config_parser.hpp"
#include "git_info.hpp"

#include <filesystem>
#include <iostream>
#include <signal.h>
#include <dlfcn.h>

void quit(int signum) {
	app->remove_window((Gtk::Window&)*win);
	delete win;
	app->release();
	app->quit();
}

void load_libsyshud() {
	void* handle = dlopen("libsyshud.so", RTLD_LAZY);
	if (!handle) {
		std::cerr << "Cannot open library: " << dlerror() << '\n';
		exit(1);
	}

	syshud_create_ptr = (syshud_create_func)dlsym(handle, "syshud_create");

	if (!syshud_create_ptr) {
		std::cerr << "Cannot load symbols: " << dlerror() << '\n';
		dlclose(handle);
		exit(1);
	}
}

int main(int argc, char* argv[]) {
	// Load the config
	#ifdef CONFIG_FILE
	std::string config_path;
	std::map<std::string, std::map<std::string, std::string>> config;
	std::map<std::string, std::map<std::string, std::string>> config_usr;

	bool cfg_sys = std::filesystem::exists("/usr/share/sys64/hud/config.conf");
	bool cfg_sys_local = std::filesystem::exists("/usr/local/share/sys64/hud/config.conf");
	bool cfg_usr = std::filesystem::exists(std::string(getenv("HOME")) + "/.config/sys64/hud/config.conf");

	// Load default config
	if (cfg_sys)
		config_path = "/usr/share/sys64/hud/config.conf";
	else if (cfg_sys_local)
		config_path = "/usr/local/share/sys64/hud/config.conf";
	else
		std::fprintf(stderr, "No default config found, Things will get funky!\n");

	config = config_parser(config_path).data;

	// Load user config
	if (cfg_usr)
		config_path = std::string(getenv("HOME")) + "/.config/sys64/hud/config.conf";
	else
		std::fprintf(stderr, "No user config found\n");

	config_usr = config_parser(config_path).data;

	// Merge configs
	for (const auto& [key, nested_map] : config_usr)
		for (const auto& [inner_key, inner_value] : nested_map)
			config[key][inner_key] = inner_value;

	// Sanity check
	if (!(cfg_sys || cfg_sys_local || cfg_usr)) {
		std::fprintf(stderr, "No config available, Something ain't right here.");
		return 1;
	}
	#endif

	// Read launch arguments
	#ifdef RUNTIME_CONFIG
	while (true) {
		switch(getopt(argc, argv, "p:o:W:H:i:P:m:t:T:b:M:k:vh")) {
			case 'p':
				config["main"]["position"] = optarg;
				continue;

			case 'o':
				config["main"]["orientation"] = optarg;
				continue;

			case 'W':
				config["main"]["width"] = optarg;
				continue;

			case 'H':
				config["main"]["height"] = optarg;
				continue;

			case 'i':
				config["main"]["icon-size"] = optarg;
				continue;

			case 'P':
				config["main"]["percentage"] = optarg;
				continue;

			case 'm':
				config["main"]["margins"] = optarg;
				continue;

			case 't':
				config["main"]["timeout"] = optarg;
				continue;

			case 'T':
				config["main"]["transition"] = optarg;
				continue;

			case 'b':
				config["main"]["backlight"] = optarg;
				continue;

			case 'M':
				config["main"]["monitors"] = optarg;
				continue;

			case 'k':
				config["main"]["keyboard"] = optarg;
				continue;

			case 'v':
				std::cout << "Commit: " << GIT_COMMIT_MESSAGE << std::endl;
				std::cout << "Date: " << GIT_COMMIT_DATE << std::endl;
				return 0;

			case 'h':
			default :
				std::cout << "usage:" << std::endl;
				std::cout << "  syshud [argument...]:\n" << std::endl;
				std::cout << "arguments:" << std::endl;
				std::cout << "  -p	Set position" << std::endl;
				std::cout << "  -o	Set orientation" << std::endl;
				std::cout << "  -W	Set window width" << std::endl;
				std::cout << "  -H	Set window Height" << std::endl;
				std::cout << "  -i	Set icon size" << std::endl;
				std::cout << "  -P	Hide percentage" << std::endl;
				std::cout << "  -m	Set margins" << std::endl;
				std::cout << "  -t	Set timeout" << std::endl;
				std::cout << "  -T	Set transition time" << std::endl;
				std::cout << "  -b	Set custom backlight path" << std::endl;
				std::cout << "  -M	Set things to monitor" << std::endl;
				std::cout << "  -k	Set keyboard path" << std::endl;
				std::cout << "  -v	Prints version info" << std::endl;
				std::cout << "  -h	Show this help message" << std::endl;
				return 0;

			case -1:
				break;
			}

			break;
	}
	#endif

	// Load the application
	app = Gtk::Application::create("funky.sys64.syshud");
	app->hold();

	load_libsyshud();
	win = syshud_create_ptr(config);

	signal(SIGINT, quit);

	return app->run();
}
