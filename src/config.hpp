#pragma once

// Build time configuration				Description
#define RUNTIME_CONFIG					// Allow the use of runtime arguments

/*
	Default config.
	Can be configured instead of using launch arguments.
*/

inline struct config {
	std::string position = "bottom";
	char orientation = 'h';
	int width = 300;
	int height = 50;
	int icon_size = 26;
	bool show_percentage = true;
	std::string margins = "0 0 0 0";
	int desired_timeout = 3;
	int transition_time = 250;
	std::string backlight_path = "";
	std::string monitors = "audio_in,audio_out,brightness";
} config_main;
