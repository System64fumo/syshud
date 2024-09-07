#include "keytoggles.hpp"

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <thread>
#include <libevdev/libevdev.h>

syshud_keytoggles::syshud_keytoggles(Glib::Dispatcher* callback, const std::string& device_path) {
	struct libevdev* dev = nullptr;
	int fd = open(device_path.c_str() , O_RDONLY | O_NONBLOCK);

	if (fd < 0) {
		std::fprintf(stderr, "Failed to open device\n");
		return;
	}

	if (libevdev_new_from_fd(fd, &dev) < 0) {
		close(fd);
		return;
	}

	// Get initial states
	caps_lock_prev = libevdev_get_event_value(dev, EV_LED, LED_CAPSL);
	num_lock_prev = libevdev_get_event_value(dev, EV_LED, LED_NUML);

	std::thread([&, fd, dev, callback]() {
		while (true) {
			struct input_event ev;
			int rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);

			// TODO: THIS IS TERRIBLE!!!!
			// This wastes a lot of battery and CPU power
			// *But it's a good temporary solution for now*
			if (rc == -EAGAIN) {
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				continue;
			}

			// Process LED events (Caps Lock, Num Lock, Scroll Lock)
			if (ev.type != EV_LED)
				continue;

			// What the hell is this mess???
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

			callback->emit();
		}
		libevdev_free(dev);
		close(fd);
	}).detach();
}
