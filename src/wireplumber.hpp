#pragma once
#include <glibmm/dispatcher.h>
#include <wp/wp.h>

class syshud_wireplumber {
	public:
		int volume;
		bool muted;
		bool input;

		syshud_wireplumber(Glib::Dispatcher* callback);
		virtual ~syshud_wireplumber();

	private:
		Glib::Dispatcher* callback;

		GPtrArray *apis;
		WpCore *core;
		WpObjectManager *om;
		int pending_plugins;

		uint32_t node_id = 0;
		uint32_t input_node_id = 0;
		const gchar* node_name;
		WpPlugin *mixer_api;
		WpPlugin *def_nodes_api;

		void activatePlugins();
		static bool isValidNodeId(uint32_t id);
		static void onMixerChanged(syshud_wireplumber* self, uint32_t id);
		static void onDefaultNodesApiChanged(syshud_wireplumber* self);
		static void onPluginActivated(WpObject* p, GAsyncResult* res, syshud_wireplumber* self);
		static void onMixerApiLoaded(WpObject* p, GAsyncResult* res, syshud_wireplumber* self);
		static void onDefaultNodesApiLoaded(WpObject* p, GAsyncResult* res, syshud_wireplumber* self);
		static void onObjectManagerInstalled(syshud_wireplumber* self);
};
