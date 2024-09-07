#include "keytoggles.hpp"

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <thread>
#include <libevdev/libevdev.h>

syshud_keytoggles::syshud_keytoggles(const std::string& device_path) {
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
	caps_lock = libevdev_get_event_value(dev, EV_LED, LED_CAPSL);
	num_lock = libevdev_get_event_value(dev, EV_LED, LED_NUML);
	scroll_lock = libevdev_get_event_value(dev, EV_LED, LED_SCROLLL);

	std::thread([&, fd, dev]() {
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
			if (ev.type == EV_LED) {
				std::printf("Update!\n");
				if (ev.code == LED_CAPSL)
					caps_lock = ev.value;

				else if (ev.code == LED_NUML)
					num_lock = ev.value;

				else if (ev.code == LED_SCROLLL)
					scroll_lock = ev.value;
			}
		}
		libevdev_free(dev);
		close(fd);
	}).detach();
}
