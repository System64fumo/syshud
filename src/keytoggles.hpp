#pragma once
#include <glibmm/dispatcher.h>

class syshud_keytoggles {
	public:
		syshud_keytoggles(Glib::Dispatcher* callback, const std::string& device_path);

		char changed;
		bool caps_lock;
		bool num_lock;

	private:
		bool caps_lock_prev;
		bool num_lock_prev;
};
