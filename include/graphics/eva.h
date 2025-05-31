#pragma once

#include "text.h"
#include "entity.h"

float* generate_gradient(int color_count, float (*arr)[3], int* length);

void render_clock(font* timer_font, struct camera global_camera); 

void draw_line(struct entity* quad, float line_x, float line_y, float width, float height); 

void draw_line_rotate(struct entity* quad, float line_x, float line_y, float length, float width, float angle); 

void draw_box(struct entity* quad, float box_x, float box_y, float width, float height, float thickness); 

void draw_slant(struct entity* quad, float line_x, float line_y, float height, float line_x_f, float line_y_f); 