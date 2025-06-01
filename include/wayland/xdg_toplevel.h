#pragma once

#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-egl-core.h>
#include <EGL/egl.h>
#include <wayland-egl.h>
#include "../glad/glad.h"
#include "../xdg-shell-client-protocol.h" // TODO FIX THESE IMPORTS SO THEY AREN"T THIS UGLY!
#include "../client_state.h"

static void xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height, struct wl_array *states){
    struct client_state *state = data; 
    if (width == 0 || height == 0){
        return; 
    }
    
    if (state->egl_window){
        // This means the window does exist and we can indeed call the resize function
        wl_egl_window_resize(state->egl_window, width, height, 0, 0); 
        wl_surface_commit(state->wl_surface); 
    }
    
    state->width = width;
    state->height = height; 
}

static void xdg_toplevel_close(void *data, struct xdg_toplevel *toplevel){
    struct client_state *state = data; 
    state->closed = true; 
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure, 
    .close = xdg_toplevel_close
}; 