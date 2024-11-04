#pragma once
#include <glibmm/dispatcher.h>
#include <wp/wp.h>

class syshud_wireplumber {
	public:
		syshud_wireplumber(Glib::Dispatcher*, Glib::Dispatcher*);
		virtual ~syshud_wireplumber();

		int volume;
		bool muted;
		const char* output_name;
		const char* input_name;

		void set_volume(const bool& type, const double& value);

	private:
		Glib::Dispatcher* input_callback;
		Glib::Dispatcher* output_callback;

		GPtrArray* apis;
		WpCore* core;
		WpObjectManager* om;
		int pending_plugins;

		uint32_t output_id = 0;
		uint32_t input_id = 0;
		const gchar* node_name;
		WpPlugin *mixer_api;
		WpPlugin *def_nodes_api;

		void activatePlugins();
		static bool is_valid_node_id(const uint32_t&);
		static void on_mixer_changed(syshud_wireplumber*, uint32_t);
		static void on_default_nodes_api_changed(syshud_wireplumber* self);
		static void on_plugin_activated(WpObject* p, GAsyncResult* res, syshud_wireplumber* self);
		static void on_mixer_api_loaded(WpObject* p, GAsyncResult* res, syshud_wireplumber* self);
		static void on_default_nodes_api_loaded(WpObject* p, GAsyncResult* res, syshud_wireplumber* self);
		static void on_object_manager_installed(syshud_wireplumber* self);
};
