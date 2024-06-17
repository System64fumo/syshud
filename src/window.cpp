#include "main.hpp"
#include "window.hpp"
#include "config.hpp"

#include <gtk4-layer-shell.h>
#include <gtkmm/cssprovider.h>
#include <filesystem>
#include <iostream>

bool timer_ticking = false;
bool first_run = false;

syshud::syshud() {
	// Initialize layer shell
	gtk_layer_init_for_window(gobj());
	gtk_layer_set_namespace(gobj(), "syshud");
	gtk_layer_set_layer(gobj(), GTK_LAYER_SHELL_LAYER_OVERLAY);

	bool edge_top = (position.find("top") != std::string::npos);
	bool edge_right = (position.find("right") != std::string::npos);
	bool edge_bottom = (position.find("bottom") != std::string::npos);
	bool edge_left = (position.find("left") != std::string::npos);

	gtk_layer_set_anchor(gobj(), GTK_LAYER_SHELL_EDGE_TOP, edge_top);
	gtk_layer_set_anchor(gobj(), GTK_LAYER_SHELL_EDGE_RIGHT, edge_right);
	gtk_layer_set_anchor(gobj(), GTK_LAYER_SHELL_EDGE_BOTTOM, edge_bottom);
	gtk_layer_set_anchor(gobj(), GTK_LAYER_SHELL_EDGE_LEFT, edge_left);

	if ((edge_top && edge_bottom) || (edge_right && edge_left)) {
		std::cerr << "Verry funny arguments you got there" << std::endl;
		std::cerr << "Would be a shame if.. The program crashed right?" << std::endl;
		app->quit();
	}
	else if (!edge_top && !edge_right && !edge_bottom && !edge_left) {
		std::cerr << "You sure you specified valid arguments?" << std::endl;
		std::cerr << "Valid arguments: \"top right bottom left\"" << std::endl;
		app->quit();
	}

	// Initialize
	set_hide_on_close(true);
	box_layout.get_style_context()->add_class("box_layout");

	// Set layout
	if (orientation == 'h') {
		// Horizontal layout
		get_style_context()->add_class("horizontal");
		set_default_size(width, height);

		// Check to see if the icon should be shown
		if (icon_size != 0)
			box_layout.append(image_volume);

		box_layout.append(scale_volume);

		// Check to see if the percentage should be shown
		if (show_percentage)
			box_layout.append(label_volume);

		scale_volume.set_hexpand(true);
		scale_volume.set_value_pos(Gtk::PositionType::RIGHT);

		// Revealer shenanigans
		if (edge_top) {
			transition_type = Gtk::RevealerTransitionType::SLIDE_DOWN;
			revealer_box.set_valign(Gtk::Align::START);
		}
		else if (edge_bottom) {
			transition_type = Gtk::RevealerTransitionType::SLIDE_UP;
			revealer_box.set_valign(Gtk::Align::END);
		}
	}
	else if (orientation == 'v') {
		// Vertical layout
		get_style_context()->add_class("vertical");
		scale_volume.set_orientation(Gtk::Orientation::VERTICAL);
		box_layout.property_orientation().set_value(Gtk::Orientation::VERTICAL);
		set_default_size(height, width);

		// Check to see if the percentage should be shown
		if (show_percentage)
			box_layout.append(label_volume);

		box_layout.append(scale_volume);

		// Check to see if the icon should be shown
		if (icon_size != 0)
			box_layout.append(image_volume);

		scale_volume.set_vexpand(true);
		scale_volume.set_inverted(true);
		scale_volume.set_value_pos(Gtk::PositionType::BOTTOM);

		// Revealer shenanigans
		if (edge_right) {
			transition_type = Gtk::RevealerTransitionType::SLIDE_LEFT;
			revealer_box.set_halign(Gtk::Align::END);
		}
		else if (edge_left) {
			transition_type = Gtk::RevealerTransitionType::SLIDE_RIGHT;
			revealer_box.set_halign(Gtk::Align::START);
		}
	}
	else {
		std::cerr << "Unknown orientation: " << orientation << std::endl;
		return;
	}

	// Set margins
	std::istringstream iss(margins);
	std::string margin;
	int count = 0;
	while (std::getline(iss, margin, ' ')) {
		if (count == 0)
			gtk_layer_set_margin(gobj(), GTK_LAYER_SHELL_EDGE_TOP, std::stoi(margin));
		else if (count == 1)
			gtk_layer_set_margin(gobj(), GTK_LAYER_SHELL_EDGE_RIGHT, std::stoi(margin));
		else if  (count == 2)
			gtk_layer_set_margin(gobj(), GTK_LAYER_SHELL_EDGE_BOTTOM, std::stoi(margin));
		else if (count == 3)
			gtk_layer_set_margin(gobj(), GTK_LAYER_SHELL_EDGE_LEFT, std::stoi(margin));
		count++;
	}

	// Animations disabled
	if (transition_time == 0)
		set_child(box_layout);

	// Animations enabled
	else {
		set_child(revealer_box);
		revealer_box.set_child(box_layout);
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

	on_change(false);
	dispatcher_audio.connect(sigc::mem_fun(*this, &syshud::on_audio_callback));
	dispatcher_backlight.connect(sigc::mem_fun(*this, &syshud::on_backlight_callback));

	// Load custom css
	std::string home_dir = getenv("HOME");
	std::string css_path = home_dir + "/.config/sys64/hud.css";

	if (!std::filesystem::exists(css_path)) return;

	auto css = Gtk::CssProvider::create();
	css->load_from_path(css_path);
	auto style_context = get_style_context();
	style_context->add_provider_for_display(property_display(), css, GTK_STYLE_PROVIDER_PRIORITY_USER);
}

void syshud::on_change(bool reason_backlight) {
	std::map<int, std::string> output_icons = {
		{0, "audio-volume-muted-symbolic"},
		{1, "audio-volume-low-symbolic"},
		{2, "audio-volume-medium-symbolic"},
		{3, "audio-volume-high-symbolic"},
		{4, "audio-volume-overamplified-symbolic"},
	};


	int value;

	// Check if we should draw the icons or not
	if (icon_size == 0)
		return;

	if (reason_backlight) {
		value = brightness;
		if (brightness > 75) {
			image_volume.set_from_icon_name("display-brightness-high-symbolic");
		}
		else if (brightness >= 50) {
			image_volume.set_from_icon_name("display-brightness-medium-symbolic");
		}
		else if (brightness >= 25) {
			image_volume.set_from_icon_name("display-brightness-low-symbolic");
		}
		else if (brightness >= 0) {
			image_volume.set_from_icon_name("display-brightness-off-symbolic");
		}
	}
	else {
		value = volume;
		get_style_context()->remove_class(output_class);

		// TODO: Replace this with a map
		// TODO: Redo the entire class thing
		// Temporarily intentionally broken
		if (muted) {
			output_class = "muted-blocking";
			input_class = "muted";
		}
		else if (volume > 100) {
			input_class = "high";
		}
		else if (volume >= 75) {
			input_class = "high";
		}
		else if (volume >= 50) {
			input_class = "medium";
		}
		else if (volume >= 25) {
			input_class = "medium";
		}
		else if (volume > 0) {
			input_class = "low";
		}

		get_style_context()->add_class(output_class);

		if (!input)
			image_volume.set_from_icon_name(output_icons[(volume - 1) / 25]);
		else
			image_volume.set_from_icon_name("audio-input-microphone-" + input_class + "-symbolic");
	}

	// Check if we should draw the percentage
	if (show_percentage)
		label_volume.set_label(std::to_string(value) + "\%");


}

void syshud::on_audio_callback() {
	#ifndef PULSEAUDIO
	volume = syshud_wp->volume;
	muted = syshud_wp->muted;
	input = syshud_wp->input;
	#endif

	on_change(false);
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

void syshud::on_backlight_callback() {
	brightness = backlight->get_brightness();
	on_change(true);
	scale_volume.set_value(brightness);

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
bool syshud::timer() {
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
