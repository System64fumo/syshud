#include "css.hpp"

#include <gtkmm/eventcontrollerkey.h>
#include <filesystem>

css_loader::css_loader(std::string path, Gtk::Window *window) {
	if (!std::filesystem::exists(path))
		return;

	auto css = Gtk::CssProvider::create();
	css->load_from_path(path);
	auto style_context = window->get_style_context();
	style_context->add_provider_for_display(window->property_display(), css, GTK_STYLE_PROVIDER_PRIORITY_USER);
}
