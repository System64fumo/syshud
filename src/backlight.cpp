#include "backlight.hpp"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sys/inotify.h>
#include <thread>
#include <unistd.h>

syshud_backlight::syshud_backlight(BacklightCallback callback, std::string custom_backlight_path) 
	: callback(callback) {
	get_backlight_path(custom_backlight_path);
	std::thread(&syshud_backlight::monitor_brightness_changes, this).detach();
}

syshud_backlight::~syshud_backlight() {
	if (inotify_fd >= 0) {
		close(inotify_fd);
	}
}

void syshud_backlight::get_backlight_path(std::string custom_backlight_path) {
	if (custom_backlight_path != "") {
		backlight_path = custom_backlight_path;
		std::cout << "Backlight: " << backlight_path << std::endl;
		return;
	}

	std::string path = "/sys/class/backlight/";
	for (const auto& entry : std::filesystem::directory_iterator(path)) {
		if (std::filesystem::is_directory(entry.path())) {
			backlight_path = entry.path();
			std::cout << "Backlight: " << backlight_path << std::endl;
			break;
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

void syshud_backlight::set_brightness(const int &value) {
	std::ofstream backlight_file(backlight_path + "/brightness", std::ios::trunc);
	backlight_file << (value * max_brightness) / 100;
}

void syshud_backlight::monitor_brightness_changes() {
	inotify_fd = inotify_init();
	if (inotify_fd < 0) {
		std::cerr << "Failed to initialize inotify" << std::endl;
		return;
	}

	int wd = inotify_add_watch(inotify_fd, (backlight_path + "/brightness").c_str(), IN_MODIFY);
	if (wd < 0) {
		std::cerr << "Failed to add watch for backlight file" << std::endl;
		close(inotify_fd);
		return;
	}

	int last_brightness = get_brightness();
	char buffer[1024];

	while (true) {
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(inotify_fd, &fds);

		struct timeval timeout;
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		int ret = select(inotify_fd + 1, &fds, NULL, NULL, &timeout);
		if (ret < 0) {
			if (errno == EINTR) continue;
			break;
		}
		else if (ret == 0) {
			int current_brightness = get_brightness();
			if (current_brightness != last_brightness) {
				last_brightness = current_brightness;
				callback();
			}
			continue;
		}

		ssize_t len = read(inotify_fd, buffer, sizeof(buffer));
		if (len < 0) {
			if (errno == EINTR) continue;
			break;
		}

		int current_brightness = get_brightness();
		if (current_brightness != last_brightness) {
			last_brightness = current_brightness;
			callback();
		}
	}
}