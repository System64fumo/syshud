#pragma once
#include <functional>
#include <string>

class syshud_keytoggles {
	public:
		using KeyTogglesCallback = std::function<void()>;
		syshud_keytoggles(KeyTogglesCallback callback, const std::string& device_path);

		char changed;

		bool caps_lock;
		bool num_lock;

	private:
		KeyTogglesCallback callback;
		bool caps_lock_prev;
		bool num_lock_prev;
};
