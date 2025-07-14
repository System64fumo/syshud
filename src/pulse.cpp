#include "pulse.hpp"
#include <math.h>
#include <iostream>

syshud_pulseaudio::syshud_pulseaudio(VolumeCallback callback) : callback(callback) {
	thread = std::thread(&syshud_pulseaudio::initialize, this);
}

void syshud_pulseaudio::initialize() {
	int ret = 1;
	mainloop = pa_mainloop_new();
	mainloop_api = pa_mainloop_get_api(mainloop);
	pa_signal_init(mainloop_api);
	context = pa_context_new(mainloop_api, "syshud");
	ret = pa_context_connect(context, nullptr, PA_CONTEXT_NOAUTOSPAWN, nullptr);
	if (ret < 0) {
		std::cerr << "Failed to connect to PulseAudio context" << std::endl;
		return;
	}

	pa_context_set_state_callback(context, context_state_callback, this);
	pa_mainloop_run(mainloop, &ret);
}

void syshud_pulseaudio::set_volume(const bool& type, const int& value) {
	std::printf("Feature not implemented yet\n");
}

void syshud_pulseaudio::quit(int ret) {
	if (mainloop_api) {
		mainloop_api->quit(mainloop_api, ret);
	}
}

syshud_pulseaudio::~syshud_pulseaudio() {
	quit(0);

	if (thread.joinable()) {
		thread.join();
	}

	if (context) {
		pa_context_unref(context);
		context = nullptr;
	}

	if (signal) {
		pa_signal_free(signal);
		pa_signal_done();
		signal = nullptr;
	}

	if (mainloop) {
		pa_mainloop_free(mainloop);
		mainloop = nullptr;
		mainloop_api = nullptr;
	}
}

void syshud_pulseaudio::exit_signal_callback(pa_mainloop_api *m, pa_signal_event *e, int sig, void *userdata) {
	syshud_pulseaudio* pa = static_cast<syshud_pulseaudio*>(userdata);
	if (pa) pa->quit(0);
}

void syshud_pulseaudio::context_state_callback(pa_context *c, void *userdata) {
	syshud_pulseaudio* pa = static_cast<syshud_pulseaudio*>(userdata);

	switch (pa_context_get_state(c)) {
		case PA_CONTEXT_CONNECTING:
		case PA_CONTEXT_AUTHORIZING:
		case PA_CONTEXT_SETTING_NAME:
			break;

		case PA_CONTEXT_READY: {
			pa_context_get_server_info(c, server_info_callback, userdata);
			pa_context_set_subscribe_callback(c, subscribe_callback, userdata);

			pa_subscription_mask_t mask = static_cast<pa_subscription_mask_t>(
				PA_SUBSCRIPTION_MASK_SINK | PA_SUBSCRIPTION_MASK_SOURCE);
			pa_context_subscribe(c, mask, nullptr, nullptr);
			break;
		}

		case PA_CONTEXT_TERMINATED:
			pa->quit(0);
			break;

		case PA_CONTEXT_FAILED:
		default:
			std::cerr << "PulseAudio failed to connect." << std::endl;
			pa->quit(1);
			break;
	}
}

void syshud_pulseaudio::subscribe_callback(pa_context *c, pa_subscription_event_type_t type, uint32_t idx, void *userdata) {
	unsigned facility = type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;

	if (facility == PA_SUBSCRIPTION_EVENT_SINK) {
		pa_context_get_sink_info_by_index(c, idx, sink_info_callback, userdata);
	}
	else if (facility == PA_SUBSCRIPTION_EVENT_SOURCE) {
		pa_context_get_source_info_by_index(c, idx, source_info_callback, userdata);
	}
	else if (facility == PA_SUBSCRIPTION_EVENT_SERVER) {
		pa_context_get_server_info(c, server_info_callback, userdata);
	}
}

void syshud_pulseaudio::sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata) {
	if (eol || !i) return;
	
	syshud_pulseaudio* pa = static_cast<syshud_pulseaudio*>(userdata);
	pa->volume = roundf(((float)pa_cvolume_avg(&(i->volume)) / (float)PA_VOLUME_NORM) * 100.0f);
	pa->muted = i->mute;

	if (pa->callback) {
		pa->callback(pa->volume, true);
	}
}

void syshud_pulseaudio::source_info_callback(pa_context *c, const pa_source_info *i, int eol, void *userdata) {
	if (eol || !i) return;
	
	syshud_pulseaudio* pa = static_cast<syshud_pulseaudio*>(userdata);
	double mic_volume = roundf(((float)pa_cvolume_avg(&(i->volume)) / (float)PA_VOLUME_NORM) * 100.0f);
	pa->muted = i->mute;

	if (pa->callback) {
		pa->callback(mic_volume, false);
	}
}

void syshud_pulseaudio::server_info_callback(pa_context *c, const pa_server_info *i, void *userdata) {
	syshud_pulseaudio* pa = static_cast<syshud_pulseaudio*>(userdata);
	pa->output_name = i->default_sink_name;
	pa->input_name = i->default_source_name;

	pa_context_get_sink_info_by_name(c, i->default_sink_name, sink_info_callback, userdata);
	pa_context_get_source_info_by_name(c, i->default_source_name, source_info_callback, userdata);
}