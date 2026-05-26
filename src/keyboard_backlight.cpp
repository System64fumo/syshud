#include "keyboard_backlight.hpp"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <sys/inotify.h>
#include <thread>

void syshud_keyboard_backlight::get_keyboard_backlight_path(std::string custom_keyboard_backlight_path) {
	if (custom_keyboard_backlight_path != "") {
		keyboard_backlight_path = custom_keyboard_backlight_path;
		std::cout << "Keyboard backlight: " << keyboard_backlight_path << std::endl;
		return;
	}
	else {
		// Search for keyboard backlight in /sys/class/leds/
		std::string path = "/sys/class/leds/";
		for (const auto& entry : std::filesystem::directory_iterator(path)) {
			if (std::filesystem::is_directory(entry.path())) {
				std::string entry_name = entry.path().filename().string();
				// Look for entries that contain "kbd" or "keyboard" or "::kbd_backlight"
				if (entry_name.find("kbd") != std::string::npos ||
					entry_name.find("keyboard") != std::string::npos ||
					entry_name.find("kbd_backlight") != std::string::npos) {
					// Verify it has brightness and max_brightness files
					if (std::filesystem::exists(entry.path() / "brightness") &&
						std::filesystem::exists(entry.path() / "max_brightness")) {
						keyboard_backlight_path = entry.path();
						std::cout << "Keyboard backlight: " << keyboard_backlight_path << std::endl;
						return;
					}
				}
			}
		}
	}
	if (keyboard_backlight_path.empty())
		std::cout << "Unable to automatically detect your keyboard backlight" << std::endl;
}

int syshud_keyboard_backlight::get_brightness() {
	std::lock_guard<std::mutex> lock(brightness_mutex);
	std::ifstream brightness_file(keyboard_backlight_path + "/brightness");
	std::ifstream max_brightness_file(keyboard_backlight_path + "/max_brightness");
	brightness_file >> brightness;
	max_brightness_file >> max_brightness;

	return (brightness / max_brightness) * 100;
}

void syshud_keyboard_backlight::set_brightness(const double &value) {
	std::ofstream backlight_file(keyboard_backlight_path + "/brightness", std::ios::trunc);
	backlight_file << (value * max_brightness) / 100;
}

syshud_keyboard_backlight::syshud_keyboard_backlight(Glib::Dispatcher* callback, std::string custom_keyboard_backlight_path) {
	get_keyboard_backlight_path(custom_keyboard_backlight_path);

	std::thread monitor_thread([&, callback]() {
		int inotify_fd = inotify_init();
		inotify_add_watch(inotify_fd, keyboard_backlight_path.c_str(), IN_MODIFY);

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

syshud_keyboard_backlight::~syshud_keyboard_backlight() {
	close(inotify_fd);
}
