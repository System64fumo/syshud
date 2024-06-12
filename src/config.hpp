/*
	Default config.
	Can be configured instead of using launch arguments.
	Runtime configuration can be disabled by deleting #define RUNTIME_CONFIG
*/

#define RUNTIME_CONFIG

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
