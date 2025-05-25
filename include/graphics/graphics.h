#pragma once
#include "../../src/globals.h"
#include <stdint.h>
#include <wayland-client-core.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl-core.h>
#include <wayland-egl.h>
#include "../../src/xdg-shell-client-protocol.h" // TODO FIX THESE IMPORTS SO THEY AREN"T THIS UGLY!
#include "../client_state.h"
#include "../cglm/cglm.h"
#include "shader.h"
#include <ft2build.h>
#include FT_FREETYPE_H

struct character {
    unsigned int TextureID;
    vec2 size;
    vec2 bearing;
    unsigned int advance; // Offset to advance to the next glyph
};

// Main render function of the program
void render(struct client_state *state);

/**
* This function handles getting the codepoints given a utf-8 encoded string for functionality later in the program
*/
uint32_t* utf8_to_codepoints(const char *s, size_t *out_len);

/**
* This function is responsible for compiling and setting up shaders for their use in the render() function of the program, this is basically run for initializing opengl and all its quirks
* - make sure to have the egl context ready before calling this
* - make sure that all the file paths are correct when running this command as well
*/
void initgl(struct client_state *state);

void create_window_egl(struct client_state *state, int32_t width, int32_t height);

void render_quad(); 