#include "main.hpp"
#include "config.hpp"
#include "config_parser.hpp"
#include "git_info.hpp"

#include <filesystem>
#include <signal.h>
#include <dlfcn.h>

QApplication* app = nullptr;
syshud* win = nullptr;
syshud_create_func syshud_create_ptr = nullptr;

void load_libsyshud() {
	void* handle = dlopen("libsyshud.so", RTLD_LAZY);
	if (!handle) {
		std::fprintf(stderr, "Cannot open library: %s\n", dlerror());
		exit(1);
	}

	syshud_create_ptr = (syshud_create_func)dlsym(handle, "syshud_create");

	if (!syshud_create_ptr) {
		std::fprintf(stderr, "Cannot load symbols: %s\n", dlerror());
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
		switch(getopt(argc, argv, "p:o:W:H:i:P:m:t:b:l:k:vh")) {
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

			case 'b':
				config["main"]["backlight"] = optarg;
				continue;

			case 'l':
				config["main"]["listeners"] = optarg;
				continue;

			case 'k':
				config["main"]["keyboard"] = optarg;
				continue;

			case 'v':
				std::printf("Commit: %s\n", GIT_COMMIT_MESSAGE);
				std::printf("Date: %s\n", GIT_COMMIT_DATE);
				return 0;

			case 'h':
			default :
				std::printf("usage:\n");
				std::printf("  syshud [argument...]:\n\n");
				std::printf("arguments:\n");
				std::printf("  -p	Set position\n");
				std::printf("  -o	Set orientation\n");
				std::printf("  -W	Set window width\n");
				std::printf("  -H	Set window Height\n");
				std::printf("  -i	Set icon size\n");
				std::printf("  -P	Hide percentage\n");
				std::printf("  -m	Set margins\n");
				std::printf("  -t	Set timeout\n");
				std::printf("  -b	Set custom backlight path\n");
				std::printf("  -l	Set things to monitor\n");
				std::printf("  -k	Set keyboard path\n");
				std::printf("  -v	Prints version info\n");
				std::printf("  -h	Show this help message\n");
				return 0;

			case -1:
				break;
			}

			break;
	}
	#endif

	// Load the application
	app = new QApplication(argc, argv);

	load_libsyshud();
	win = syshud_create_ptr(config);

	return app->exec();
}
