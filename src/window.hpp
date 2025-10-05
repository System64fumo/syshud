#pragma once
#include <gtkmm/window.h>
#include <gtkmm/scale.h>
#include <gtkmm/revealer.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/image.h>

#include "animations.hpp"
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
		bool muted;
		std::string previous_class;
		float timeout;
		char last_reason;
		sigc::connection hide_overlay_connection;
		sigc::connection timeout_connection;
		property_animator scale_animator;

		#ifdef AUDIO_PULSEAUDIO
		syshud_pulseaudio *listener_audio;
		#endif

		#ifdef AUDIO_WIREPLUMBER
		syshud_wireplumber *listener_audio;
		#endif

		#ifdef FEATURE_BACKLIGHT
		syshud_backlight *listener_backlight;
		#endif

		#ifdef FEATURE_KEYBOARD
		syshud_keytoggles *listener_keytoggles;
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
		void setup_listeners();
		bool timer();
};

extern "C" {
	syshud *syshud_create(const std::map<std::string, std::map<std::string, std::string>>&);
}
