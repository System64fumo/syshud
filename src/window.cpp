#include "window.hpp"

#include <gtk4-layer-shell.h>
#include <gtkmm/cssprovider.h>
#include <filesystem>
syshud::syshud(const std::map<std::string, std::map<std::string, std::string>>& cfg) :
	config_main(cfg), muted(false), timeout(-1), last_reason('s') {

	// Initialize layer shell
	gtk_layer_init_for_window(gobj());
	gtk_layer_set_namespace(gobj(), "syshud");
	gtk_layer_set_layer(gobj(), GTK_LAYER_SHELL_LAYER_OVERLAY);

	bool edge_top = (config_main["main"]["position"].find("top") != std::string::npos);
	bool edge_right = (config_main["main"]["position"].find("right") != std::string::npos);
	bool edge_bottom = (config_main["main"]["position"].find("bottom") != std::string::npos);
	bool edge_left = (config_main["main"]["position"].find("left") != std::string::npos);

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
	icon_theme = Gtk::IconTheme::get_for_display(Gdk::Display::get_default());
	box_layout.get_style_context()->add_class("box_layout");
	setup_listeners();

	// Set layout
	if (config_main["main"]["orientation"][0] == 'h') {
		// Horizontal layout
		get_style_context()->add_class("horizontal");
		set_default_size(std::stoi(config_main["main"]["width"]), std::stoi(config_main["main"]["height"]));

		// Check to see if the icon should be shown
		if (std::stoi(config_main["main"]["icon-size"]) != 0)
			box_layout.append(image_volume);

		box_layout.append(scale_volume);

		// Check to see if the percentage should be shown
		if (config_main["main"]["show-percentage"] == "true")
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
	else if (config_main["main"]["orientation"][0] == 'v') {
		// Vertical layout
		get_style_context()->add_class("vertical");
		scale_volume.set_orientation(Gtk::Orientation::VERTICAL);
		box_layout.property_orientation().set_value(Gtk::Orientation::VERTICAL);
		set_default_size(std::stoi(config_main["main"]["height"]), std::stoi(config_main["main"]["width"]));

		// Check to see if the percentage should be shown
		if (config_main["main"]["show-percentage"] == "true")
			box_layout.append(label_volume);

		box_layout.append(scale_volume);

		// Check to see if the icon should be shown
		if (std::stoi(config_main["main"]["icon-size"]) != 0)
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
		std::fprintf(stderr, "Unknown orientation: %s\n", config_main["main"]["orientation"].c_str());
		return;
	}

	// Set margins
	std::istringstream iss(config_main["main"]["margins"]);
	std::string margin_str;
	int count = 0;

	while (std::getline(iss, margin_str, ' ')) {
		const int& margin = std::stoi(margin_str);

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
	if (std::stoi(config_main["main"]["transition-time"]) == 0)
		set_child(box_layout);

	// Animations enabled
	else {
		set_child(revealer_box);
		revealer_box.set_child(box_layout);
		revealer_box.set_transition_type(transition_type);
		revealer_box.set_transition_duration(std::stoi(config_main["main"]["transition-time"]));
		revealer_box.set_reveal_child(false);
	}

	scale_volume.set_range(0, 100);
	scale_volume.set_increments(5, 10);
	scale_volume.signal_change_value().connect(sigc::mem_fun(*this, &syshud::on_scale_change), true);

	if (std::stoi(config_main["main"]["icon-size"]) != 0) {
		image_volume.set_pixel_size(std::stoi(config_main["main"]["icon-size"]));
		image_volume.set_size_request(std::stoi(config_main["main"]["height"]), std::stoi(config_main["main"]["height"]));
	}

	if (config_main["main"]["show-percentage"] == "true")
		label_volume.set_size_request(std::stoi(config_main["main"]["height"]), std::stoi(config_main["main"]["height"]));

	dispatcher_audio_in.connect(sigc::bind(sigc::mem_fun(*this, &syshud::on_audio_callback), true));
	dispatcher_audio_out.connect(sigc::bind(sigc::mem_fun(*this, &syshud::on_audio_callback), false));

	#ifdef FEATURE_BACKLIGHT
	dispatcher_backlight.connect([&]() {
		scale_volume.show();
		on_change('b', listener_backlight->get_brightness());
	});
	#endif

	#ifdef FEATURE_KEYBOARD
	dispatcher_keytoggles.connect([&]() {
		scale_volume.hide();
		on_change('k', listener_keytoggles->changed);
	});
	#endif

	const std::string& style_path = "/usr/share/sys64/hud/style.css";
	const std::string& style_path_usr = std::string(getenv("HOME")) + "/.config/sys64/hud/style.css";

	// Load base style
	if (std::filesystem::exists(style_path)) {
		auto css = Gtk::CssProvider::create();
		css->load_from_path(style_path);
		get_style_context()->add_provider_for_display(property_display(), css, GTK_STYLE_PROVIDER_PRIORITY_USER);
	}
	// Load user style
	if (std::filesystem::exists(style_path_usr)) {
		auto css = Gtk::CssProvider::create();
		css->load_from_path(style_path_usr);
		get_style_context()->add_provider_for_display(property_display(), css, GTK_STYLE_PROVIDER_PRIORITY_USER);
	}
}

syshud::~syshud() {
	#ifdef AUDIO_PULSEAUDIO
	delete listener_audio;
	#else
	delete listener_audio;
	#endif
}

void syshud::on_change(const char& reason, const int& value) {
	hide_overlay_connection.disconnect();
	timeout_connection.disconnect();
	if (last_reason == 's') {
		last_reason = reason;
		return;
	}

	last_reason = reason;

	if (timeout < 0) {
		show();
		revealer_box.set_reveal_child(true);
	}

	timeout = std::stof(config_main["main"]["timeout"]);
	timeout_connection = Glib::signal_timeout().connect(sigc::mem_fun(*this, &syshud::timer), static_cast<unsigned int>(std::round(timeout * 1000)));

	// Check if we should draw the icons or not
	if (std::stoi(config_main["main"]["icon-size"]) == 0)
		return;

	// Map
	std::map<int, std::string> value_levels = {
		{0, "muted"},
		{1, "low"},
		{2, "medium"},
		{3, "high"},
	};

	std::string label;

	// Audio input
	if (reason == 'i') {
		if (muted)
			icon = "audio-input-microphone-muted-symbolic";
		else if (value <= 100)
			icon = "audio-input-microphone-" + value_levels[value / 34 + 1] + "-symbolic";
	}

	// Audio output
	else if (reason == 'o') {
		if (muted)
			icon = "audio-volume-muted-symbolic";
		else if (value <= 100)
			icon = "audio-volume-" + value_levels[value / 34 + 1] + "-symbolic";
		else
			icon = "audio-volume-overamplified-symbolic";
	}

	#ifdef FEATURE_BACKLIGHT
	else if (reason == 'b') {
		if (value == 0)
			icon = "display-brightness-off-symbolic";
		else
			icon = "display-brightness-" + value_levels[value / 34 + 1] + "-symbolic";
	}
	#endif

	#ifdef FEATURE_KEYBOARD
	else if (reason == 'k') {
		if (value == 'c') {
			label = "Caps Lock";
			icon = listener_keytoggles->caps_lock ? "capslock-enabled-symbolic" : "capslock-disabled-symbolic";
		}
		else if (value == 'n') {
			label = "Num Lock";
			icon = listener_keytoggles->caps_lock ? "numlock-enabled-symbolic" : "numlock-disabled-symbolic";
		}
	}
	#endif

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

	// Show data
	check_icon();
	image_volume.set_from_icon_name(icon);
	scale_animator.animate_property(&scale_volume,
		PROPERTY_SCALE_VALUE, value, 0.25);
	if (config_main["main"]["show-percentage"] == "true")
		label_volume.set_label(label);
}

bool syshud::on_scale_change(const Gtk::ScrollType&, const double& val) {
	if (false) {} // Dud

	#ifdef AUDIO_WIREPLUMBER
	else if (last_reason == 'i')
		listener_audio->set_volume(false, val);
	else if (last_reason == 'o')
		listener_audio->set_volume(true, val);
	#endif

	#ifdef FEATURE_BACKLIGHT
	else if (last_reason == 'b')
		listener_backlight->set_brightness(val);
	#endif

	return false;
}

void syshud::on_audio_callback(const bool& input) {
	const int& volume = listener_audio->volume;
	muted = listener_audio->muted;

	scale_volume.show();
	if (input)
		on_change('i', volume);
	else
		on_change('o', volume);
}

void syshud::setup_listeners() {
	std::istringstream iss(config_main["main"]["listeners"]);
	std::string listener;

	Glib::Dispatcher* audio_in = nullptr;
	Glib::Dispatcher* audio_out = nullptr;

	while (std::getline(iss, listener, ',')) {
		if (listener == "audio_in")
			audio_in = &dispatcher_audio_in;
		else if (listener == "audio_out")
			audio_out = &dispatcher_audio_out;

		#ifdef FEATURE_BACKLIGHT
		else if (listener == "backlight")
			listener_backlight = new syshud_backlight(&dispatcher_backlight, config_main["main"]["backlight-path"]);
		#endif

		#ifdef FEATURE_KEYBOARD
		else if (listener == "keyboard" && config_main["main"]["keyboard-path"] != "" && config_main["main"]["orientation"][0] == 'h')
			listener_keytoggles = new syshud_keytoggles(&dispatcher_keytoggles, config_main["main"]["keyboard-path"]);
		#endif

		else
			std::fprintf(stderr, "Unknown listener: %s\n", listener.c_str());
	}

	if (audio_in || audio_out) {
		#ifdef AUDIO_PULSEAUDIO
		listener_audio = new syshud_pulseaudio(audio_out);
		#else
		listener_audio = new syshud_wireplumber(audio_in, audio_out);
		#endif
	}
}

void syshud::check_icon() {
	if (!icon_theme->has_icon(icon)) {
		std::fprintf(stderr, "[Warning] Icon: %s is missing\n", icon.c_str());

		if (last_reason == 'i' || last_reason == 'o')
			icon = "audio-volume-high-symbolic";
		else if (last_reason == 'b')
			icon = "display-brightness-symbolic";
		else if (last_reason == 'k')
			icon = "keyboard-brightness-symbolic";
	}
}

bool syshud::timer() {
	// Start hiding the overlay
	revealer_box.set_reveal_child(false);

	// Hide the overlay after it's no longer visible
	hide_overlay_connection = Glib::signal_timeout().connect([&]() {
		hide();
		return false;
	}, std::stoi(config_main["main"]["transition-time"]));

	timeout = -1;

	return false;
}

extern "C" {
	syshud *syshud_create(const std::map<std::string, std::map<std::string, std::string>>& cfg) {
		return new syshud(cfg);
	}
}
