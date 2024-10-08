#include "main.hpp"
#include "config.hpp"
#include "config_parser.hpp"
#include "git_info.hpp"

#include <filesystem>
#include <iostream>
#include <signal.h>
#include <dlfcn.h>

void quit(int signum) {
	app->remove_window(*win);
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
	if (std::filesystem::exists(std::string(getenv("HOME")) + "/.config/sys64/hud/config.conf"))
		config_path = std::string(getenv("HOME")) + "/.config/sys64/hud/config.conf";
	else if (std::filesystem::exists("/usr/share/sys64/hud/config.conf"))
		config_path = "/usr/share/sys64/hud/config.conf";
	else
		config_path = "/usr/local/share/sys64/hud/config.conf";

	config_parser config(config_path);

	if (config.available) {
		std::string cfg_position = config.get_value("main", "position");
		if (cfg_position != "empty")
			config_main.position = cfg_position;

		std::string cfg_orientation = config.get_value("main", "orientation");
		if (cfg_orientation != "empty")
			config_main.orientation = cfg_orientation[0];

		std::string cfg_width = config.get_value("main", "width");
		if (cfg_width != "empty")
			config_main.width = std::stoi(cfg_width);

		std::string cfg_height = config.get_value("main", "height");
		if (cfg_height != "empty")
			config_main.height = std::stoi(cfg_height);

		std::string cfg_icon_size = config.get_value("main", "icon_size");
		if (cfg_icon_size != "empty")
			config_main.icon_size = std::stoi(cfg_icon_size);

		std::string cfg_percentage = config.get_value("main", "percentage");
		if (cfg_percentage != "empty")
			config_main.show_percentage = (cfg_percentage == "true");

		std::string cfg_margins = config.get_value("main", "margins");
		if (cfg_margins != "empty")
			config_main.margins = cfg_margins;

		std::string cfg_timeout = config.get_value("main", "timeout");
		if (cfg_timeout != "empty")
			config_main.desired_timeout = std::stoi(cfg_timeout);

		std::string cfg_transition = config.get_value("main", "transition");
		if (cfg_transition != "empty")
			config_main.transition_time = std::stoi(cfg_transition);

		std::string cfg_backlight = config.get_value("main", "backlight");
		if (cfg_backlight != "empty" && !cfg_backlight.empty())
			config_main.backlight_path = cfg_backlight;

		std::string cfg_monitors = config.get_value("main", "monitors");
		if (cfg_monitors != "empty")
			config_main.monitors = cfg_monitors;

		std::string cfg_keyboard = config.get_value("main", "keyboard");
		if (cfg_keyboard != "empty")
			config_main.keyboard = cfg_keyboard;
	}
	#endif

	// Read launch arguments
	#ifdef RUNTIME_CONFIG
	while (true) {
		switch(getopt(argc, argv, "p:co:cW:dH:di:dPm:dt:dT:db:dM:dk:dvh")) {
			case 'p':
				config_main.position = optarg;
				continue;

			case 'o':
				config_main.orientation = optarg[0];
				continue;

			case 'W':
				config_main.width = std::stoi(optarg);
				continue;

			case 'H':
				config_main.height = std::stoi(optarg);
				continue;

			case 'i':
				config_main.icon_size = std::stoi(optarg);
				continue;

			case 'P':
				config_main.show_percentage = false;
				continue;

			case 'm':
				config_main.margins = optarg;
				continue;

			case 't':
				config_main.desired_timeout = std::stoi(optarg);
				continue;

			case 'T':
				config_main.transition_time = std::stoi(optarg);
				continue;

			case 'b':
				config_main.backlight_path = optarg;
				continue;

			case 'M':
				config_main.monitors = optarg;
				continue;

			case 'k':
				config_main.keyboard = optarg;
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
	win = syshud_create_ptr(config_main);

	signal(SIGINT, quit);

	return app->run();
}
