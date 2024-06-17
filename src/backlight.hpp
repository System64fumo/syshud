#pragma once
#include <glibmm/dispatcher.h>
#include <mutex>

class syshud_backlight {
	public:
		syshud_backlight(Glib::Dispatcher* callback, std::string custom_backlight_path);
		~syshud_backlight();

		int get_brightness();

	private:
		int inotify_fd;
		std::string backlight_path;
		std::mutex brightness_mutex;

		void get_backlight_path(std::string custom_backlight_path);
};
