#pragma once
#include <functional>
#include <mutex>
#include <string>

class syshud_backlight {
public:
	using BacklightCallback = std::function<void()>;
	syshud_backlight(BacklightCallback callback, std::string custom_backlight_path = "");
	~syshud_backlight();

	int get_brightness();
	void set_brightness(const int &value);

private:
	double brightness;
	double max_brightness;
	int inotify_fd = -1;
	std::string backlight_path;
	std::mutex brightness_mutex;
	BacklightCallback callback;

	void get_backlight_path(std::string custom_backlight_path);
	void monitor_brightness_changes();
};