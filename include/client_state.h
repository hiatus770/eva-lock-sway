#pragma once

#include <stdbool.h>
#include <xkbcommon/xkbcommon.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <EGL/egl.h>
#include "globals.h"
#include "wayland/pointer_struct.h"
#include "ext-session-lock-v1-protocol.h"

#define MAX_OUTPUTS 8

struct client_state; // forward declaration for lock_output back-pointer

struct lock_output {
    struct wl_output *wl_output;
    struct wl_surface *wl_surface;
    struct wl_egl_window *egl_window;
    EGLSurface egl_surface;
    struct ext_session_lock_surface_v1 *lock_surface;
    int width, height;
    bool configured;
    struct client_state *state;
};

struct client_state
{
    // Wayland specific
    struct wl_display *wl_display;
    struct wl_compositor *compositor;
    struct wl_shm *wl_shm;
    struct wl_registry *wl_registry;
    struct xdg_wm_base *xdg_wm_base;
    struct wl_seat *wl_seat;

    // Surface related wayland things
    struct wl_surface *wl_surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;

    // Input related wayland things
    struct wl_keyboard *wl_keyboard;
    struct wl_pointer *wl_pointer;
    struct wl_touch *wl_touch;

    // Locking
    struct ext_session_lock_manager_v1 *ext_session_lock_manager_v1;
    struct ext_session_lock_v1 *ext_session_lock_v1;
    bool session_locked; // true after .locked event from compositor
    struct lock_output lock_outputs[MAX_OUTPUTS];
    int num_outputs;

    // For pointer
    struct pointer_event pointer_event;
    struct xkb_state *xkb_state;
    struct xkb_context *xkb_context;
    struct xkb_keymap *xkb_keymap;

    int width, height;
    int locked; // 1 == LOCKED, 0 == UNLOCKED / NOT LOCKED
    bool closed;
    clock_state state;
    app_mode mode;

    // Password input (lock mode)
    char  password[256];
    int   password_len;
    int   indicator_step;    // 0-3 cycles through STOP/SLOW/NORMAL/RACING on keypress
    bool  indicator_visible; // toggles on each keypress for visual feedback

    // Countdown after successful auth
    bool  counting_down;
    float countdown_timer;

    // Auth feedback
    float flash_timer;    // counts down from 1.0 to 0.0
    float flash_r, flash_g, flash_b;
    int   auth_result;    // 0=none, 1=success, 2=fail

    // State
    float offset;
    uint32_t last_frame;
    float last_dt;        // delta time in seconds, updated each frame
    float intense_time;   // accumulated time for vignette oscillation

    // Orbit camera
    float cam_yaw;          // actual (smoothed) horizontal angle
    float cam_pitch;        // actual (smoothed) vertical angle
    float cam_yaw_target;   // target set by mouse input
    float cam_pitch_target;
    double last_pointer_x;
    double last_pointer_y;
    bool pointer_valid;

    // Frame rate limiting
    int timer_fd;
    uint32_t target_frame_ms; // e.g. 33 for ~30fps, 16 for ~60fps

    // Egl things
    EGLContext egl_context;
    EGLSurface egl_surface;
    struct wl_egl_window *egl_window;
    EGLDisplay egl_display;
    EGLConfig egl_config;
};
