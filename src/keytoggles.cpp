#include "keytoggles.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <thread>
#include <libevdev/libevdev.h>

syshud_keytoggles::syshud_keytoggles(KeyTogglesCallback callback, const std::string& device_path) : callback(callback) {
	std::thread([&, device_path, callback]() {
		struct libevdev* dev = nullptr;
		int fd = -1;

		while (true) {
			// Attempt to open the device
			fd = open(device_path.c_str(), O_RDONLY | O_NONBLOCK);
			if (fd < 0) {
				std::this_thread::sleep_for(std::chrono::seconds(1));
				continue;
			}

			// Attempt to initialize libevdev
			if (libevdev_new_from_fd(fd, &dev) < 0) {
				close(fd);
				std::this_thread::sleep_for(std::chrono::seconds(1));
				continue;
			}

			// Get initial states
			caps_lock_prev = libevdev_get_event_value(dev, EV_LED, LED_CAPSL);
			num_lock_prev = libevdev_get_event_value(dev, EV_LED, LED_NUML);

			// Main event loop
			while (true) {
				struct input_event ev;
				int rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);

				if (rc == -EAGAIN) {
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					continue;
				}

				// Check if the device is still connected
				if (rc == -ENODEV)
					break;

				// Process LED events (Caps Lock, Num Lock, Scroll Lock)
				if (ev.type != EV_LED)
					continue;

				if (ev.code == LED_CAPSL)
					caps_lock = ev.value;

				else if (ev.code == LED_NUML)
					num_lock = ev.value;

				if (caps_lock != caps_lock_prev)
					changed = 'c';

				else if (num_lock != num_lock_prev)
					changed = 'n';

				caps_lock_prev = caps_lock;
				num_lock_prev = num_lock;

				callback();
			}

			libevdev_free(dev);
			close(fd);
			dev = nullptr;
			fd = -1;
		}
	}).detach();
}
