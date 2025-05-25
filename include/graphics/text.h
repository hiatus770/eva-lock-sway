#pragma once
#include "map.h"
#include "camera.h" 
// This class exports the render_text function that handles all the different fonts etc etc

typedef struct {
    table* characters;
    unsigned int t_vao, t_vbo; // these are the related vbo and vao of the font use dfor loading
    char* goal; // this is the set of characters that it must be able to render (extra for japanese basically)
    float scale_x, scale_y;
    int pixel_size;
    struct shader* font_shader;
} font;

void render_font(font* font, char* source, float x, float y, float scale, vec3 color, struct camera global_camera); 

void init_font(font* font, struct shader* f_shader, char* font_otf, char* goal_source, int pixel_size, float scale_x, float scale_Y);

void init_test_bloom(); 
