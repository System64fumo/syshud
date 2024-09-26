#include "backlight.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <sys/inotify.h>
#include <thread>

void syshud_backlight::get_backlight_path(std::string custom_backlight_path) {
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
				continue;
			}
		}
	}
	if (backlight_path.empty())
		std::cout << "Unable to automatically detect your backlight" << std::endl;
}

int syshud_backlight::get_brightness() {
	std::lock_guard<std::mutex> lock(brightness_mutex);
	std::ifstream brightness_file(backlight_path + "/brightness");
	std::ifstream max_brightness_file(backlight_path + "/max_brightness");
	brightness_file >> brightness;
	max_brightness_file >> max_brightness;

	return (brightness / max_brightness) * 100;
}

void syshud_backlight::set_brightness(const double &value) {
	std::ofstream backlight_file(backlight_path + "/brightness", std::ios::trunc);
	backlight_file << (value * max_brightness) / 100;
}

syshud_backlight::syshud_backlight(Glib::Dispatcher* callback, std::string custom_backlight_path) {
	get_backlight_path(custom_backlight_path);

	std::thread monitor_thread([&, callback]() {
		int inotify_fd = inotify_init();
		inotify_add_watch(inotify_fd, backlight_path.c_str(), IN_MODIFY);

		int last_brightness = get_brightness();
		char buffer[1024];

		while (true) {
			ssize_t ret = read(inotify_fd, buffer, 1024);
			(void)ret; // Return value does not matter

			int brightness = get_brightness();
			if (brightness != last_brightness) {
				last_brightness = brightness;
				callback->emit();
			}
		}
	});
	monitor_thread.detach();
}

syshud_backlight::~syshud_backlight() {
	close(inotify_fd);
}
