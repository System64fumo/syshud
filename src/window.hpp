#pragma once
#include <glibmm/dispatcher.h>
#include <gtkmm/window.h>
#include <gtkmm/scale.h>
#include <gtkmm/revealer.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/image.h>

#include "config.hpp"

#ifdef PULSEAUDIO
#include "pulse.hpp"
#else
#include "wireplumber.hpp"
#endif
#include "backlight.hpp"
#include "keytoggles.hpp"

class syshud : public Gtk::Window {

	public:
		syshud(const std::map<std::string, std::map<std::string, std::string>>&);
		~syshud();

		sigc::connection timeout_connection;
		Gtk::Revealer revealer_box;

		#ifdef PULSEAUDIO
		PulseAudio *pa;
		#else
		syshud_wireplumber *syshud_wp;
		#endif
		syshud_backlight *backlight;
		syshud_keytoggles *keytoggle_watcher;


	private:
		std::map<std::string, std::map<std::string, std::string>> config_main;
		bool muted = false;
		bool first_run = false;
		bool timer_ticking = false;
		std::string previous_class;
		int timeout = 1;
		char last_reason;

		Gtk::Box box_layout;
		Gtk::Image image_volume;
		Gtk::Scale scale_volume;
		Gtk::Label label_volume;
		Gtk::RevealerTransitionType transition_type;
		Glib::Dispatcher dispatcher_audio_in;
		Glib::Dispatcher dispatcher_audio_out;
		Glib::Dispatcher dispatcher_backlight;
		Glib::Dispatcher dispatcher_keytoggles;

		void InitLayout();
		void on_change(const char &reason, const int &value);
		bool on_scale_change(Gtk::ScrollType scroll_type, double val);
		void on_audio_callback(const bool &input);
		void on_backlight_callback();
		void on_keytoggle_callback();
		void setup_monitors();
		bool timer();
};

extern "C" {
	syshud *syshud_create(const std::map<std::string, std::map<std::string, std::string>>& cfg);
}
