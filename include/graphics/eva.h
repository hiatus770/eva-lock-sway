#pragma once

#include "text.h"

float* generate_gradient(int color_count, float (*arr)[3], int* length);

void render_clock(font* timer_font, struct camera global_camera); 