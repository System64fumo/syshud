#include "main.hpp"
#include "window.hpp"
#include "config.hpp"

#include <gtk4-layer-shell.h>
#include <gtkmm/cssprovider.h>
#include <filesystem>

bool timer_ticking = false;
bool first_run = false;

sysvol::sysvol() {
	// Initialize layer shell
	gtk_layer_init_for_window(gobj());
	gtk_layer_set_namespace(gobj(), "sysvol");
	gtk_layer_set_layer(gobj(), GTK_LAYER_SHELL_LAYER_OVERLAY);

	GtkLayerShellEdge edge = GTK_LAYER_SHELL_EDGE_BOTTOM;
	switch (position) {
		case 0:
			edge = GTK_LAYER_SHELL_EDGE_TOP;
			scale_volume.set_value_pos(Gtk::PositionType::RIGHT);
			break;
		case 1:
			edge = GTK_LAYER_SHELL_EDGE_RIGHT;
			scale_volume.set_value_pos(Gtk::PositionType::BOTTOM);
			break;
		case 2:
			edge = GTK_LAYER_SHELL_EDGE_BOTTOM;
			scale_volume.set_value_pos(Gtk::PositionType::RIGHT);
			break;
		case 3:
			edge = GTK_LAYER_SHELL_EDGE_LEFT;
			scale_volume.set_value_pos(Gtk::PositionType::BOTTOM);
			break;

	}
	gtk_layer_set_anchor(gobj(), edge, true);

	// Set layout
	if (position % 2) {
		// Vertical layout
		get_style_context()->add_class("vertical");
		scale_volume.set_orientation(Gtk::Orientation::VERTICAL);
		box_layout.property_orientation().set_value(Gtk::Orientation::VERTICAL);

		set_default_size(height, width);
		if (show_percentage)
			box_layout.append(label_volume);
		box_layout.append(scale_volume);
		if (icon_size != 0)
			box_layout.append(image_volume);
		scale_volume.set_vexpand(true);
		scale_volume.set_inverted(true);
	}
	else {
		// Horizontal layout
		get_style_context()->add_class("horizontal");
		set_default_size(width, height);
		if (icon_size != 0)
			box_layout.append(image_volume);
		box_layout.append(scale_volume);
		if (show_percentage)
			box_layout.append(label_volume);
		scale_volume.set_hexpand(true);
	}

	// Initialize
	set_hide_on_close(true);
	box_layout.get_style_context()->add_class("box_layout");
	box_layout.set_margin(margin);

	// Animations disabled
	if (transition_time == 0)
		set_child(box_layout);

	// Animations enabled
	else {
		set_child(revealer_box);
		revealer_box.set_child(box_layout);

		Gtk::RevealerTransitionType transition_type = Gtk::RevealerTransitionType::SLIDE_UP;
		switch (position) {
			case 0:
				revealer_box.set_valign(Gtk::Align::START);
				transition_type = Gtk::RevealerTransitionType::SLIDE_DOWN;
				break;
			case 1:
				revealer_box.set_halign(Gtk::Align::END);
				transition_type = Gtk::RevealerTransitionType::SLIDE_LEFT;
				break;
			case 2:
				revealer_box.set_valign(Gtk::Align::END);
				transition_type = Gtk::RevealerTransitionType::SLIDE_UP;
				break;
			case 3:
				revealer_box.set_halign(Gtk::Align::START);
				transition_type = Gtk::RevealerTransitionType::SLIDE_RIGHT;
				break;
		}

		revealer_box.set_transition_type(transition_type);
		revealer_box.set_transition_duration(transition_time);
		revealer_box.set_reveal_child(false);
	}
	scale_volume.set_range(0, 100);
	scale_volume.set_increments(5, 10);

	// TODO: Re enable this once code for setting volume has been added
	scale_volume.set_sensitive(false);

	if (icon_size != 0) {
		image_volume.set_pixel_size(icon_size);
		image_volume.set_size_request(height, height);
	}

	if (show_percentage)
		label_volume.set_size_request(height, height);

	on_change();
	scale_volume.signal_value_changed().connect(sigc::mem_fun(*this, &sysvol::on_change));
	dispatcher_callback.connect(sigc::mem_fun(*this, &sysvol::on_callback));

	// Load custom css
	std::string home_dir = getenv("HOME");
	std::string css_path = home_dir + "/.config/sys64/volume.css";

	if (!std::filesystem::exists(css_path)) return;

	auto css = Gtk::CssProvider::create();
	css->load_from_path(css_path);
	auto style_context = get_style_context();
	style_context->add_provider_for_display(property_display(), css, GTK_STYLE_PROVIDER_PRIORITY_USER);
}

void sysvol::on_change() {
	// Check if we should draw the percentage
	if (show_percentage)
		label_volume.set_label(std::to_string(volume) + "\%");

	// Check if we should draw the icons or not
	if (icon_size == 0)
		return;

	// Check if we're muted
	if (muted) {
		image_volume.set_from_icon_name("audio-volume-muted-blocking-symbolic");
		return;
	}

	if (volume >= 75)
		image_volume.set_from_icon_name("audio-volume-high-symbolic");
	else if (volume >= 50)
		image_volume.set_from_icon_name("audio-volume-medium-symbolic");
	else if (volume >= 25)
		image_volume.set_from_icon_name("audio-volume-low-symbolic");
	else if (volume > 0)
		image_volume.set_from_icon_name("audio-volume-muted-symbolic");
}

void sysvol::on_callback() {
	#ifndef PULSEAUDIO
	volume = sysvol_wp->volume;
	muted = sysvol_wp->muted;
	#endif

	on_change();
	scale_volume.set_value(volume);
	if (timer_ticking)
		timeout = desired_timeout;
		
	else if (timeout == 1) {
		if (!first_run) {
			first_run = true;
			return;
		}
		win->show();
		revealer_box.set_reveal_child(true);
		timer_ticking = true;
		timeout = desired_timeout;
		Glib::signal_timeout().connect(sigc::ptr_fun(&timer), 1000);
	}
}

// This is a terrible mess, Dear lord.
bool sysvol::timer() {
	if (timeout == 1) {
		// TODO: Add cancel event
		Glib::signal_timeout().connect([]() {
			win->hide();
			return false;
		}, transition_time);
		win->revealer_box.set_reveal_child(false);
		timer_ticking = false;
		return false;
	}
	else
		timeout--;
	return true;
}
