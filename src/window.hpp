#pragma once
#include <gtkmm/window.h>
#include <gtkmm/scale.h>
#include <gtkmm/revealer.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/image.h>

#include "config.hpp"

#ifdef AUDIO_PULSEAUDIO
#include "pulse.hpp"
#endif
#ifdef AUDIO_WIREPLUMBER
#include "wireplumber.hpp"
#endif
#ifdef FEATURE_BACKLIGHT
#include "backlight.hpp"
#endif
#ifdef FEATURE_KEYBOARD
#include "keytoggles.hpp"
#endif

class syshud : public Gtk::Window {

	public:
		syshud(const std::map<std::string, std::map<std::string, std::string>>&);
		~syshud();

	private:
		std::map<std::string, std::map<std::string, std::string>> config_main;
		bool muted = false;
		std::string previous_class;
		int timeout = 1;
		char last_reason;
		sigc::connection timeout_connection;

		#ifdef AUDIO_PULSEAUDIO
		PulseAudio *pa;
		#endif

		#ifdef AUDIO_WIREPLUMBER
		syshud_wireplumber *syshud_wp;
		#endif

		#ifdef FEATURE_BACKLIGHT
		syshud_backlight *backlight;
		#endif

		#ifdef FEATURE_KEYBOARD
		syshud_keytoggles *keytoggle_watcher;
		#endif

		Gtk::Box box_layout;
		Gtk::Image image_volume;
		Gtk::Scale scale_volume;
		Gtk::Label label_volume;
		Gtk::Revealer revealer_box;
		Gtk::RevealerTransitionType transition_type;
		Glib::Dispatcher dispatcher_audio_in;
		Glib::Dispatcher dispatcher_audio_out;
		Glib::Dispatcher dispatcher_backlight;
		Glib::Dispatcher dispatcher_keytoggles;

		void InitLayout();
		void on_change(const char&, const int&);
		bool on_scale_change(const Gtk::ScrollType&, const double&);
		void on_audio_callback(const bool&);
		void on_backlight_callback();
		void setup_monitors();
		bool timer();
};

extern "C" {
	syshud *syshud_create(const std::map<std::string, std::map<std::string, std::string>>&);
}
