#include "main.hpp"
#include "window.hpp"
#include "config.hpp"
#include "git_info.hpp"

#include <iostream>
#include <thread>
#include <signal.h>

std::thread thread_audio;

void quit(int signum) {
	#ifdef PULSEAUDIO
	// Disconnect pulseaudio
	pa.quit(0);
	#else
	delete(win->sysvol_wp);
	#endif

	thread_audio.join();

	// Remove window
	app->release();
	app->remove_window(*win);
	delete win;
	app->quit();
}

void audio_server() {
#ifdef PULSEAUDIO
	pa = PulseAudio();
	if (pa.initialize() != 0)
		quit(0);
#else
	win->sysvol_wp = new sysvol_wireplumber(&win->dispatcher_audio);
#endif
	if (backlight_path != "-")
		win->backlight = new sysvol_backlight(&win->dispatcher_backlight, backlight_path);
}

int main(int argc, char* argv[]) {
	#ifdef RUNTIME_CONFIG
	// Read launch arguments
	while (true) {
		switch(getopt(argc, argv, "p:co:cW:dH:di:dPm:dt:dT:db:dvh")) {
			case 'p':
				position = optarg;
				continue;

			case 'o':
				orientation = optarg[0];
				continue;

			case 'W':
				width = std::stoi(optarg);
				continue;

			case 'H':
				height = std::stoi(optarg);
				continue;

			case 'i':
				icon_size = std::stoi(optarg);
				continue;

			case 'P':
				show_percentage = false;
				continue;

			case 'm':
				margins = optarg;
				continue;

			case 't':
				desired_timeout = std::stoi(optarg);
				continue;

			case 'T':
				transition_time = std::stoi(optarg);
				continue;

			case 'b':
				backlight_path = optarg;
				continue;

			case 'v':
				std::cout << "Commit: " << GIT_COMMIT_MESSAGE << std::endl;
				std::cout << "Date: " << GIT_COMMIT_DATE << std::endl;
				return 0;

			case 'h':
			default :
				std::cout << "usage:" << std::endl;
				std::cout << "  sysvol [argument...]:\n" << std::endl;
				std::cout << "arguments:" << std::endl;
				std::cout << "  -p	Set position" << std::endl;
				std::cout << "  -o	Set orientation" << std::endl;
				std::cout << "  -W	Set window width" << std::endl;
				std::cout << "  -H	Set window Height" << std::endl;
				std::cout << "  -i	Set icon size" << std::endl;
				std::cout << "  -P	Hide percentage" << std::endl;
				std::cout << "  -m	Set margins" << std::endl;
				std::cout << "  -t	Set timeout" << std::endl;
				std::cout << "  -T	Set transition time" << std::endl;
				std::cout << "  -b	Set custom backlight path" << std::endl;
				std::cout << "  -v	Prints version info" << std::endl;
				std::cout << "  -h	Show this help message" << std::endl;
				return 0;

			case -1:
				break;
			}

			break;
	}
	#endif

	signal(SIGINT, quit);

	app = Gtk::Application::create("funky.sys64.sysvol");
	app->hold();
	win = new sysvol();

	thread_audio = std::thread(audio_server);

	return app->run();
}
