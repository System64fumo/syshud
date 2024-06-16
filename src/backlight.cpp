#include "backlight.hpp"
#include "main.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <sys/inotify.h>
#include <glibmm/dispatcher.h>

void sysvol_backlight::get_backlight_path(std::string custom_backlight_path) {
	if (custom_backlight_path != "") {
		backlight_path = custom_backlight_path;
		std::cout << "Backlight: " << backlight_path << std::endl;
		return;
	}
	else {
		std::string path = "/sys/class/backlight/";
		for (const auto& entry : std::filesystem::directory_iterator(path)) {
			if (std::filesystem::is_directory(entry.path())) {
				backlight_path = entry.path();
				std::cout << "Backlight: " << backlight_path << std::endl;
				return;
			}
		}
		std::cout << "Unable to automatically detect your backlight" << std::endl;
	}
}

int sysvol_backlight::get_brightness() {
	std::ifstream brightness_file(backlight_path + "/brightness");
	std::ifstream max_brightness_file(backlight_path + "/max_brightness");
	double brightness;
	double max_brightness;
	brightness_file >> brightness;
	max_brightness_file >> max_brightness;

	return (brightness / max_brightness) * 100;
}

sysvol_backlight::sysvol_backlight(Glib::Dispatcher* callback, std::string custom_backlight_path) {
	get_backlight_path(custom_backlight_path);

	inotify_fd = inotify_init();
	inotify_add_watch(inotify_fd, backlight_path.c_str(), IN_MODIFY);

	const size_t buffer_len = 1024;
	char buffer[buffer_len];
	int last_brightness = get_brightness();

	// TODO: Literally rewrite all of this mess.
	// This is terrible.
	// Also consider not blocking the constructor.
	while (true) {
		read(inotify_fd, buffer, buffer_len);

		int brightness = get_brightness();
		if (brightness != last_brightness) {
			last_brightness = brightness;
			win->brightness = brightness;
			callback->emit();
		}
	}

}

sysvol_backlight::~sysvol_backlight() {
	close(inotify_fd);
}
