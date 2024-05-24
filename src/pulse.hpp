#include <pulse/pulseaudio.h>

class PulseAudio {
	private:
		pa_mainloop* mainloop;
		pa_mainloop_api* mainloop_api;
		pa_context* context;
		pa_signal_event* signal;

	public:
		PulseAudio() : 
			mainloop(NULL),
			mainloop_api(NULL),
			context(NULL),
			signal(NULL) {}

	int initialize();
	void quit(int ret);
	void destroy();
	~PulseAudio();

	private:
		static void exit_signal_callback(pa_mainloop_api *m, pa_signal_event *e, int sig, void *userdata);
		static void context_state_callback(pa_context *c, void *userdata);
		static void subscribe_callback(pa_context *c, pa_subscription_event_type_t type, uint32_t idx, void *userdata);
		static void sink_info_callback(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
		static void server_info_callback(pa_context *c, const pa_server_info *i, void *userdata);
};
