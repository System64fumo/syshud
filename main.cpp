#include "main.hpp"
#include "config.hpp"
#include "pulse.hpp"

#include <gtk4-layer-shell.h>
#include <gtkmm/cssprovider.h>
#include <glibmm.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <filesystem>
#include <string>

#include <stdio.h>
#include <signal.h>

bool timer_ticking = false;
PulseAudio pa;
std::thread thread_audio;

// This is a terrible mess, Dear lord.
bool timer() {
	if (timeout == 1) {
		win->hide();
		timer_ticking = false;
		return false;
	}
	else
		timeout--;
    return true;
}

void sysvol::on_callback() {
	scale_volume.set_value(volume);
	if (timer_ticking)
		timeout = desired_timeout;
		
	else if (timeout == 1) {
		win->show();
		timer_ticking = true;
		timeout = desired_timeout;
		Glib::signal_timeout().connect(sigc::ptr_fun(&timer), 1000);
	}
}

// TODO: Replace if else statements with something better
// What is this??? Am i becoming the next yandere dev?
void sysvol::on_change() {
	int volume = scale_volume.get_value();
	if (show_percentage)
		label_volume.set_label(std::to_string(volume) + "\%");
	if (volume >= 75)
		image_volume.set_from_icon_name("audio-volume-high-symbolic");
	else if (volume >= 50)
		image_volume.set_from_icon_name("audio-volume-medium-symbolic");
	else if (volume >= 25)
		image_volume.set_from_icon_name("audio-volume-low-symbolic");
	else if (volume > 0)
		image_volume.set_from_icon_name("audio-volume-muted-symbolic");
}


void audio_server() {
	pa = PulseAudio();
	pa.initialize();
	pa.run();
}

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
	gtk_layer_set_margin(gobj(), edge, margin);

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
		box_layout.append(image_volume);
		scale_volume.set_vexpand(true);
		scale_volume.set_inverted(true);
	}
	else {
		// Horizontal layout
		get_style_context()->add_class("horizontal");
		set_default_size(width, height);
		box_layout.append(image_volume);
		box_layout.append(scale_volume);
		if (show_percentage)
			box_layout.append(label_volume);
		scale_volume.set_hexpand(true);
	}

	// Initialize
	set_hide_on_close(true);
	set_child(box_layout);
	image_volume.set_pixel_size(icon_size);

	scale_volume.set_range(0, 100);
	scale_volume.set_increments(5, 10);

	// TODO: Re enable this once code for setting volume has been added
	scale_volume.set_sensitive(false);

	image_volume.set_size_request(height, height);
	if (show_percentage)
		label_volume.set_size_request(height, height);

	on_change();
	scale_volume.signal_value_changed().connect(sigc::mem_fun(*this, &sysvol::on_change));
	m_Dispatcher.connect(sigc::mem_fun(*this, &sysvol::on_callback));

	// Load custom css
	std::string home_dir = getenv("HOME");
	std::string css_path = home_dir + "/.config/sys64/volume.css";

	if (!std::filesystem::exists(css_path)) return;

	auto css = Gtk::CssProvider::create();
	css->load_from_path(css_path);
	auto style_context = get_style_context();
	style_context->add_provider_for_display(property_display(), css, GTK_STYLE_PROVIDER_PRIORITY_USER);
}

void quit(int signum) {
	// Disconnect audio
	pa.quit(0);
	thread_audio.join();

	// Remove window
	app->release();
	app->remove_window(*win);
	delete win;
	app->quit();
}

int main(int argc, char* argv[]) {

	// Read launch arguments
	while (true) {
		switch(getopt(argc, argv, "p:dW:dH:di:dPm:dt:dh")) {
			case 'p':
				position = std::stoi(optarg);
				continue;

			case 'W':
				width = std::stoi(optarg);
				continue;

			case 'H':
				height = std::stoi(optarg);
				continue;

			case 'i':
				icon_size = std::stoi(optarg);
				continue;

			case 'P':
				show_percentage = false;
				continue;

			case 'm':
				margin = std::stoi(optarg);
				continue;

			case 't':
				desired_timeout = std::stoi(optarg);
				continue;

			case 'h':
			default :
				printf("usage:\n");
				printf("  sysvol [argument...]:\n\n");
				printf("arguments:\n");
				printf("  -p	Set position\n");
				printf("  -W	Set window width\n");
				printf("  -H	Set window Height\n");
				printf("  -i	Set icon size\n");
				printf("  -P	Hide percentage\n");
				printf("  -m	Set margins\n");
				printf("  -t	Set timeout\n");
				printf("  -h	Show this help message\n");
				return 0;

			case -1:
				break;
			}

			break;
	}

	signal(SIGINT, quit);

	app = Gtk::Application::create("funky.sys64.sysvol");
	app->hold();
	win = new sysvol();

	thread_audio = std::thread(audio_server);

	return app->run();
}
