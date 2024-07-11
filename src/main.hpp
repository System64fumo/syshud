#pragma once
#include "window.hpp"

inline syshud* win;

typedef syshud* (*syshud_create_func)(const config &cfg);
inline syshud_create_func syshud_create_ptr;
