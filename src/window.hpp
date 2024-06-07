#pragma once
#include <gtkmm.h>

#ifdef PULSEAUDIO
#include "pulse.hpp"
#else
#include "wireplumber.hpp"
#endif

class sysvol : public Gtk::Window {

	public:
		int volume;
		bool muted;

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
