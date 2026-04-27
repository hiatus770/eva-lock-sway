// #include <cctype>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include "../include/ext-session-lock-v1-protocol.h"
#include <math.h>

// Wayland related imports 
#include <wayland-client-core.h>
#include <xkbcommon/xkbcommon.h> 
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl-core.h>
#include <wayland-egl.h>
#include <EGL/egl.h>


// #include <GL/gl.h>
// #include <glad/glad.h>

// Graphics related imports 
#include "../include/glad/glad.h"
#include "../include/xdg-shell-client-protocol.h"
#include "../include/globals.h"

// Other imports 
#include "../include/graphics/shader.h"
#include <assert.h>

/* Wayland Code */
#include "../include/client_state.h" // Main client state used for the program
#include "../include/wayland/wl_buffer.h" // Buffer listener
#include "../include/wayland/xdg_surface.h"
#include "../include/wayland/xdg_toplevel.h" // Top level application stuff handles closing and resizing
#include "../include/wayland/wm_base.h" // Base wm
#include "../include/wayland/wl_callback_listener.h"
#include "../include/wayland/registry_handler.h"
#include "../include/graphics/graphics.h"
#include "../include/wayland/lock_manager.h"

static volatile sig_atomic_t g_should_exit = 0;

static void handle_signal(int sig) {
    (void)sig;
    g_should_exit = 1;
}

void wayland_init(struct client_state *state){
    state->width = 1920;
    state->height = 1080;
    state->wl_display = wl_display_connect(NULL);
    state->wl_registry = wl_display_get_registry(state->wl_display);

    if (!state->wl_display)
    {
        fprintf(stderr, "Failed to connect to wayland display.\n");
        return;
    }

    wl_registry_add_listener(state->wl_registry, &registry_listener, state);
    wl_display_roundtrip(state->wl_display);
    fprintf(stderr, "Connected to wayland display.\n");

    state->wl_surface = wl_compositor_create_surface(state->compositor);

    if (!(state->mode & MODE_LOCK)) {
        // Normal path: regular xdg_surface window
        state->xdg_surface = xdg_wm_base_get_xdg_surface(state->xdg_wm_base, state->wl_surface);
        xdg_surface_add_listener(state->xdg_surface, &xdg_surface_listener, state);
        state->xdg_toplevel = xdg_surface_get_toplevel(state->xdg_surface);
        xdg_toplevel_add_listener(state->xdg_toplevel, &xdg_toplevel_listener, state);
    }
    // Lock path: lock request is deferred to after EGL/GL init in main()
}

int main(int argc, char *argv[])
{
    signal(SIGTERM, handle_signal);
    signal(SIGINT,  handle_signal);

    struct client_state state = {0};
    // Default orbit camera: matches hardcoded position {1.5,0,3} looking at {-0.5,0,-1}
    // offset from target = {2,0,4}, radius = sqrt(20)
    state.cam_yaw          = atan2f(2.0f, 4.0f);
    state.cam_pitch        = 0.0f;
    state.cam_yaw_target   = state.cam_yaw;
    state.cam_pitch_target = 0.0f;
    state.state = NORMAL;
    state.xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    state.locked = 0;

    // Parse CLI arguments
    state.mode = MODE_CLOCK;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--lock") == 0)    state.mode |= MODE_LOCK;
        if (strcmp(argv[i], "--intense") == 0) state.mode |= MODE_INTENSE;
    }
    if (state.mode & MODE_INTENSE) state.state = ALARM;

    wayland_init(&state);
    if (state.closed) return 1;

    create_window_egl(&state, 1920, 1080);

    if (!gladLoadGLLoader((GLADloadproc)eglGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return 1;
    }

    initgl(&state);

    if (!eglMakeCurrent(state.egl_display, state.egl_surface, state.egl_surface, state.egl_context)) {
        fprintf(stderr, "Failed to make EGL context current: %d\n", eglGetError());
    }

    if (state.mode & MODE_LOCK) {
        // Lock request happens HERE — after EGL/GL are fully ready
        if (!state.ext_session_lock_manager_v1) {
            fprintf(stderr, "Compositor does not support ext-session-lock-v1\n");
            return 1;
        }
        state.ext_session_lock_v1 = ext_session_lock_manager_v1_lock(
            state.ext_session_lock_manager_v1);
        ext_session_lock_v1_add_listener(state.ext_session_lock_v1,
            &ext_session_lock_v1_listener, &state);
        // Roundtrip: waits for .locked → handle_locked creates lock surface
        //            then .configure → lock_surface_configure renders first frame
        wl_display_roundtrip(state.wl_display);
        if (state.closed) return 1; // compositor refused the lock
    } else {
        xdg_toplevel_set_title(state.xdg_toplevel, "eva-clock");
        wl_surface_commit(state.wl_surface);
    }

    struct wl_surface *frame_surface = state.wl_surface;
    if (state.mode & MODE_LOCK) {
        for (int i = 0; i < state.num_outputs; i++) {
            if (state.lock_outputs[i].configured) {
                frame_surface = state.lock_outputs[i].wl_surface;
                break;
            }
        }
    }
    struct wl_callback *cb = wl_surface_frame(frame_surface);
    wl_callback_add_listener(cb, &wl_surface_frame_listener, &state);

    // wl_surface_frame only fires after the next commit on that surface.
    // The configure renders already happened, so we need one more commit to
    // attach the frame callback. Render to all outputs once here.
    if (state.mode & MODE_LOCK) {
        for (int i = 0; i < state.num_outputs; i++) {
            struct lock_output *lo = &state.lock_outputs[i];
            if (!lo->configured) continue;
            eglMakeCurrent(state.egl_display, lo->egl_surface, lo->egl_surface, state.egl_context);
            state.egl_surface = lo->egl_surface;
            state.width  = lo->width;
            state.height = lo->height;
            SRC_WIDTH  = lo->width;
            SRC_HEIGHT = lo->height;
            recreate_framebuffers();
            render(&state);
        }
    }

    while (wl_display_dispatch(state.wl_display) != -1)
    {
        if (state.closed || g_should_exit) break;

        // Successful auth: unlock and exit
        if (state.auth_result == 1 && state.session_locked) {
            ext_session_lock_v1_unlock_and_destroy(state.ext_session_lock_v1);
            state.ext_session_lock_v1 = NULL;
            state.session_locked = false;
            wl_display_roundtrip(state.wl_display);
            break;
        }
        state.auth_result = 0;
    }

    eglMakeCurrent(state.egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(state.egl_display, state.egl_context);

    if (state.mode & MODE_LOCK) {
        // Destroy per-output resources
        for (int i = 0; i < state.num_outputs; i++) {
            struct lock_output *lo = &state.lock_outputs[i];
            if (lo->lock_surface) ext_session_lock_surface_v1_destroy(lo->lock_surface);
            if (lo->egl_surface)  eglDestroySurface(state.egl_display, lo->egl_surface);
            if (lo->egl_window)   wl_egl_window_destroy(lo->egl_window);
            if (lo->wl_surface)   wl_surface_destroy(lo->wl_surface);
        }
        // Destroy original EGL window/surface from create_window_egl (unused in lock mode)
        wl_egl_window_destroy(state.egl_window);
        // state.egl_surface aliases a lo->egl_surface already destroyed — skip it

        if (state.ext_session_lock_v1) {
            if (state.session_locked)
                ext_session_lock_v1_unlock_and_destroy(state.ext_session_lock_v1);
            else
                ext_session_lock_v1_destroy(state.ext_session_lock_v1);
        }
    } else {
        eglDestroySurface(state.egl_display, state.egl_surface);
        wl_egl_window_destroy(state.egl_window);
        if (state.xdg_toplevel) xdg_toplevel_destroy(state.xdg_toplevel);
        if (state.xdg_surface)  xdg_surface_destroy(state.xdg_surface);
    }

    wl_surface_destroy(state.wl_surface);
    wl_display_disconnect(state.wl_display);
    return 0;
}
