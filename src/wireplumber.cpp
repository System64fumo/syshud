#include "wireplumber.hpp"
#include <iostream>
#include <cmath>

bool syshud_wireplumber::isValidNodeId(uint32_t id) {
	return id > 0 && id < G_MAXUINT32;
}

void syshud_wireplumber::onMixerChanged(syshud_wireplumber* self, uint32_t id) {
	GVariant* variant = nullptr;
	if (!isValidNodeId(id)) {
		std::cerr << "Invalid node ID: " << id << std::endl;
		std::cerr << "Ignoring volume update." << std::endl;
		return;
	}

	g_autoptr(WpNode) node = static_cast<WpNode*>(wp_object_manager_lookup(
				self->om, WP_TYPE_NODE, WP_CONSTRAINT_TYPE_G_PROPERTY, "bound-id",
				"=u", id, nullptr));

	if (node == nullptr)
		return;

	g_signal_emit_by_name(self->mixer_api, "get-volume", id, &variant);
	double temp_volume;
	g_variant_lookup(variant, "volume", "d", &temp_volume);
	g_variant_lookup(variant, "mute", "b", &self->muted);
	// There is supposed to be a freeup thing here,
	// Too bad it segfaults!

	if (variant == nullptr) {
		std::cerr << "Node does not support volume\n" << std::endl;
		return;
	}

	// Figure out if the change came from an input or output device
	const std::string media_class = std::string(
						wp_pipewire_object_get_property(
							WP_PIPEWIRE_OBJECT(node), "media.class"));

	// Set values and trigger a callback
	self->volume = round(temp_volume * 100.0);
	if (media_class == "Audio/Source") {
		if (self->input_callback != nullptr)
			self->input_callback->emit();
	}
	else {
		if (self->output_callback != nullptr)
			self->output_callback->emit();
	}
}

void syshud_wireplumber::onDefaultNodesApiChanged(syshud_wireplumber* self) {
	g_signal_emit_by_name(self->def_nodes_api, "get-default-node", "Audio/Sink", &self->node_id);
	g_signal_emit_by_name(self->def_nodes_api, "get-default-node", "Audio/Source", &self->input_node_id);
	if (!isValidNodeId(self->node_id)) {
		std::cerr << "Invalid output node ID Ignoring volume update." << std::endl;
		return;
	}
	if (!isValidNodeId(self->input_node_id)) {
		std::cerr << "Invalid input node ID Ignoring volume update." << std::endl;
		return;
	}

	g_autoptr(WpNode) output_node = static_cast<WpNode*>(
	wp_object_manager_lookup(self->om, WP_TYPE_NODE, WP_CONSTRAINT_TYPE_G_PROPERTY,
							 "bound-id", "=u", self->node_id, nullptr));

	g_autoptr(WpNode) input_node = static_cast<WpNode*>(
	wp_object_manager_lookup(self->om, WP_TYPE_NODE, WP_CONSTRAINT_TYPE_G_PROPERTY,
							 "bound-id", "=u", self->input_node_id, nullptr));

	const gchar* output_node_name = wp_pipewire_object_get_property(WP_PIPEWIRE_OBJECT(output_node), "node.name");
	const gchar* input_node_name = wp_pipewire_object_get_property(WP_PIPEWIRE_OBJECT(output_node), "node.name");

	std::cout << "Audio device changed"<< std::endl;
	std::cout << "Output: " << output_node_name << std::endl;
	std::cout << "Output ID: " << self->node_id << std::endl;
	std::cout << "Output: " << input_node_name << std::endl;
	std::cout << "Output ID: " << self->input_node_id << std::endl;
}

void syshud_wireplumber::onPluginActivated(WpObject* p, GAsyncResult* res, syshud_wireplumber* self) {
	const auto* pluginName = wp_plugin_get_name(WP_PLUGIN(p));
	g_autoptr(GError) error = nullptr;
	if (wp_object_activate_finish(p, res, &error) == 0) {
		std::cerr << "error activating plugin: " << pluginName << std::endl;
		std::cerr << error->message << std::endl;
		return;
	}

	if (--self->pending_plugins == 0) {
		wp_core_install_object_manager(self->core, self->om);
	}
}

void syshud_wireplumber::activatePlugins() {
	for (uint16_t i = 0; i < apis->len; i++) {
		WpPlugin* plugin = static_cast<WpPlugin*>(g_ptr_array_index(apis, i));
		pending_plugins++;
		wp_object_activate(WP_OBJECT(plugin), WP_PLUGIN_FEATURE_ENABLED, nullptr,
			(GAsyncReadyCallback)onPluginActivated, this);
	}
}

void syshud_wireplumber::onMixerApiLoaded(WpObject* p, GAsyncResult* res, syshud_wireplumber* self) {
	gboolean success = FALSE;
	g_autoptr(GError) error = nullptr;

	success = wp_core_load_component_finish(self->core, res, nullptr);

	if (!success) {
		std::cout << "Mixer API failed to load" << std::endl;
		return;
	}

	g_ptr_array_add(self->apis, ({
					WpPlugin* p = wp_plugin_find(self->core, "mixer-api");
					g_object_set(G_OBJECT(p), "scale", 1 /* cubic */, nullptr);
					p;
					}));

	self->activatePlugins();
}

void syshud_wireplumber::onDefaultNodesApiLoaded(WpObject* p, GAsyncResult* res, syshud_wireplumber* self) {
	gboolean success = FALSE;
	g_autoptr(GError) error = nullptr;

	success = wp_core_load_component_finish(self->core, res, &error);

	if (!success) {
		std::cout << "Node API failed to load" << std::endl;
		return;
	}

	g_ptr_array_add(self->apis, wp_plugin_find(self->core, "default-nodes-api"));

	wp_core_load_component(self->core, "libwireplumber-module-mixer-api", "module", nullptr,
							"mixer-api", nullptr, (GAsyncReadyCallback)onMixerApiLoaded, self);
}

void syshud_wireplumber::onObjectManagerInstalled(syshud_wireplumber* self) {
	self->def_nodes_api = wp_plugin_find(self->core, "default-nodes-api");

	if (self->def_nodes_api == nullptr) {
		std::cerr << "Default nodes API is not loaded\n" << std::endl;
		return;
	}

	self->mixer_api = wp_plugin_find(self->core, "mixer-api");

	if (self->mixer_api == nullptr) {
		std::cerr << "Mixer api is not loaded\n" << std::endl;
		return;
	}

	g_signal_emit_by_name(self->def_nodes_api, "get-default-node", "Audio/Sink", &self->node_id);
	g_signal_emit_by_name(self->def_nodes_api, "get-default-node", "Audio/Source", &self->input_node_id);

	onMixerChanged(self, self->node_id);

	g_signal_connect_swapped(self->mixer_api, "changed", (GCallback)onMixerChanged, self);
	g_signal_connect_swapped(self->def_nodes_api, "changed", (GCallback)onDefaultNodesApiChanged,
							 self);
}

syshud_wireplumber::syshud_wireplumber(Glib::Dispatcher* input_callback, Glib::Dispatcher* output_callback) {
	this->input_callback = input_callback;
	this->output_callback = output_callback;
	wp_init(WP_INIT_PIPEWIRE);
	core = wp_core_new(NULL, NULL, NULL);
	apis = g_ptr_array_new_with_free_func(g_object_unref);
	om = wp_object_manager_new();

	wp_object_manager_add_interest(om, WP_TYPE_NODE,WP_CONSTRAINT_TYPE_PW_PROPERTY,
								   "media.class", "=s", "Audio/Sink", nullptr);
	wp_object_manager_add_interest(om, WP_TYPE_NODE,WP_CONSTRAINT_TYPE_PW_PROPERTY,
								   "media.class", "=s", "Audio/Source", nullptr);

	if (wp_core_connect(core) == 0) {
		std::cout << "Could not connect" << std::endl;
	}

	g_signal_connect_swapped(om, "installed", (GCallback)onObjectManagerInstalled, this);

	wp_core_load_component(core, "libwireplumber-module-default-nodes-api", "module", nullptr,
							"default-nodes-api", nullptr, (GAsyncReadyCallback)onDefaultNodesApiLoaded,
							this);
}

syshud_wireplumber::~syshud_wireplumber() {
	wp_core_disconnect(core);
	g_clear_pointer(&apis, g_ptr_array_unref);
	g_clear_object(&om);
	g_clear_object(&core);
	g_clear_object(&mixer_api);
	g_clear_object(&def_nodes_api);
}
