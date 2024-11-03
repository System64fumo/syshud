#pragma once
#include <map>
#include <string>
#include <gtkmm/application.h>

Glib::RefPtr<Gtk::Application> app;
class syshud {};
syshud* win;

typedef syshud* (*syshud_create_func)(const std::map<std::string, std::map<std::string, std::string>>&);
syshud_create_func syshud_create_ptr;
