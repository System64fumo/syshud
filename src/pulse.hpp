#pragma once
#include <pulse/pulseaudio.h>
#include <glibmm/dispatcher.h>

class PulseAudio {
	public:
		PulseAudio(Glib::Dispatcher* output_callback);
		~PulseAudio();
		int initialize();

		int volume;
		bool muted;
		const char* output_name;
		const char* input_name;

	private:
		void quit(int ret);

		pa_mainloop* mainloop;
		pa_mainloop_api* mainloop_api;
		pa_context* context;
		pa_signal_event* signal;

		int previous_volume;
		bool previous_muted;
		Glib::Dispatcher* output_callback;

		static void exit_signal_callback(pa_mainloop_api *m, pa_signal_event *e, int sig, void *userdata);
		static void context_state_callback(pa_context *c, void *userdata);
		static void subscribe_callback(pa_context *c, pa_subscription_event_type_t type, uint32_t idx, void *userdata);
		static void sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
		static void server_info_callback(pa_context *c, const pa_server_info *i, void *userdata);
};
