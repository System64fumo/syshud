#pragma once

// Build time configuration				Description
#define RUNTIME_CONFIG					// Allow the use of runtime arguments
#define CONFIG_FILE						// Allow the use of a config file

// Default config
struct config_hud {
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
};
