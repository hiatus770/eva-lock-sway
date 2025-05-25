#pragma once

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <wayland-client-core.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl-core.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
// Common imports needed to setup all the opengl goodies
#include "../glad/glad.h"
#include "../../src/xdg-shell-client-protocol.h" // TODO FIX THESE IMPORTS SO THEY AREN"T THIS UGLY!
#include "../client_state.h"
#include "../graphics/graphics.h"

static void xdg_surface_configure (void *data, struct xdg_surface *xdg_surface, uint32_t serial){
    struct client_state *state = data;

    xdg_surface_ack_configure(xdg_surface, serial);

    if (state->height != 0 && state->width !=0){
        glViewport(0, 0, state->width, state->height);
    }

    render(state);  // Calls the main graphics render function
    wl_surface_commit(state->wl_surface);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure
}; 
