#pragma once

#include <wayland-client-core.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl-core.h>
#include <wayland-egl.h>
#include "../ext-session-lock-v1-protocol.h"
#include "../client_state.h"
#include "../graphics/graphics.h"

static void lock_surface_configure(void *data,
    struct ext_session_lock_surface_v1 *lock_surface,
    uint32_t serial, uint32_t width, uint32_t height)
{
    struct client_state *state = data;
    ext_session_lock_surface_v1_ack_configure(lock_surface, serial);
    if ((int)width > 0 && (int)height > 0 && state->egl_window) {
        state->width  = (int)width;
        state->height = (int)height;
        wl_egl_window_resize(state->egl_window, (int)width, (int)height, 0, 0);
    }
    // Render first frame — eglSwapBuffers attaches the buffer, satisfying the compositor
    render(state);
}

static const struct ext_session_lock_surface_v1_listener lock_surface_listener = {
    .configure = lock_surface_configure,
};

static void handle_locked(void *data, struct ext_session_lock_v1 *lock)
{
    fprintf(stderr, "Session locked!\n");
    struct client_state *state = data;
    state->session_locked = true;
    state->locked = 1;

    if (!state->wl_output) {
        fprintf(stderr, "No wl_output available for lock surface\n");
        return;
    }

    state->ext_lock_surface = ext_session_lock_v1_get_lock_surface(
        state->ext_session_lock_v1, state->wl_surface, state->wl_output);
    ext_session_lock_surface_v1_add_listener(
        state->ext_lock_surface, &lock_surface_listener, state);
    // DO NOT commit here — no buffer is attached yet. The configure callback will render first.
}

static void handle_finished(void *data, struct ext_session_lock_v1 *lock)
{
    fprintf(stderr, "Exiting lock! OR Failed to lock!\n");
    struct client_state *state = data;
    if (!state->session_locked) {
        // Compositor refused the lock — destroy without unlocking
        ext_session_lock_v1_destroy(lock);
        state->ext_session_lock_v1 = NULL;
    }
    state->closed = true;
}

static const struct ext_session_lock_v1_listener ext_session_lock_v1_listener = {
    .locked   = handle_locked,
    .finished = handle_finished,
};
