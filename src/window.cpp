#include "main.hpp"
#include "window.hpp"
#include "css.hpp"
#include "config.hpp"

#include <gtk4-layer-shell.h>
#include <filesystem>
#include <iostream>
#include <thread>

syshud::syshud() {
	// Initialize layer shell
	gtk_layer_init_for_window(gobj());
	gtk_layer_set_namespace(gobj(), "syshud");
	gtk_layer_set_layer(gobj(), GTK_LAYER_SHELL_LAYER_OVERLAY);

	bool edge_top = (config_main.position.find("top") != std::string::npos);
	bool edge_right = (config_main.position.find("right") != std::string::npos);
	bool edge_bottom = (config_main.position.find("bottom") != std::string::npos);
	bool edge_left = (config_main.position.find("left") != std::string::npos);

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
	std::thread audio_thread(&syshud::audio_server, this);
	audio_thread.detach();

	// Set layout
	if (config_main.orientation == 'h') {
		// Horizontal layout
		get_style_context()->add_class("horizontal");
		set_default_size(config_main.width, config_main.height);

		// Check to see if the icon should be shown
		if (config_main.icon_size != 0)
			box_layout.append(image_volume);

		box_layout.append(scale_volume);

		// Check to see if the percentage should be shown
		if (config_main.show_percentage)
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
	else if (config_main.orientation == 'v') {
		// Vertical layout
		get_style_context()->add_class("vertical");
		scale_volume.set_orientation(Gtk::Orientation::VERTICAL);
		box_layout.property_orientation().set_value(Gtk::Orientation::VERTICAL);
		set_default_size(config_main.height, config_main.width);

		// Check to see if the percentage should be shown
		if (config_main.show_percentage)
			box_layout.append(label_volume);

		box_layout.append(scale_volume);

		// Check to see if the icon should be shown
		if (config_main.icon_size != 0)
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
		std::cerr << "Unknown orientation: " << config_main.orientation << std::endl;
		return;
	}

	// Set margins
	std::istringstream iss(config_main.margins);
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
	if (config_main.transition_time == 0)
		set_child(box_layout);

	// Animations enabled
	else {
		set_child(revealer_box);
		revealer_box.set_child(box_layout);
		revealer_box.set_transition_type(transition_type);
		revealer_box.set_transition_duration(config_main.transition_time);
		revealer_box.set_reveal_child(false);
	}

	scale_volume.set_range(0, 100);
	scale_volume.set_increments(5, 10);

	// TODO: Re enable this once code for setting volume has been added
	scale_volume.set_sensitive(false);

	if (config_main.icon_size != 0) {
		image_volume.set_pixel_size(config_main.icon_size);
		image_volume.set_size_request(config_main.height, config_main.height);
	}

	if (config_main.show_percentage)
		label_volume.set_size_request(config_main.height, config_main.height);

	dispatcher_audio_in.connect(sigc::bind(sigc::mem_fun(*this, &syshud::on_audio_callback), true));
	dispatcher_audio_out.connect(sigc::bind(sigc::mem_fun(*this, &syshud::on_audio_callback), false));
	dispatcher_backlight.connect(sigc::mem_fun(*this, &syshud::on_backlight_callback));

	// Load custom css
	std::string home_dir = getenv("HOME");
	std::string css_path = home_dir + "/.config/sys64/hud/style.css";
	css_loader loader(css_path, this);
}

void syshud::on_change(const char &reason, const int &value) {
	timeout_connection.disconnect();

	if (timer_ticking)
		timeout = config_main.desired_timeout;

	else if (timeout == 1) {
		show();

		revealer_box.set_reveal_child(true);
		timer_ticking = true;
		timeout = config_main.desired_timeout;
		Glib::signal_timeout().connect(sigc::mem_fun(*this, &syshud::timer), 1000);
	}

	std::string icon;

	// Map
	std::map<int, std::string> value_levels = {
		{0, "muted"},
		{1, "low"},
		{2, "medium"},
		{3, "high"},
	};

	// Check if we should draw the icons or not
	if (config_main.icon_size == 0)
		return;

	if (reason == 'b') {
		if (value == 0)
			icon = "display-brightness-off-symbolic";
		else
			icon = "display-brightness-" + value_levels[value / 34 + 1] + "-symbolic";
	}
	else if (reason == 'i') {
		if (muted)
			icon = "audio-input-microphone-muted-symbolic";
		else if (value <= 100)
			icon = "audio-input-microphone-" + value_levels[value / 34 + 1] + "-symbolic";
	}
	else if (reason == 'o') {
		if (muted)
			icon = "audio-volume-muted-blocking-symbolic";
		else if (value <= 100)
			icon = "audio-volume-" + value_levels[value / 34 + 1] + "-symbolic";
		else
			icon = "audio-volume-overamplified-symbolic";
	}

	// Set appropiate class
	if (value < 100) {
		get_style_context()->remove_class(previous_class);
		if (muted) {
			previous_class = value_levels[0];
		}
		else {
			previous_class = value_levels[value / 34 + 1];
		}
		get_style_context()->add_class(previous_class);
	}

	// Set the appropiate icon
	if (!icon.empty())
		image_volume.set_from_icon_name(icon);

	// Set the appropiate value
	scale_volume.set_value(value);

	// Check if we should draw the percentage
	if (config_main.show_percentage)
		label_volume.set_label(std::to_string(value) + "\%");
}

void syshud::on_audio_callback(const bool &input) {
	#ifdef PULSEAUDIO
	int volume = pa->volume;
	muted = pa->muted;
	#endif

	#ifndef PULSEAUDIO
	int volume = syshud_wp->volume;
	muted = syshud_wp->muted;
	#endif

	if (!first_run) {
		first_run = true;
			return;
	}

	if (input)
		on_change('i', volume);
	else
		on_change('o', volume);
}

void syshud::on_backlight_callback() {
	on_change('b', backlight->get_brightness());
}

void syshud::audio_server() {
	std::istringstream iss(config_main.monitors);
	std::string monitor;

	Glib::Dispatcher *audio_in = nullptr;
	Glib::Dispatcher *audio_out = nullptr;

	while (std::getline(iss, monitor, ',')) {
		if (monitor == "audio_in") {
			audio_in = &dispatcher_audio_in;
		}
		else if (monitor == "audio_out") {
			audio_out = &dispatcher_audio_out;
		}
		else if (monitor == "brightness") {
			backlight = new syshud_backlight(&dispatcher_backlight, config_main.backlight_path);
		}
		else {
			std::cerr << "Unknown monitor: " << monitor << std::endl;
		}
	}

	if (audio_in != nullptr || audio_out != nullptr) {
		#ifdef PULSEAUDIO
		pa = new PulseAudio(audio_out);
		if (pa->initialize() != 0)
			pa->quit(0);
		#else
		syshud_wp = new syshud_wireplumber(audio_in, audio_out);
		#endif
	}
}

bool syshud::timer() {
	if (timeout == 1) {
		// Start hiding the overlay
		revealer_box.set_reveal_child(false);
		timer_ticking = false;

		// Hide the overlay after it's no longer visible
		timeout_connection = Glib::signal_timeout().connect([&]() {
			hide();
			return false;
		}, config_main.transition_time);

		return false;
	}
	else
		timeout--;
	return true;
}
