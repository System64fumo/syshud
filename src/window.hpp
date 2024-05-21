#pragma once
#include <gtkmm.h>

class sysvol : public Gtk::Window {

	public:
		int volume;
		bool muted;

		Gtk::Scale scale_volume;
		Gtk::Revealer revealer_box;
		Glib::Dispatcher m_Dispatcher;
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
