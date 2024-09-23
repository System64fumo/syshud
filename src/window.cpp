#include "window.hpp"
#include "css.hpp"

#include <gtk4-layer-shell.h>
#include <filesystem>
#include <thread>

syshud::syshud(const config_hud &cfg) {
	config_main = cfg;

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
		std::fprintf(stderr, "Verry funny arguments you got there\n");
		std::fprintf(stderr, "Would be a shame if.. The program crashed right?\n");
		exit(1);
	}
	else if (!edge_top && !edge_right && !edge_bottom && !edge_left) {
		std::fprintf(stderr, "You sure you specified valid arguments?\n");
		std::fprintf(stderr, "Valid arguments: \"top right bottom left\"\n");
		exit(1);
	}

	// Initialize
	set_name("syshud");
	set_hide_on_close(true);
	box_layout.get_style_context()->add_class("box_layout");
	std::thread(&syshud::setup_monitors, this).detach();

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
		std::fprintf(stderr, "Unknown orientation: %d\n", config_main.orientation);
		return;
	}

	// Set margins
	std::istringstream iss(config_main.margins);
	std::string margin_str;
	int count = 0;

	while (std::getline(iss, margin_str, ' ')) {
		int margin = std::stoi(margin_str);

		if (count == 0)
			gtk_layer_set_margin(gobj(), GTK_LAYER_SHELL_EDGE_TOP, margin);
		else if (count == 1)
			gtk_layer_set_margin(gobj(), GTK_LAYER_SHELL_EDGE_RIGHT, margin);
		else if  (count == 2)
			gtk_layer_set_margin(gobj(), GTK_LAYER_SHELL_EDGE_BOTTOM, margin);
		else if (count == 3)
			gtk_layer_set_margin(gobj(), GTK_LAYER_SHELL_EDGE_LEFT, margin);
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

	scale_volume.signal_change_value().connect(sigc::mem_fun(*this, &syshud::on_scale_change), true);

	if (config_main.icon_size != 0) {
		image_volume.set_pixel_size(config_main.icon_size);
		image_volume.set_size_request(config_main.height, config_main.height);
	}

	if (config_main.show_percentage)
		label_volume.set_size_request(config_main.height, config_main.height);

	dispatcher_audio_in.connect(sigc::bind(sigc::mem_fun(*this, &syshud::on_audio_callback), true));
	dispatcher_audio_out.connect(sigc::bind(sigc::mem_fun(*this, &syshud::on_audio_callback), false));
	dispatcher_backlight.connect(sigc::mem_fun(*this, &syshud::on_backlight_callback));
	dispatcher_keytoggles.connect(sigc::mem_fun(*this, &syshud::on_keytoggle_callback));

	// Load custom css
	std::string style_path;
	if (std::filesystem::exists(std::string(getenv("HOME")) + "/.config/sys64/hud/style.css"))
		style_path = std::string(getenv("HOME")) + "/.config/sys64/hud/style.css";
	else if (std::filesystem::exists("/usr/share/sys64/hud/style.css"))
		style_path = "/usr/share/sys64/hud/style.css";
	else
		style_path = "/usr/local/share/sys64/hud/style.css";

	css_loader loader(style_path, this);
}

syshud::~syshud() {
	#ifdef PULSEAUDIO
	delete pa;
	#else
	delete syshud_wp;
	#endif
}

void syshud::on_change(const char &reason, const int &value) {
	timeout_connection.disconnect();
	last_reason = reason;

	if (timer_ticking)
		timeout = config_main.desired_timeout;

	else if (timeout == 1) {
		show();

		revealer_box.set_reveal_child(true);
		timer_ticking = true;
		timeout = config_main.desired_timeout;
		Glib::signal_timeout().connect(sigc::mem_fun(*this, &syshud::timer), 1000);
	}

	// Check if we should draw the icons or not
	if (config_main.icon_size == 0)
		return;

	// Map
	std::map<int, std::string> value_levels = {
		{0, "muted"},
		{1, "low"},
		{2, "medium"},
		{3, "high"},
	};

	std::string label;
	std::string icon;
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
	else if (reason == 'k') {
		if (value == 'c') {
			label = "Caps Lock";
			icon = keytoggle_watcher->caps_lock ? "capslock-enabled-symbolic" : "capslock-disabled-symbolic";
		}
		else if (value == 'n') {
			label = "Num Lock";
			icon = keytoggle_watcher->caps_lock ? "numlock-enabled-symbolic" : "numlock-disabled-symbolic";
		}
	}

	if (reason != 'k') {
		label = std::to_string(value) + "\%";

		// Set appropiate class
		box_layout.get_style_context()->remove_class(previous_class);
	
		if (muted && reason !=  'b')
			previous_class = value_levels[0];
		else
			previous_class = value_levels[std::clamp(value, 0, 100) / 34 + 1];
	
		box_layout.get_style_context()->add_class(previous_class);
	}

	// Set the appropiate icon
	image_volume.set_from_icon_name(icon);

	// Set the appropiate value
	scale_volume.set_value(value);

	// Check if we should draw the percentage
	if (config_main.show_percentage)
		label_volume.set_label(label);
}

bool syshud::on_scale_change(Gtk::ScrollType scroll_type, double val) {
	if (last_reason == 'b') {
		// TODO: Add backlight set code
	}
	else if (last_reason == 'i') {
		#ifndef PULSEAUDIO
		syshud_wp->set_volume(false, val);
		#endif
	}
	else if (last_reason == 'o') {
		#ifndef PULSEAUDIO
		syshud_wp->set_volume(true, val);
		#endif
	}
	return false;
}

void syshud::on_audio_callback(const bool &input) {
	#ifdef PULSEAUDIO
	int volume = pa->volume;
	muted = pa->muted;
	#else
	int volume = syshud_wp->volume;
	muted = syshud_wp->muted;
	#endif

	if (!first_run) {
		first_run = true;
			return;
	}

	scale_volume.show();
	if (input)
		on_change('i', volume);
	else
		on_change('o', volume);
}

void syshud::on_backlight_callback() {
	scale_volume.show();
	on_change('b', backlight->get_brightness());
}

void syshud::on_keytoggle_callback() {
	scale_volume.hide();
	on_change('k', keytoggle_watcher->changed);
}

void syshud::setup_monitors() {
	std::istringstream iss(config_main.monitors);
	std::string monitor;

	Glib::Dispatcher *audio_in = nullptr;
	Glib::Dispatcher *audio_out = nullptr;

	// Sleep a little, This *should* help with some weird edge cases..
	usleep(100 * 1000);

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
		else if (monitor == "keyboard" && config_main.keyboard != "" && config_main.orientation == 'h') {
			keytoggle_watcher = new syshud_keytoggles(&dispatcher_keytoggles, config_main.keyboard);
		}
		else {
			std::fprintf(stderr, "Unknown monitor: %s\n", monitor.c_str());
		}
	}

	if (audio_in != nullptr || audio_out != nullptr) {
		#ifdef PULSEAUDIO
		pa = new PulseAudio(audio_out);
		if (pa->initialize() != 0)
			delete pa;
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

extern "C" {
	syshud *syshud_create(const config_hud &cfg) {
		return new syshud(cfg);
	}
}
