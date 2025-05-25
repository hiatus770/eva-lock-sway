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

static const struct wl_callback_listener wl_surface_frame_listener;

// This is the important function that is called whenever we are suppose dto render something according to the compositor
static void wl_surface_frame_done (void *data, struct wl_callback *cb, uint32_t time){
    // Destroy this callback
    wl_callback_destroy(cb);

    // Request another frame from the compositor
    struct client_state *state = data;
    cb = wl_surface_frame(state->wl_surface);
    wl_callback_add_listener(cb, &wl_surface_frame_listener, state); // This is why we defined our struct earlier and then redefine later

    // Idk if this is needed anymore
    if (state->last_frame != 0){
        int elapsed = time - state->last_frame;
        state->offset += elapsed / 1000.0 * 24;
    }

    // Call to the render library based on the current state
    render(state);

    state->last_frame = time;
}

static const struct wl_callback_listener wl_surface_frame_listener = {
    .done = wl_surface_frame_done
};
