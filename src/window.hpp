#pragma once
#define QT_NO_SIGNALS_SLOTS_KEYWORDS

#include <QWidget>
#include <QSlider>
#include <QLabel>

#include "config.hpp"

#ifdef FEATURE_WIREPLUMBER
#include "wireplumber.hpp"
#endif
#ifdef FEATURE_PULSEAUDIO
#include "pulse.hpp"
#endif
#ifdef FEATURE_BACKLIGHT
#include "backlight.hpp"
#endif
#ifdef FEATURE_KEYBOARD
#include "keytoggles.hpp"
#endif

class syshud : public QWidget {
	public:
		syshud(const std::map<std::string, std::map<std::string, std::string>>&);
	
	private:
		std::map<std::string, std::map<std::string, std::string>> config_main;
		char last_reason;
		int original_label_size;

		QLabel *label;
		QSlider *slider;
		QLabel *icon_label;
		QTimer *hide_timer;

		#ifdef FEATURE_WIREPLUMBER
		syshud_wireplumber* listener_audio;
		#endif
		#ifdef FEATURE_PULSEAUDIO
		syshud_pulseaudio* listener_audio;
		#endif
		#ifdef FEATURE_BACKLIGHT
		syshud_keytoggles *listener_keytoggles;
		#endif
		#ifdef FEATURE_KEYBOARD
		syshud_backlight *listener_backlight;
		#endif

		void setup_window();
		void setup_layer_shell();
		void setup_listeners();
		void on_change(const char&, const int&);
		void load_qss(const std::string&);
};

extern "C" {
	syshud *syshud_create(const std::map<std::string, std::map<std::string, std::string>>&);
}