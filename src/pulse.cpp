#include "pulse.hpp"

#include <thread>
#include <math.h>

syshud_pulseaudio::syshud_pulseaudio(Glib::Dispatcher* output_callback) :
	output_callback(output_callback) {

	mainloop = pa_mainloop_new();
	mainloop_api = pa_mainloop_get_api(mainloop);
	pa_signal_init(mainloop_api);
	context = pa_context_new(mainloop_api, "syshud");
	pa_context_connect(context, nullptr, PA_CONTEXT_NOAUTOSPAWN, nullptr);
	pa_context_set_state_callback(context, context_state_callback, this);

	// Is this thread safe?
	std::thread([&]() {
		pa_mainloop_run(mainloop, NULL);
	}).detach();
}

void syshud_pulseaudio::quit(int ret = 0) {
	mainloop_api->quit(mainloop_api, ret);
}

syshud_pulseaudio::~syshud_pulseaudio() {
	quit(0);

	// Cleanup
	if (context) {
		pa_context_unref(context);
		context = NULL;
	}

	if (signal) {
		pa_signal_free(signal);
		pa_signal_done();
		signal = NULL;
	}

	if (mainloop) {
		pa_mainloop_free(mainloop);
		mainloop = NULL;
		mainloop_api = NULL;
	}
}

void syshud_pulseaudio::exit_signal_callback(pa_mainloop_api *m, pa_signal_event *e, int sig, void *userdata) {
	syshud_pulseaudio* pa = (syshud_pulseaudio*)userdata;
	if (pa) pa->quit();
}

void syshud_pulseaudio::context_state_callback(pa_context *c, void *userdata) {
	syshud_pulseaudio* pa = (syshud_pulseaudio*)userdata;

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
			std::fprintf(stderr, "Pulseaudio failed to connect.\n");
				pa->quit(1);
			break;
	}
}


void syshud_pulseaudio::subscribe_callback(pa_context *c, pa_subscription_event_type_t type, uint32_t idx, void *userdata) {
	unsigned facility = type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;

	pa_operation *op = NULL;

	if (facility == PA_SUBSCRIPTION_EVENT_SINK)
		op = pa_context_get_sink_info_by_index(c, idx, sink_info_callback, userdata);

	if (op)
		pa_operation_unref(op);

	if (facility == PA_SUBSCRIPTION_EVENT_SERVER)
		pa_context_get_server_info(c, server_info_callback, userdata);
}

void syshud_pulseaudio::sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
	syshud_pulseaudio* pa = (syshud_pulseaudio*)userdata;
	if (!i)
		return;

	if (strcmp(i->name, pa->output_name))
		return;

	// Set new values
	pa->volume = roundf(((float)pa_cvolume_avg(&(i->volume)) / (float)PA_VOLUME_NORM) * 100.0f);
	pa->muted = i->mute;

	// Trigger an update if needed
	if (pa->volume != pa->previous_volume || pa->muted != pa->previous_muted) {
		pa->output_callback->emit();

		pa->previous_volume = pa->volume;
		pa->previous_muted = pa->muted;
	}
}

void syshud_pulseaudio::server_info_callback(pa_context *c, const pa_server_info *i, void *userdata) {
	syshud_pulseaudio* pa = (syshud_pulseaudio*)userdata;
	pa->output_name = i->default_sink_name;
	pa->input_name = i->default_source_name;
	// std::printf("Output: %s\n", i->default_sink_name);
	// std::printf("Input: %s\n", i->default_source_name);
	
	pa_context_get_sink_info_by_name(c, i->default_sink_name, sink_info_callback, userdata);
}
