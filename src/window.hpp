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

class sysvol : public Gtk::Window {

	public:
		int volume;
		bool muted;
		std::string volume_class;

		#ifdef PULSEAUDIO
		PulseAudio pa;
		#else
		sysvol_wireplumber *sysvol_wp;
		#endif

		Gtk::Scale scale_volume;
		Gtk::Revealer revealer_box;
		Glib::Dispatcher dispatcher_callback;
		void on_callback();
		bool hide_box();
		sysvol();

	private:
		Gtk::Box box_layout;
		Gtk::Image image_volume;
		Gtk::Label label_volume;

		void InitLayout();
		void on_change();
		static bool timer();
};
