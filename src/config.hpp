#pragma once

/*
	Default config.
	Can be configured instead of using launch arguments.
*/

// Current								Default
inline std::string position = "bottom";	// bottom
inline char orientation = 'h';			// h
inline int width = 300;					// 300
inline int height = 50;					// 50
inline int icon_size = 26;				// 26
inline bool show_percentage = true;		// true
inline std::string margins = "0 0 0 0";	// 0 0 0 0
inline int desired_timeout = 3;			// 3
inline int transition_time = 250;		// 250
inline std::string backlight_path = "";	// ""

// Build time configuration				Description
#define RUNTIME_CONFIG					// Allow the use of runtime arguments
// TODO: add options to select what stuff should be monitored
// eg: audio_in, audio_out, brightness, ect..
