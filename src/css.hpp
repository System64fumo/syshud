#pragma once
#include <gtkmm/window.h>
#include <gtkmm/cssprovider.h>

class css_loader : public Glib::RefPtr<Gtk::StyleProvider> {
	public:
		css_loader(std::string path, Gtk::Window *window);
};
