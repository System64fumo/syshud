#pragma once
#define QT_NO_SIGNALS_SLOTS_KEYWORDS

#include <QWidget>
#include <QSlider>
#include <QLabel>

#include "config.hpp"
#include "wireplumber.hpp"
#include "backlight.hpp"
#include "keytoggles.hpp"

class syshud : public QWidget {
	public:
		syshud(const std::map<std::string, std::map<std::string, std::string>>& cfg);
	
	private:
		std::map<std::string, std::map<std::string, std::string>> config_main;
		char last_reason;

		QLabel *label;
		QSlider *slider;
		QLabel *icon_label;
		QTimer *hide_timer;

		// TODO: Re-Add compile time options to disable some of these
		syshud_wireplumber* listener_wireplumber;
		syshud_keytoggles *listener_keytoggles;
		syshud_backlight *listener_backlight;

		void setup_window();
		void setup_layer_shell();
		void on_change(const char&, const int&);
		void loadStyleSheet(const std::string& filePath);
};

extern "C" {
	syshud *syshud_create(const std::map<std::string, std::map<std::string, std::string>>& cfg);
}