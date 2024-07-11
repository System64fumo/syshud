#pragma once
#include "window.hpp"

config config_main;
syshud* win;

typedef syshud* (*syshud_create_func)(const config &cfg);
syshud_create_func syshud_create_ptr;
