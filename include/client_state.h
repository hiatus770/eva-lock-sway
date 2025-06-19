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

    // For pointer
    struct pointer_event pointer_event;
    struct xkb_state *xkb_state; 
    struct xkb_context *xkb_context; 
    struct xkb_keymap *xkb_keymap; 

    int width, height;
    int locked; // 1 == LOCKED, 0 == UNLOCKED / NOT LOCKED
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

struct eva_surface {
    struct wl_surface *wl_surface; 
    struct xdg_surface *xdg_surface; 
    struct xdg_toplevel *xdg_toplevel;
    EGLContext egl_context; 
    EGLSurface egl_surface; 
    struct wl_egl_window *egl_window; 
    EGLDisplay egl_display; 
    
}; 