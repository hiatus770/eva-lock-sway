#pragma once

#include <wayland-client-core.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl-core.h>
#include <wayland-egl.h>
#include "../ext-session-lock-v1-protocol.h"
#include "../client_state.h"
#include "../graphics/graphics.h"

static const struct ext_session_lock_surface_v1_listener lock_surface_listener;

static void lock_surface_configure(void *data,
    struct ext_session_lock_surface_v1 *lock_surface,
    uint32_t serial, uint32_t width, uint32_t height)
{
    struct lock_output *lo = data;
    struct client_state *state = lo->state;

    fprintf(stderr, "lock_surface_configure: output %d serial=%u width=%u height=%u\n",
        (int)(lo - state->lock_outputs), serial, width, height);

    ext_session_lock_surface_v1_ack_configure(lock_surface, serial);

    if ((int)width > 0 && (int)height > 0) {
        lo->width  = (int)width;
        lo->height = (int)height;

        // Destroy and recreate EGL window+surface at the correct dimensions.
        // wl_egl_window_resize doesn't take effect before eglSwapBuffers on Mesa.
        eglMakeCurrent(state->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (lo->egl_surface) eglDestroySurface(state->egl_display, lo->egl_surface);
        if (lo->egl_window)  wl_egl_window_destroy(lo->egl_window);

        lo->egl_window  = wl_egl_window_create(lo->wl_surface, (int)width, (int)height);
        lo->egl_surface = eglCreateWindowSurface(state->egl_display, state->egl_config, lo->egl_window, NULL);
        eglMakeCurrent(state->egl_display, lo->egl_surface, lo->egl_surface, state->egl_context);
    }

    lo->configured = true;

    // Render first frame to this output
    state->egl_surface = lo->egl_surface;
    state->width  = lo->width;
    state->height = lo->height;
    SRC_WIDTH  = lo->width;
    SRC_HEIGHT = lo->height;
    recreate_framebuffers();
    render(state);

    fprintf(stderr, "lock_surface_configure: first frame rendered for output %d\n",
        (int)(lo - state->lock_outputs));
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

    for (int i = 0; i < state->num_outputs; i++) {
        struct lock_output *lo = &state->lock_outputs[i];
        lo->state = state;

        lo->wl_surface  = wl_compositor_create_surface(state->compositor);
        // Temp size — configure event will give us the real dimensions
        lo->egl_window  = wl_egl_window_create(lo->wl_surface, 1920, 1080);
        lo->egl_surface = eglCreateWindowSurface(state->egl_display, state->egl_config, lo->egl_window, NULL);
        lo->lock_surface = ext_session_lock_v1_get_lock_surface(
            state->ext_session_lock_v1, lo->wl_surface, lo->wl_output);
        ext_session_lock_surface_v1_add_listener(lo->lock_surface, &lock_surface_listener, lo);

        fprintf(stderr, "Created lock surface for output %d\n", i);
    }
}

static void handle_finished(void *data, struct ext_session_lock_v1 *lock)
{
    fprintf(stderr, "Exiting lock! OR Failed to lock!\n");
    struct client_state *state = data;
    if (!state->session_locked) {
        ext_session_lock_v1_destroy(lock);
        state->ext_session_lock_v1 = NULL;
    }
    state->closed = true;
}

static const struct ext_session_lock_v1_listener ext_session_lock_v1_listener = {
    .locked   = handle_locked,
    .finished = handle_finished,
};
