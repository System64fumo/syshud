#pragma once
#include <glibmm/dispatcher.h>
#include <mutex>

class syshud_keyboard_backlight {
	public:
		syshud_keyboard_backlight(Glib::Dispatcher* callback, std::string custom_keyboard_backlight_path);
		~syshud_keyboard_backlight();

		int get_brightness();
		void set_brightness(const double &value);

	private:
		double brightness;
		double max_brightness;

		int inotify_fd;
		std::string keyboard_backlight_path;
		std::mutex brightness_mutex;

		void get_keyboard_backlight_path(std::string custom_keyboard_backlight_path);
};
