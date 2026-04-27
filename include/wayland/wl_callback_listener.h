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
#include "../xdg-shell-client-protocol.h" // TODO FIX THESE IMPORTS SO THEY AREN"T THIS UGLY!
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

    float dt = (state->last_frame != 0) ? (time - state->last_frame) / 1000.0f : 0.016f;
    state->last_dt = dt;
    state->offset += dt * 24;

    // Decay flash overlay
    if (state->flash_timer > 0.0f) {
        state->flash_timer -= dt;
        if (state->flash_timer < 0.0f) state->flash_timer = 0.0f;
    }

    // Countdown to unlock
    if (state->counting_down) {
        state->countdown_timer -= dt;
        if (state->countdown_timer <= 0.0f) {
            state->countdown_timer = 0.0f;
            state->counting_down = false;
            state->auth_result = 1;
        }
    }

    // Call to the render library based on the current state
    render(state);

    state->last_frame = time;
}

static const struct wl_callback_listener wl_surface_frame_listener = {
    .done = wl_surface_frame_done
};
