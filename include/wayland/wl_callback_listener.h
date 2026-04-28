#pragma once
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/timerfd.h>
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
    wl_callback_destroy(cb);

    struct client_state *state = data;

    float dt = (state->last_frame != 0) ? (time - state->last_frame) / 1000.0f : 0.016f;
    state->last_dt = dt;
    state->offset += dt * 24;

    // Decay flash overlay
    if (state->flash_timer > 0.0f) {
        state->flash_timer -= dt;
        if (state->flash_timer < 0.0f) state->flash_timer = 0.0f;
    }

    // Smooth orbit camera interpolation (exponential decay, framerate-independent)
    float lerp = 1.0f - expf(-dt * 8.0f);
    state->cam_yaw   += (state->cam_yaw_target   - state->cam_yaw)   * lerp;
    state->cam_pitch += (state->cam_pitch_target  - state->cam_pitch) * lerp;

    // Countdown to unlock
    if (state->counting_down) {
        state->countdown_timer -= dt;
        if (state->countdown_timer <= 0.0f) {
            state->countdown_timer = 0.0f;
            state->counting_down = false;
            state->auth_result = 1;
        }
    }

    if (state->mode & MODE_LOCK) {
        // Render to every configured output
        for (int i = 0; i < state->num_outputs; i++) {
            struct lock_output *lo = &state->lock_outputs[i];
            if (!lo->configured) continue;

            eglMakeCurrent(state->egl_display, lo->egl_surface, lo->egl_surface, state->egl_context);
            state->egl_surface = lo->egl_surface;
            state->width  = lo->width;
            state->height = lo->height;

            if (SRC_WIDTH != lo->width || SRC_HEIGHT != lo->height) {
                SRC_WIDTH  = lo->width;
                SRC_HEIGHT = lo->height;
                recreate_framebuffers();
            }

            render(state);
        }
    } else {
        render(state);
    }

    state->last_frame = time;

    // Arm the timer for the next frame — main loop will request wl_surface_frame when it fires
    struct itimerspec its = {
        .it_value = {
            .tv_sec  =  state->target_frame_ms / 1000,
            .tv_nsec = (state->target_frame_ms % 1000) * 1000000L,
        },
    };
    timerfd_settime(state->timer_fd, 0, &its, NULL);
}

static const struct wl_callback_listener wl_surface_frame_listener = {
    .done = wl_surface_frame_done
};
