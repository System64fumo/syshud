#pragma once
#include <glibmm/dispatcher.h>

class sysvol_backlight {
	public:
		sysvol_backlight(Glib::Dispatcher* callback);
		~sysvol_backlight();

	private:
		int inotify_fd;
		std::string backlight_path;
		int get_brightness();
		void get_backlight_path();
		
};
