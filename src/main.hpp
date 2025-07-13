#pragma once
#include <map>
#include <string>
#include <QApplication>

extern QApplication* app;
class syshud;
extern syshud* win;

typedef syshud* (*syshud_create_func)(const std::map<std::string, std::map<std::string, std::string>>&);
extern syshud_create_func syshud_create_ptr;
