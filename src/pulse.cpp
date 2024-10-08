#include "pulse.hpp"

#include <math.h>

PulseAudio::PulseAudio(Glib::Dispatcher* output_callback) {
	this->output_callback = output_callback;
}

int PulseAudio::initialize() {
	int ret = 1;
	mainloop = pa_mainloop_new();
	mainloop_api = pa_mainloop_get_api(mainloop);
	pa_signal_init(mainloop_api);
	context = pa_context_new(mainloop_api, "syshud");
	ret = pa_context_connect(context, nullptr, PA_CONTEXT_NOAUTOSPAWN, nullptr);
	if (ret < 0)
		return ret;

	pa_context_set_state_callback(context, context_state_callback, this);
	pa_mainloop_run(mainloop, &ret);
	return ret;
}

void PulseAudio::quit(int ret = 0) {
	mainloop_api->quit(mainloop_api, ret);
}

PulseAudio::~PulseAudio() {
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

void PulseAudio::exit_signal_callback(pa_mainloop_api *m, pa_signal_event *e, int sig, void *userdata) {
	PulseAudio* pa = (PulseAudio*)userdata;
	if (pa) pa->quit();
}

void PulseAudio::context_state_callback(pa_context *c, void *userdata) {
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
			std::fprintf(stderr, "PulseAudio failed to connect.\n");
				pa->quit(1);
			break;
	}
}


void PulseAudio::subscribe_callback(pa_context *c, pa_subscription_event_type_t type, uint32_t idx, void *userdata) {
	unsigned facility = type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;

	pa_operation *op = NULL;

	if (facility == PA_SUBSCRIPTION_EVENT_SINK)
		op = pa_context_get_sink_info_by_index(c, idx, sink_info_callback, userdata);

	if (op)
		pa_operation_unref(op);

	if (facility == PA_SUBSCRIPTION_EVENT_SERVER)
		pa_context_get_server_info(c, server_info_callback, userdata);
}

void PulseAudio::sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
	PulseAudio* pa = (PulseAudio*)userdata;
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

void PulseAudio::server_info_callback(pa_context *c, const pa_server_info *i, void *userdata) {
	PulseAudio* pa = (PulseAudio*)userdata;
	pa->output_name = i->default_sink_name;
	pa->input_name = i->default_source_name;
	std::printf("Output: %s\n", i->default_sink_name);
	std::printf("Input: %s\n", i->default_source_name);
	
	pa_context_get_sink_info_by_name(c, i->default_sink_name, sink_info_callback, userdata);
}
