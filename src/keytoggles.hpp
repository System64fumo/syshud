#pragma once
#include <atomic>
#include <glibmm/dispatcher.h>

class syshud_keytoggles {
	public:
		syshud_keytoggles(Glib::Dispatcher* callback, const std::string& device_path);

		std::atomic<char> changed{};
		std::atomic<bool> caps_lock{false};
		std::atomic<bool> num_lock{false};

	private:
		std::atomic<bool> caps_lock_prev{false};
		std::atomic<bool> num_lock_prev{false};
};
