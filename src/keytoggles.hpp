#include <string>

class syshud_keytoggles {
	public:
		syshud_keytoggles(const std::string& device_path);
		bool caps_lock;
		bool num_lock;
		bool scroll_lock;
};
