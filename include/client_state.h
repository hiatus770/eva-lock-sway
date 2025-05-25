#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <EGL/egl.h>

struct client_state
{
    struct wl_display *wl_display;
    struct wl_compositor *compositor;
    struct wl_shm *wl_shm;
    struct wl_registry *wl_registry;
    struct xdg_wm_base *xdg_wm_base;

    // Objects
    struct wl_surface *wl_surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;

    int width, height;
    bool closed;

    // State
    float offset;
    uint32_t last_frame;

    // Egl things
    EGLContext egl_context;
    EGLSurface egl_surface;
    struct wl_egl_window *egl_window;
    EGLDisplay egl_display;
};
