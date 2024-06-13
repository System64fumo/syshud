#pragma once
#include <glibmm/dispatcher.h>
#include <wp/wp.h>

class sysvol_wireplumber {
	public:
		int volume;
		bool muted;
		int input_volume;
		bool input_muted;


		sysvol_wireplumber(Glib::Dispatcher* callback);
		virtual ~sysvol_wireplumber();

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
		static void updateVolume(uint32_t id, sysvol_wireplumber* self);
		static void onMixerChanged(sysvol_wireplumber* self);
		static void onDefaultNodesApiChanged(sysvol_wireplumber* self);
		static void onPluginActivated(WpObject* p, GAsyncResult* res, sysvol_wireplumber* self);
		static void onMixerApiLoaded(WpObject* p, GAsyncResult* res, sysvol_wireplumber* self);
		static void onDefaultNodesApiLoaded(WpObject* p, GAsyncResult* res, sysvol_wireplumber* self);
		static void onObjectManagerInstalled(sysvol_wireplumber* self);
};
