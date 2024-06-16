#pragma once

#include <glibmm/dispatcher.h>
#include <gtkmm/window.h>
#include <gtkmm/scale.h>
#include <gtkmm/revealer.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/image.h>

#ifdef PULSEAUDIO
#include "pulse.hpp"
#else
#include "wireplumber.hpp"
#endif

#include "backlight.hpp"

class sysvol : public Gtk::Window {

	public:
		int volume;
		bool muted;
		bool input;
		int brightness;

		#ifdef PULSEAUDIO
		PulseAudio pa;
		#else
		sysvol_wireplumber *sysvol_wp;
		#endif
		sysvol_backlight *backlight;

		Gtk::Scale scale_volume;
		Gtk::Revealer revealer_box;
		Gtk::RevealerTransitionType transition_type;
		Glib::Dispatcher dispatcher_audio;
		Glib::Dispatcher dispatcher_backlight;
		void on_audio_callback();
		void on_backlight_callback();
		bool hide_box();
		sysvol();

	private:
		std::string output_class;
		std::string input_class;

		Gtk::Box box_layout;
		Gtk::Image image_volume;
		Gtk::Label label_volume;

		void InitLayout();
		void on_change(bool reason_backlight);
		static bool timer();
};
