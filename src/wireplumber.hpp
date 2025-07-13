// wireplumber.hpp
#pragma once
#include <wp/wp.h>
#include <cstdint>
#include <string>
#include <functional>

class syshud_wireplumber {
	public:
		using VolumeCallback = std::function<void(double volume, bool output)>;

		syshud_wireplumber(VolumeCallback callback = nullptr);
		virtual ~syshud_wireplumber();

		int volume;
		bool muted;
		const char* output_name;
		const char* input_name;

		void set_volume(const bool& type, const int& value);

	private:
		VolumeCallback callback;
		GPtrArray* apis;
		WpCore* core;
		WpObjectManager* om;
		int pending_plugins;

		uint32_t output_id = 0;
		uint32_t input_id = 0;
		const gchar* node_name;
		WpPlugin *mixer_api;
		WpPlugin *def_nodes_api;

		void initialize();
		void activatePlugins();
		static bool is_valid_node_id(const uint32_t&);
		static void on_mixer_changed(syshud_wireplumber*, uint32_t);
		static void on_default_nodes_api_changed(syshud_wireplumber* self);
		static void on_plugin_activated(WpObject* p, GAsyncResult* res, syshud_wireplumber* self);
		static void on_mixer_api_loaded(WpObject* p, GAsyncResult* res, syshud_wireplumber* self);
		static void on_default_nodes_api_loaded(WpObject* p, GAsyncResult* res, syshud_wireplumber* self);
		static void on_object_manager_installed(syshud_wireplumber* self);
};
