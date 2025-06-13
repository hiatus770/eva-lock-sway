#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <EGL/egl.h>
#include "wayland/pointer.h"
#include "globals.h"

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

    // For pointer
    struct pointer_event pointer_event;


    int width, height;
    bool closed;
    clock_state state;

    // State
    float offset;
    uint32_t last_frame;

    // Egl things
    EGLContext egl_context;
    EGLSurface egl_surface;
    struct wl_egl_window *egl_window;
    EGLDisplay egl_display;
};
