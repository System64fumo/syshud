#include "main.hpp"
#include "config.hpp"
#include <gtk4-layer-shell.h>
#include <gtkmm/cssprovider.h>
#include <glibmm.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <filesystem>
#include <string>

#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <pulse/pulseaudio.h>

int timeout = 1;
int volume = 0;
bool timer_ticking = false;
const char* default_sink;
sysvol* win;

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

class PulseAudio {
private:
	pa_mainloop* _mainloop;
	pa_mainloop_api* _mainloop_api;
	pa_context* _context;
	pa_signal_event* _signal;

public:
	PulseAudio()
		: _mainloop(NULL), _mainloop_api(NULL), _context(NULL), _signal(NULL)
	{
	}


	bool initialize() {
		_mainloop = pa_mainloop_new();
		if (!_mainloop)
			return false;

		_mainloop_api = pa_mainloop_get_api(_mainloop);

		if (pa_signal_init(_mainloop_api) != 0)
			return false;

		_signal = pa_signal_new(SIGINT, exit_signal_callback, this);
		if (!_signal)
			return false;
		signal(SIGPIPE, SIG_IGN);

		_context = pa_context_new(_mainloop_api, "sysvol");
		if (!_context)
			return false;

		if (pa_context_connect(_context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) < 0)
			return false;

		pa_context_set_state_callback(_context, context_state_callback, this);

		return true;
	}


	int run() {
		int ret = 1;
		if (pa_mainloop_run(_mainloop, &ret) < 0)
			return ret;

		return ret;
	}

	void quit(int ret = 0) {
		_mainloop_api->quit(_mainloop_api, ret);
	}

	void destroy() {
		if (_context) {
			pa_context_unref(_context);
			_context = NULL;
		}

		if (_signal) {
			pa_signal_free(_signal);
			pa_signal_done();
			_signal = NULL;
		}

		if (_mainloop) {
			pa_mainloop_free(_mainloop);
			_mainloop = NULL;
			_mainloop_api = NULL;
		}
	}

	~PulseAudio() {
		destroy();
	}

private:
	static void exit_signal_callback(pa_mainloop_api *m, pa_signal_event *e, int sig, void *userdata) {
		PulseAudio* pa = (PulseAudio*)userdata;
		if (pa) pa->quit();
	}

	static void context_state_callback(pa_context *c, void *userdata) {
		assert(c && userdata);

		PulseAudio* pa = (PulseAudio*)userdata;

		switch (pa_context_get_state(c)) {
			case PA_CONTEXT_CONNECTING:
			case PA_CONTEXT_AUTHORIZING:
			case PA_CONTEXT_SETTING_NAME:
				break;

			case PA_CONTEXT_READY:
				pa_context_get_server_info(c, server_info_callback, userdata);
				pa_context_set_subscribe_callback(c, subscribe_callback, userdata);
				pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK, NULL, NULL);
				break;

			case PA_CONTEXT_TERMINATED:
				pa->quit(0);
				break;

			case PA_CONTEXT_FAILED:
			default:
				fprintf(stderr, "PulseAudio failed to connect.\n");
				pa->quit(1);
				break;
		}
	}


	static void subscribe_callback(pa_context *c, pa_subscription_event_type_t type, uint32_t idx, void *userdata) {
		unsigned facility = type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;

		pa_operation *op = NULL;

		if (facility == PA_SUBSCRIPTION_EVENT_SINK)
			op = pa_context_get_sink_info_by_index(c, idx, sink_info_callback, userdata);

		if (op)
			pa_operation_unref(op);
	}

	static void sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
		if (!i)
			return;
		
		if (strcmp(i->name, default_sink))
			return;

		// This could be better..
		int previous_volume = volume;
		volume = roundf(((float)pa_cvolume_avg(&(i->volume)) / (float)PA_VOLUME_NORM) * 100.0f);
		if (volume != previous_volume)
			win->m_Dispatcher.emit();
	}

	static void server_info_callback(pa_context *c, const pa_server_info *i, void *userdata) {
		default_sink = i->default_sink_name;
		pa_context_get_sink_info_by_name(c, i->default_sink_name, sink_info_callback, userdata);
	}
};
/* END OF PULSE STUFF */

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
	PulseAudio pa = PulseAudio();
	pa.initialize();
	int ret = pa.run();
}

sysvol::sysvol() {
	// Initialize layer shell
	gtk_layer_init_for_window(gobj());
	gtk_layer_set_namespace(gobj(), "sysvol");
	gtk_layer_set_layer(gobj(), GTK_LAYER_SHELL_LAYER_OVERLAY);

	GtkLayerShellEdge edge;
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

	app = Gtk::Application::create("funky.sys64.sysvol");
	app->hold();
	win = new sysvol();

	thread_audio = std::thread(audio_server);

	return app->run();
}
