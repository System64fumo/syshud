#pragma once
#include "window.hpp"

#include <gtkmm/application.h>

Glib::RefPtr<Gtk::Application> app;
config_hud config_main;
syshud* win;

typedef syshud* (*syshud_create_func)(const config_hud &cfg);
syshud_create_func syshud_create_ptr;
